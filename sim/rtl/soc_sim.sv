// A simulation model of the Etch A Sketch SoC.
//
// On the board, Qsys generates the Avalon interconnect that wires the Clarvi
// core to its memories and peripherals. Qsys output is a build artifact and is
// not in this repository, so this module plays that part: it decodes the same
// memory map that software/src/avalon_addr.h describes and attaches the same
// peripherals. The core, the rotary decoders and the button scanner are the
// real RTL, unchanged.
//
// What is deliberately *not* modelled is PixelStream. On hardware it reads the
// framebuffer and drives the LCD; here the framebuffer is simply read out
// through the probe port when the run finishes.

`timescale 1ns/10ps

module soc_sim #(
    parameter DATA_ADDR_WIDTH  = 28,       // wide enough to reach 0x08000000
    parameter INSTR_ADDR_WIDTH = 14,       // 64 KiB of code
    parameter RAM_WORDS        = 1 << 14,  // 64 KiB
    parameter FB_WORDS         = 65280,    // 480 * 272 * 2 bytes
    parameter DEBOUNCE_WIDTH   = 4         // shortened; the board uses 15
)(
    input  logic        clock,
    input  logic        reset,

    // raw quadrature from the two dials, and the display board's buttons
    input  logic [1:0]  dial_l,
    input  logic [1:0]  dial_r,
    input  logic [15:0] buttons,

    // peripheral outputs, for the harness to observe
    output logic [23:0] hex,
    output logic [9:0]  leds,

    // asynchronous read-out of the framebuffer
    input  logic [15:0] probe_addr,
    output logic [31:0] probe_data,

    output logic [31:0] debug_pc
);

    // ---------------------------------------------------------------- memory
    logic [31:0] ram [RAM_WORDS];
    logic [31:0] fb  [FB_WORDS];

    string mem_file;
    initial begin
        if (!$value$plusargs("mem=%s", mem_file))
            mem_file = "mem.txt";
        $readmemh(mem_file, ram);
        for (int i = 0; i < FB_WORDS; i++)
            fb[i] = 32'h0;
    end

    assign probe_data = fb[probe_addr];

    // ------------------------------------------------------------ core wires
    logic [DATA_ADDR_WIDTH-1:0]  main_address;
    logic [3:0]                  main_byteenable;
    logic                        main_read, main_write;
    logic [31:0]                 main_readdata, main_writedata;
    logic                        main_readdatavalid;
    logic [INSTR_ADDR_WIDTH-1:0] instr_address;
    logic                        instr_read;
    logic [31:0]                 instr_readdata;

    // ------------------------------------------------------------- peripherals
    logic [7:0] rotary_pos_l, rotary_pos_r;

    rotary #(.DEBOUNCE_COUNTER_WIDTH(DEBOUNCE_WIDTH)) dial_left (
        .clk(clock), .rst(reset), .rotary_in(dial_l),
        .rotary_pos(rotary_pos_l), .rot_cw(), .rot_ccw()
    );
    rotary #(.DEBOUNCE_COUNTER_WIDTH(DEBOUNCE_WIDTH)) dial_right (
        .clk(clock), .rst(reset), .rotary_in(dial_r),
        .rotary_pos(rotary_pos_r), .rot_cw(), .rot_ccw()
    );

    // The buttons reach the SoC through a parallel-in serial-out shift
    // register on the display board. Model that chip so the real ShiftRegCtl
    // RTL is exercised rather than bypassed.
    logic        shiftreg_clk, shiftreg_loadn, shiftreg_out, shiftreg_clk_q;
    logic [15:0] shiftreg_shadow;
    logic [15:0] buttons_scanned;

    always_ff @(posedge clock) begin
        shiftreg_clk_q <= shiftreg_clk;
        if (!shiftreg_loadn)
            shiftreg_shadow <= buttons;
        else if (shiftreg_clk && !shiftreg_clk_q)
            shiftreg_shadow <= shiftreg_shadow >> 1;
    end
    assign shiftreg_out = shiftreg_shadow[0];

    shiftregctl scanner (
        .clock_50m(clock), .reset(reset),
        .shiftreg_clk(shiftreg_clk), .shiftreg_loadn(shiftreg_loadn),
        .shiftreg_out(shiftreg_out), .buttons(buttons_scanned)
    );

    // ----------------------------------------------------------- address map
    // main_address is a word address: {high, main_address, 2'b0} == byte address.
    wire is_ram = (main_address[DATA_ADDR_WIDTH-1:14] == '0);
    wire is_pio = (main_address[DATA_ADDR_WIDTH-1:24] == 4'h1);
    wire is_fb  = (main_address[DATA_ADDR_WIDTH-1:24] == 4'h2);

    localparam PIO_LED   = 8'h00; // byte 0x04000000
    localparam PIO_HEX   = 8'h20; // byte 0x04000080
    localparam PIO_DIALL = 8'h40; // byte 0x04000100
    localparam PIO_DIALR = 8'h80; // byte 0x04000200
    localparam PIO_BTN   = 8'hC0; // byte 0x04000300

    // ------------------------------------------------------------ bus slaves
    logic [31:0] ram_q, fb_q, pio_q;
    logic        sel_ram_q, sel_fb_q;

    always_ff @(posedge clock) begin
        // data port
        if (main_write && is_ram)
            for (int i = 0; i < 4; i++)
                if (main_byteenable[i])
                    ram[main_address[13:0]][i*8 +: 8] <= main_writedata[i*8 +: 8];
        if (main_write && is_fb)
            for (int i = 0; i < 4; i++)
                if (main_byteenable[i])
                    fb[main_address[15:0]][i*8 +: 8] <= main_writedata[i*8 +: 8];
        if (main_write && is_pio) begin
            case (main_address[7:0])
                PIO_LED: leds <= main_writedata[9:0];
                PIO_HEX: hex  <= main_writedata[23:0];
                default: ;
            endcase
        end

        // Clarvi only supports slaves with a fixed one-cycle read latency, so
        // every read is registered here and flagged valid on the next cycle.
        ram_q <= ram[main_address[13:0]];
        fb_q  <= fb[main_address[15:0]];
        case (main_address[7:0])
            PIO_DIALL: pio_q <= {24'h0, rotary_pos_l};
            PIO_DIALR: pio_q <= {24'h0, rotary_pos_r};
            PIO_BTN:   pio_q <= {16'h0, buttons_scanned};
            PIO_HEX:   pio_q <= {8'h0, hex};
            PIO_LED:   pio_q <= {22'h0, leds};
            default:   pio_q <= 32'h0;
        endcase
        sel_ram_q <= is_ram;
        sel_fb_q  <= is_fb;
        main_readdatavalid <= main_read;

        // instruction port, same one-cycle latency
        if (instr_read)
            instr_readdata <= ram[instr_address];
    end

    assign main_readdata = sel_ram_q ? ram_q : sel_fb_q ? fb_q : pio_q;

    // ------------------------------------------------------------------ core
    clarvi_avalon #(
        .DATA_ADDR_WIDTH (DATA_ADDR_WIDTH),
        .INSTR_ADDR_WIDTH(INSTR_ADDR_WIDTH)
    ) cpu (
        .clock                   (clock),
        .reset                   (reset),
        .avm_main_address        (main_address),
        .avm_main_byteenable     (main_byteenable),
        .avm_main_read           (main_read),
        .avm_main_readdata       (main_readdata),
        .avm_main_write          (main_write),
        .avm_main_writedata      (main_writedata),
        .avm_main_waitrequest    (1'b0),
        .avm_main_readdatavalid  (main_readdatavalid),
        .avm_instr_address       (instr_address),
        .avm_instr_read          (instr_read),
        .avm_instr_readdata      (instr_readdata),
        .avm_instr_waitrequest   (1'b0),
        .avm_instr_readdatavalid (1'b1),
        .inr_irq                 (1'b0),
        .debug_register28        (),
        .debug_scratch           (),
        .debug_pc                (debug_pc)
    );

endmodule
