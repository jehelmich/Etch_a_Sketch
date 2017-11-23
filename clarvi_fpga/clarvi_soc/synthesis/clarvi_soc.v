// clarvi_soc.v

// Generated using ACDS version 16.1 196

`timescale 1 ps / 1 ps
module clarvi_soc (
		input  wire [15:0] buttons_pio_external_connection_export,   //   buttons_pio_external_connection.export
		input  wire        clk_clk,                                  //                               clk.clk
		output wire [23:0] hex_pio_external_connection_export,       //       hex_pio_external_connection.export
		output wire [9:0]  led_pio_external_connection_export,       //       led_pio_external_connection.export
		input  wire [7:0]  leftdial_pio_external_connection_export,  //  leftdial_pio_external_connection.export
		output wire [7:0]  pixelstream_0_conduit_end_0_lcd_red,      //       pixelstream_0_conduit_end_0.lcd_red
		output wire [7:0]  pixelstream_0_conduit_end_0_lcd_green,    //                                  .lcd_green
		output wire [7:0]  pixelstream_0_conduit_end_0_lcd_blue,     //                                  .lcd_blue
		output wire        pixelstream_0_conduit_end_0_lcd_hsync,    //                                  .lcd_hsync
		output wire        pixelstream_0_conduit_end_0_lcd_vsync,    //                                  .lcd_vsync
		output wire        pixelstream_0_conduit_end_0_lcd_de,       //                                  .lcd_de
		output wire        pixelstream_0_conduit_end_0_lcd_dclk,     //                                  .lcd_dclk
		output wire        pixelstream_0_conduit_end_0_lcd_dclk_en,  //                                  .lcd_dclk_en
		input  wire        reset_reset_n,                            //                             reset.reset_n
		input  wire [7:0]  rightdial_pio_external_connection_export  // rightdial_pio_external_connection.export
	);

	wire         pll_outclk0_clk;                                              // pll:outclk_0 -> [PixelStream_0:csi_clockreset_clk, buttons_pio:clk, clarvi_0:clock, hex_pio:clk, led_pio:clk, leftdial_pio:clk, mm_interconnect_0:pll_outclk0_clk, mm_interconnect_1:pll_outclk0_clk, onchip_memory2_0:clk, onchip_memory2_0:clk2, rightdial_pio:clk, rst_controller:clk, video_memory:clk]
	wire         pll_outclk1_clk;                                              // pll:outclk_1 -> PixelStream_0:csi_video_clk
	wire  [31:0] clarvi_0_instr_readdata;                                      // mm_interconnect_0:clarvi_0_instr_readdata -> clarvi_0:avm_instr_readdata
	wire         clarvi_0_instr_waitrequest;                                   // mm_interconnect_0:clarvi_0_instr_waitrequest -> clarvi_0:avm_instr_waitrequest
	wire  [13:0] clarvi_0_instr_address;                                       // clarvi_0:avm_instr_address -> mm_interconnect_0:clarvi_0_instr_address
	wire         clarvi_0_instr_read;                                          // clarvi_0:avm_instr_read -> mm_interconnect_0:clarvi_0_instr_read
	wire         clarvi_0_instr_readdatavalid;                                 // mm_interconnect_0:clarvi_0_instr_readdatavalid -> clarvi_0:avm_instr_readdatavalid
	wire         mm_interconnect_0_onchip_memory2_0_s2_chipselect;             // mm_interconnect_0:onchip_memory2_0_s2_chipselect -> onchip_memory2_0:chipselect2
	wire  [31:0] mm_interconnect_0_onchip_memory2_0_s2_readdata;               // onchip_memory2_0:readdata2 -> mm_interconnect_0:onchip_memory2_0_s2_readdata
	wire  [13:0] mm_interconnect_0_onchip_memory2_0_s2_address;                // mm_interconnect_0:onchip_memory2_0_s2_address -> onchip_memory2_0:address2
	wire   [3:0] mm_interconnect_0_onchip_memory2_0_s2_byteenable;             // mm_interconnect_0:onchip_memory2_0_s2_byteenable -> onchip_memory2_0:byteenable2
	wire         mm_interconnect_0_onchip_memory2_0_s2_write;                  // mm_interconnect_0:onchip_memory2_0_s2_write -> onchip_memory2_0:write2
	wire  [31:0] mm_interconnect_0_onchip_memory2_0_s2_writedata;              // mm_interconnect_0:onchip_memory2_0_s2_writedata -> onchip_memory2_0:writedata2
	wire         mm_interconnect_0_onchip_memory2_0_s2_clken;                  // mm_interconnect_0:onchip_memory2_0_s2_clken -> onchip_memory2_0:clken2
	wire  [31:0] clarvi_0_main_readdata;                                       // mm_interconnect_1:clarvi_0_main_readdata -> clarvi_0:avm_main_readdata
	wire         clarvi_0_main_waitrequest;                                    // mm_interconnect_1:clarvi_0_main_waitrequest -> clarvi_0:avm_main_waitrequest
	wire  [28:0] clarvi_0_main_address;                                        // clarvi_0:avm_main_address -> mm_interconnect_1:clarvi_0_main_address
	wire   [3:0] clarvi_0_main_byteenable;                                     // clarvi_0:avm_main_byteenable -> mm_interconnect_1:clarvi_0_main_byteenable
	wire         clarvi_0_main_read;                                           // clarvi_0:avm_main_read -> mm_interconnect_1:clarvi_0_main_read
	wire         clarvi_0_main_readdatavalid;                                  // mm_interconnect_1:clarvi_0_main_readdatavalid -> clarvi_0:avm_main_readdatavalid
	wire         clarvi_0_main_write;                                          // clarvi_0:avm_main_write -> mm_interconnect_1:clarvi_0_main_write
	wire  [31:0] clarvi_0_main_writedata;                                      // clarvi_0:avm_main_writedata -> mm_interconnect_1:clarvi_0_main_writedata
	wire  [31:0] pixelstream_0_master_burstreads_readdata;                     // mm_interconnect_1:PixelStream_0_master_burstreads_readdata -> PixelStream_0:avm_m0_readdata
	wire         pixelstream_0_master_burstreads_waitrequest;                  // mm_interconnect_1:PixelStream_0_master_burstreads_waitrequest -> PixelStream_0:avm_m0_waitrequest
	wire  [30:0] pixelstream_0_master_burstreads_address;                      // PixelStream_0:avm_m0_address -> mm_interconnect_1:PixelStream_0_master_burstreads_address
	wire         pixelstream_0_master_burstreads_read;                         // PixelStream_0:avm_m0_read -> mm_interconnect_1:PixelStream_0_master_burstreads_read
	wire         pixelstream_0_master_burstreads_readdatavalid;                // mm_interconnect_1:PixelStream_0_master_burstreads_readdatavalid -> PixelStream_0:avm_m0_readdatavalid
	wire  [31:0] pixelstream_0_master_burstreads_writedata;                    // PixelStream_0:avm_m0_writedata -> mm_interconnect_1:PixelStream_0_master_burstreads_writedata
	wire         pixelstream_0_master_burstreads_write;                        // PixelStream_0:avm_m0_write -> mm_interconnect_1:PixelStream_0_master_burstreads_write
	wire         mm_interconnect_1_onchip_memory2_0_s1_chipselect;             // mm_interconnect_1:onchip_memory2_0_s1_chipselect -> onchip_memory2_0:chipselect
	wire  [31:0] mm_interconnect_1_onchip_memory2_0_s1_readdata;               // onchip_memory2_0:readdata -> mm_interconnect_1:onchip_memory2_0_s1_readdata
	wire  [13:0] mm_interconnect_1_onchip_memory2_0_s1_address;                // mm_interconnect_1:onchip_memory2_0_s1_address -> onchip_memory2_0:address
	wire   [3:0] mm_interconnect_1_onchip_memory2_0_s1_byteenable;             // mm_interconnect_1:onchip_memory2_0_s1_byteenable -> onchip_memory2_0:byteenable
	wire         mm_interconnect_1_onchip_memory2_0_s1_write;                  // mm_interconnect_1:onchip_memory2_0_s1_write -> onchip_memory2_0:write
	wire  [31:0] mm_interconnect_1_onchip_memory2_0_s1_writedata;              // mm_interconnect_1:onchip_memory2_0_s1_writedata -> onchip_memory2_0:writedata
	wire         mm_interconnect_1_onchip_memory2_0_s1_clken;                  // mm_interconnect_1:onchip_memory2_0_s1_clken -> onchip_memory2_0:clken
	wire         mm_interconnect_1_video_memory_s1_chipselect;                 // mm_interconnect_1:video_memory_s1_chipselect -> video_memory:chipselect
	wire  [31:0] mm_interconnect_1_video_memory_s1_readdata;                   // video_memory:readdata -> mm_interconnect_1:video_memory_s1_readdata
	wire  [15:0] mm_interconnect_1_video_memory_s1_address;                    // mm_interconnect_1:video_memory_s1_address -> video_memory:address
	wire   [3:0] mm_interconnect_1_video_memory_s1_byteenable;                 // mm_interconnect_1:video_memory_s1_byteenable -> video_memory:byteenable
	wire         mm_interconnect_1_video_memory_s1_write;                      // mm_interconnect_1:video_memory_s1_write -> video_memory:write
	wire  [31:0] mm_interconnect_1_video_memory_s1_writedata;                  // mm_interconnect_1:video_memory_s1_writedata -> video_memory:writedata
	wire         mm_interconnect_1_video_memory_s1_clken;                      // mm_interconnect_1:video_memory_s1_clken -> video_memory:clken
	wire         mm_interconnect_1_led_pio_s1_chipselect;                      // mm_interconnect_1:led_pio_s1_chipselect -> led_pio:chipselect
	wire  [31:0] mm_interconnect_1_led_pio_s1_readdata;                        // led_pio:readdata -> mm_interconnect_1:led_pio_s1_readdata
	wire   [1:0] mm_interconnect_1_led_pio_s1_address;                         // mm_interconnect_1:led_pio_s1_address -> led_pio:address
	wire         mm_interconnect_1_led_pio_s1_write;                           // mm_interconnect_1:led_pio_s1_write -> led_pio:write_n
	wire  [31:0] mm_interconnect_1_led_pio_s1_writedata;                       // mm_interconnect_1:led_pio_s1_writedata -> led_pio:writedata
	wire         mm_interconnect_1_hex_pio_s1_chipselect;                      // mm_interconnect_1:hex_pio_s1_chipselect -> hex_pio:chipselect
	wire  [31:0] mm_interconnect_1_hex_pio_s1_readdata;                        // hex_pio:readdata -> mm_interconnect_1:hex_pio_s1_readdata
	wire   [1:0] mm_interconnect_1_hex_pio_s1_address;                         // mm_interconnect_1:hex_pio_s1_address -> hex_pio:address
	wire         mm_interconnect_1_hex_pio_s1_write;                           // mm_interconnect_1:hex_pio_s1_write -> hex_pio:write_n
	wire  [31:0] mm_interconnect_1_hex_pio_s1_writedata;                       // mm_interconnect_1:hex_pio_s1_writedata -> hex_pio:writedata
	wire  [31:0] mm_interconnect_1_leftdial_pio_s1_readdata;                   // leftdial_pio:readdata -> mm_interconnect_1:leftdial_pio_s1_readdata
	wire   [1:0] mm_interconnect_1_leftdial_pio_s1_address;                    // mm_interconnect_1:leftdial_pio_s1_address -> leftdial_pio:address
	wire  [31:0] mm_interconnect_1_rightdial_pio_s1_readdata;                  // rightdial_pio:readdata -> mm_interconnect_1:rightdial_pio_s1_readdata
	wire   [1:0] mm_interconnect_1_rightdial_pio_s1_address;                   // mm_interconnect_1:rightdial_pio_s1_address -> rightdial_pio:address
	wire  [31:0] mm_interconnect_1_buttons_pio_s1_readdata;                    // buttons_pio:readdata -> mm_interconnect_1:buttons_pio_s1_readdata
	wire   [1:0] mm_interconnect_1_buttons_pio_s1_address;                     // mm_interconnect_1:buttons_pio_s1_address -> buttons_pio:address
	wire  [31:0] mm_interconnect_1_pixelstream_0_slave_parameters_readdata;    // PixelStream_0:avs_s0_readdata -> mm_interconnect_1:PixelStream_0_slave_parameters_readdata
	wire         mm_interconnect_1_pixelstream_0_slave_parameters_waitrequest; // PixelStream_0:avs_s0_waitrequest -> mm_interconnect_1:PixelStream_0_slave_parameters_waitrequest
	wire   [3:0] mm_interconnect_1_pixelstream_0_slave_parameters_address;     // mm_interconnect_1:PixelStream_0_slave_parameters_address -> PixelStream_0:avs_s0_address
	wire         mm_interconnect_1_pixelstream_0_slave_parameters_read;        // mm_interconnect_1:PixelStream_0_slave_parameters_read -> PixelStream_0:avs_s0_read
	wire         mm_interconnect_1_pixelstream_0_slave_parameters_write;       // mm_interconnect_1:PixelStream_0_slave_parameters_write -> PixelStream_0:avs_s0_write
	wire  [31:0] mm_interconnect_1_pixelstream_0_slave_parameters_writedata;   // mm_interconnect_1:PixelStream_0_slave_parameters_writedata -> PixelStream_0:avs_s0_writedata
	wire         clarvi_0_interrupt_receiver_0_irq;                            // irq_mapper:sender_irq -> clarvi_0:inr_irq
	wire         rst_controller_reset_out_reset;                               // rst_controller:reset_out -> [PixelStream_0:csi_clockreset_reset_n, buttons_pio:reset_n, clarvi_0:reset, hex_pio:reset_n, led_pio:reset_n, leftdial_pio:reset_n, mm_interconnect_0:clarvi_0_reset_reset_bridge_in_reset_reset, mm_interconnect_1:clarvi_0_reset_reset_bridge_in_reset_reset, onchip_memory2_0:reset, onchip_memory2_0:reset2, rightdial_pio:reset_n, rst_translator:in_reset, video_memory:reset]
	wire         rst_controller_reset_out_reset_req;                           // rst_controller:reset_req -> [onchip_memory2_0:reset_req, onchip_memory2_0:reset_req2, rst_translator:reset_req_in, video_memory:reset_req]

	mkPixelStream pixelstream_0 (
		.csi_clockreset_clk               (pll_outclk0_clk),                                              //        clockreset.clk
		.csi_clockreset_reset_n           (~rst_controller_reset_out_reset),                              //  clockreset_reset.reset_n
		.avs_s0_address                   (mm_interconnect_1_pixelstream_0_slave_parameters_address),     //  slave_parameters.address
		.avs_s0_writedata                 (mm_interconnect_1_pixelstream_0_slave_parameters_writedata),   //                  .writedata
		.avs_s0_write                     (mm_interconnect_1_pixelstream_0_slave_parameters_write),       //                  .write
		.avs_s0_read                      (mm_interconnect_1_pixelstream_0_slave_parameters_read),        //                  .read
		.avs_s0_readdata                  (mm_interconnect_1_pixelstream_0_slave_parameters_readdata),    //                  .readdata
		.avs_s0_waitrequest               (mm_interconnect_1_pixelstream_0_slave_parameters_waitrequest), //                  .waitrequest
		.avm_m0_readdata                  (pixelstream_0_master_burstreads_readdata),                     // master_burstreads.readdata
		.avm_m0_readdatavalid             (pixelstream_0_master_burstreads_readdatavalid),                //                  .readdatavalid
		.avm_m0_waitrequest               (pixelstream_0_master_burstreads_waitrequest),                  //                  .waitrequest
		.avm_m0_writedata                 (pixelstream_0_master_burstreads_writedata),                    //                  .writedata
		.avm_m0_address                   (pixelstream_0_master_burstreads_address),                      //                  .address
		.avm_m0_read                      (pixelstream_0_master_burstreads_read),                         //                  .read
		.avm_m0_write                     (pixelstream_0_master_burstreads_write),                        //                  .write
		.csi_video_clk                    (pll_outclk1_clk),                                              //             video.clk
		.coe_hdmi_r                       (pixelstream_0_conduit_end_0_lcd_red),                          //     conduit_end_0.lcd_red
		.coe_hdmi_g                       (pixelstream_0_conduit_end_0_lcd_green),                        //                  .lcd_green
		.coe_hdmi_b                       (pixelstream_0_conduit_end_0_lcd_blue),                         //                  .lcd_blue
		.coe_hdmi_hsd                     (pixelstream_0_conduit_end_0_lcd_hsync),                        //                  .lcd_hsync
		.coe_hdmi_vsd                     (pixelstream_0_conduit_end_0_lcd_vsync),                        //                  .lcd_vsync
		.coe_hdmi_de                      (pixelstream_0_conduit_end_0_lcd_de),                           //                  .lcd_de
		.csi_clockreset_clk_coe_hdmi_dclk (pixelstream_0_conduit_end_0_lcd_dclk),                         //                  .lcd_dclk
		.CLK_GATE_coe_hdmi_dclk           (pixelstream_0_conduit_end_0_lcd_dclk_en)                       //                  .lcd_dclk_en
	);

	clarvi_soc_buttons_pio buttons_pio (
		.clk      (pll_outclk0_clk),                           //                 clk.clk
		.reset_n  (~rst_controller_reset_out_reset),           //               reset.reset_n
		.address  (mm_interconnect_1_buttons_pio_s1_address),  //                  s1.address
		.readdata (mm_interconnect_1_buttons_pio_s1_readdata), //                    .readdata
		.in_port  (buttons_pio_external_connection_export)     // external_connection.export
	);

	clarvi_avalon #(
		.DATA_ADDR_WIDTH     (29),
		.INSTR_ADDR_WIDTH    (14),
		.INITIAL_PC          (0),
		.DEFAULT_TRAP_VECTOR (0)
	) clarvi_0 (
		.reset                   (rst_controller_reset_out_reset),    //                reset.reset
		.inr_irq                 (clarvi_0_interrupt_receiver_0_irq), // interrupt_receiver_0.irq
		.avm_main_address        (clarvi_0_main_address),             //                 main.address
		.avm_main_byteenable     (clarvi_0_main_byteenable),          //                     .byteenable
		.avm_main_read           (clarvi_0_main_read),                //                     .read
		.avm_main_readdata       (clarvi_0_main_readdata),            //                     .readdata
		.avm_main_write          (clarvi_0_main_write),               //                     .write
		.avm_main_writedata      (clarvi_0_main_writedata),           //                     .writedata
		.avm_main_waitrequest    (clarvi_0_main_waitrequest),         //                     .waitrequest
		.avm_main_readdatavalid  (clarvi_0_main_readdatavalid),       //                     .readdatavalid
		.avm_instr_address       (clarvi_0_instr_address),            //                instr.address
		.avm_instr_read          (clarvi_0_instr_read),               //                     .read
		.avm_instr_readdata      (clarvi_0_instr_readdata),           //                     .readdata
		.avm_instr_waitrequest   (clarvi_0_instr_waitrequest),        //                     .waitrequest
		.avm_instr_readdatavalid (clarvi_0_instr_readdatavalid),      //                     .readdatavalid
		.clock                   (pll_outclk0_clk),                   //                clock.clk
		.debug_register28        (),                                  //                debug.register28
		.debug_scratch           (),                                  //                     .scratch
		.debug_pc                ()                                   //                     .pc
	);

	clarvi_soc_hex_pio hex_pio (
		.clk        (pll_outclk0_clk),                         //                 clk.clk
		.reset_n    (~rst_controller_reset_out_reset),         //               reset.reset_n
		.address    (mm_interconnect_1_hex_pio_s1_address),    //                  s1.address
		.write_n    (~mm_interconnect_1_hex_pio_s1_write),     //                    .write_n
		.writedata  (mm_interconnect_1_hex_pio_s1_writedata),  //                    .writedata
		.chipselect (mm_interconnect_1_hex_pio_s1_chipselect), //                    .chipselect
		.readdata   (mm_interconnect_1_hex_pio_s1_readdata),   //                    .readdata
		.out_port   (hex_pio_external_connection_export)       // external_connection.export
	);

	clarvi_soc_led_pio led_pio (
		.clk        (pll_outclk0_clk),                         //                 clk.clk
		.reset_n    (~rst_controller_reset_out_reset),         //               reset.reset_n
		.address    (mm_interconnect_1_led_pio_s1_address),    //                  s1.address
		.write_n    (~mm_interconnect_1_led_pio_s1_write),     //                    .write_n
		.writedata  (mm_interconnect_1_led_pio_s1_writedata),  //                    .writedata
		.chipselect (mm_interconnect_1_led_pio_s1_chipselect), //                    .chipselect
		.readdata   (mm_interconnect_1_led_pio_s1_readdata),   //                    .readdata
		.out_port   (led_pio_external_connection_export)       // external_connection.export
	);

	clarvi_soc_leftdial_pio leftdial_pio (
		.clk      (pll_outclk0_clk),                            //                 clk.clk
		.reset_n  (~rst_controller_reset_out_reset),            //               reset.reset_n
		.address  (mm_interconnect_1_leftdial_pio_s1_address),  //                  s1.address
		.readdata (mm_interconnect_1_leftdial_pio_s1_readdata), //                    .readdata
		.in_port  (leftdial_pio_external_connection_export)     // external_connection.export
	);

	clarvi_soc_onchip_memory2_0 onchip_memory2_0 (
		.clk         (pll_outclk0_clk),                                  //   clk1.clk
		.address     (mm_interconnect_1_onchip_memory2_0_s1_address),    //     s1.address
		.clken       (mm_interconnect_1_onchip_memory2_0_s1_clken),      //       .clken
		.chipselect  (mm_interconnect_1_onchip_memory2_0_s1_chipselect), //       .chipselect
		.write       (mm_interconnect_1_onchip_memory2_0_s1_write),      //       .write
		.readdata    (mm_interconnect_1_onchip_memory2_0_s1_readdata),   //       .readdata
		.writedata   (mm_interconnect_1_onchip_memory2_0_s1_writedata),  //       .writedata
		.byteenable  (mm_interconnect_1_onchip_memory2_0_s1_byteenable), //       .byteenable
		.reset       (rst_controller_reset_out_reset),                   // reset1.reset
		.reset_req   (rst_controller_reset_out_reset_req),               //       .reset_req
		.address2    (mm_interconnect_0_onchip_memory2_0_s2_address),    //     s2.address
		.chipselect2 (mm_interconnect_0_onchip_memory2_0_s2_chipselect), //       .chipselect
		.clken2      (mm_interconnect_0_onchip_memory2_0_s2_clken),      //       .clken
		.write2      (mm_interconnect_0_onchip_memory2_0_s2_write),      //       .write
		.readdata2   (mm_interconnect_0_onchip_memory2_0_s2_readdata),   //       .readdata
		.writedata2  (mm_interconnect_0_onchip_memory2_0_s2_writedata),  //       .writedata
		.byteenable2 (mm_interconnect_0_onchip_memory2_0_s2_byteenable), //       .byteenable
		.clk2        (pll_outclk0_clk),                                  //   clk2.clk
		.reset2      (rst_controller_reset_out_reset),                   // reset2.reset
		.reset_req2  (rst_controller_reset_out_reset_req),               //       .reset_req
		.freeze      (1'b0)                                              // (terminated)
	);

	clarvi_soc_pll pll (
		.refclk   (clk_clk),         //  refclk.clk
		.rst      (~reset_reset_n),  //   reset.reset
		.outclk_0 (pll_outclk0_clk), // outclk0.clk
		.outclk_1 (pll_outclk1_clk), // outclk1.clk
		.locked   ()                 // (terminated)
	);

	clarvi_soc_leftdial_pio rightdial_pio (
		.clk      (pll_outclk0_clk),                             //                 clk.clk
		.reset_n  (~rst_controller_reset_out_reset),             //               reset.reset_n
		.address  (mm_interconnect_1_rightdial_pio_s1_address),  //                  s1.address
		.readdata (mm_interconnect_1_rightdial_pio_s1_readdata), //                    .readdata
		.in_port  (rightdial_pio_external_connection_export)     // external_connection.export
	);

	clarvi_soc_video_memory video_memory (
		.clk        (pll_outclk0_clk),                              //   clk1.clk
		.address    (mm_interconnect_1_video_memory_s1_address),    //     s1.address
		.clken      (mm_interconnect_1_video_memory_s1_clken),      //       .clken
		.chipselect (mm_interconnect_1_video_memory_s1_chipselect), //       .chipselect
		.write      (mm_interconnect_1_video_memory_s1_write),      //       .write
		.readdata   (mm_interconnect_1_video_memory_s1_readdata),   //       .readdata
		.writedata  (mm_interconnect_1_video_memory_s1_writedata),  //       .writedata
		.byteenable (mm_interconnect_1_video_memory_s1_byteenable), //       .byteenable
		.reset      (rst_controller_reset_out_reset),               // reset1.reset
		.reset_req  (rst_controller_reset_out_reset_req),           //       .reset_req
		.freeze     (1'b0)                                          // (terminated)
	);

	clarvi_soc_mm_interconnect_0 mm_interconnect_0 (
		.pll_outclk0_clk                            (pll_outclk0_clk),                                  //                          pll_outclk0.clk
		.clarvi_0_reset_reset_bridge_in_reset_reset (rst_controller_reset_out_reset),                   // clarvi_0_reset_reset_bridge_in_reset.reset
		.clarvi_0_instr_address                     (clarvi_0_instr_address),                           //                       clarvi_0_instr.address
		.clarvi_0_instr_waitrequest                 (clarvi_0_instr_waitrequest),                       //                                     .waitrequest
		.clarvi_0_instr_read                        (clarvi_0_instr_read),                              //                                     .read
		.clarvi_0_instr_readdata                    (clarvi_0_instr_readdata),                          //                                     .readdata
		.clarvi_0_instr_readdatavalid               (clarvi_0_instr_readdatavalid),                     //                                     .readdatavalid
		.onchip_memory2_0_s2_address                (mm_interconnect_0_onchip_memory2_0_s2_address),    //                  onchip_memory2_0_s2.address
		.onchip_memory2_0_s2_write                  (mm_interconnect_0_onchip_memory2_0_s2_write),      //                                     .write
		.onchip_memory2_0_s2_readdata               (mm_interconnect_0_onchip_memory2_0_s2_readdata),   //                                     .readdata
		.onchip_memory2_0_s2_writedata              (mm_interconnect_0_onchip_memory2_0_s2_writedata),  //                                     .writedata
		.onchip_memory2_0_s2_byteenable             (mm_interconnect_0_onchip_memory2_0_s2_byteenable), //                                     .byteenable
		.onchip_memory2_0_s2_chipselect             (mm_interconnect_0_onchip_memory2_0_s2_chipselect), //                                     .chipselect
		.onchip_memory2_0_s2_clken                  (mm_interconnect_0_onchip_memory2_0_s2_clken)       //                                     .clken
	);

	clarvi_soc_mm_interconnect_1 mm_interconnect_1 (
		.pll_outclk0_clk                               (pll_outclk0_clk),                                              //                          pll_outclk0.clk
		.clarvi_0_reset_reset_bridge_in_reset_reset    (rst_controller_reset_out_reset),                               // clarvi_0_reset_reset_bridge_in_reset.reset
		.clarvi_0_main_address                         (clarvi_0_main_address),                                        //                        clarvi_0_main.address
		.clarvi_0_main_waitrequest                     (clarvi_0_main_waitrequest),                                    //                                     .waitrequest
		.clarvi_0_main_byteenable                      (clarvi_0_main_byteenable),                                     //                                     .byteenable
		.clarvi_0_main_read                            (clarvi_0_main_read),                                           //                                     .read
		.clarvi_0_main_readdata                        (clarvi_0_main_readdata),                                       //                                     .readdata
		.clarvi_0_main_readdatavalid                   (clarvi_0_main_readdatavalid),                                  //                                     .readdatavalid
		.clarvi_0_main_write                           (clarvi_0_main_write),                                          //                                     .write
		.clarvi_0_main_writedata                       (clarvi_0_main_writedata),                                      //                                     .writedata
		.PixelStream_0_master_burstreads_address       (pixelstream_0_master_burstreads_address),                      //      PixelStream_0_master_burstreads.address
		.PixelStream_0_master_burstreads_waitrequest   (pixelstream_0_master_burstreads_waitrequest),                  //                                     .waitrequest
		.PixelStream_0_master_burstreads_read          (pixelstream_0_master_burstreads_read),                         //                                     .read
		.PixelStream_0_master_burstreads_readdata      (pixelstream_0_master_burstreads_readdata),                     //                                     .readdata
		.PixelStream_0_master_burstreads_readdatavalid (pixelstream_0_master_burstreads_readdatavalid),                //                                     .readdatavalid
		.PixelStream_0_master_burstreads_write         (pixelstream_0_master_burstreads_write),                        //                                     .write
		.PixelStream_0_master_burstreads_writedata     (pixelstream_0_master_burstreads_writedata),                    //                                     .writedata
		.buttons_pio_s1_address                        (mm_interconnect_1_buttons_pio_s1_address),                     //                       buttons_pio_s1.address
		.buttons_pio_s1_readdata                       (mm_interconnect_1_buttons_pio_s1_readdata),                    //                                     .readdata
		.hex_pio_s1_address                            (mm_interconnect_1_hex_pio_s1_address),                         //                           hex_pio_s1.address
		.hex_pio_s1_write                              (mm_interconnect_1_hex_pio_s1_write),                           //                                     .write
		.hex_pio_s1_readdata                           (mm_interconnect_1_hex_pio_s1_readdata),                        //                                     .readdata
		.hex_pio_s1_writedata                          (mm_interconnect_1_hex_pio_s1_writedata),                       //                                     .writedata
		.hex_pio_s1_chipselect                         (mm_interconnect_1_hex_pio_s1_chipselect),                      //                                     .chipselect
		.led_pio_s1_address                            (mm_interconnect_1_led_pio_s1_address),                         //                           led_pio_s1.address
		.led_pio_s1_write                              (mm_interconnect_1_led_pio_s1_write),                           //                                     .write
		.led_pio_s1_readdata                           (mm_interconnect_1_led_pio_s1_readdata),                        //                                     .readdata
		.led_pio_s1_writedata                          (mm_interconnect_1_led_pio_s1_writedata),                       //                                     .writedata
		.led_pio_s1_chipselect                         (mm_interconnect_1_led_pio_s1_chipselect),                      //                                     .chipselect
		.leftdial_pio_s1_address                       (mm_interconnect_1_leftdial_pio_s1_address),                    //                      leftdial_pio_s1.address
		.leftdial_pio_s1_readdata                      (mm_interconnect_1_leftdial_pio_s1_readdata),                   //                                     .readdata
		.onchip_memory2_0_s1_address                   (mm_interconnect_1_onchip_memory2_0_s1_address),                //                  onchip_memory2_0_s1.address
		.onchip_memory2_0_s1_write                     (mm_interconnect_1_onchip_memory2_0_s1_write),                  //                                     .write
		.onchip_memory2_0_s1_readdata                  (mm_interconnect_1_onchip_memory2_0_s1_readdata),               //                                     .readdata
		.onchip_memory2_0_s1_writedata                 (mm_interconnect_1_onchip_memory2_0_s1_writedata),              //                                     .writedata
		.onchip_memory2_0_s1_byteenable                (mm_interconnect_1_onchip_memory2_0_s1_byteenable),             //                                     .byteenable
		.onchip_memory2_0_s1_chipselect                (mm_interconnect_1_onchip_memory2_0_s1_chipselect),             //                                     .chipselect
		.onchip_memory2_0_s1_clken                     (mm_interconnect_1_onchip_memory2_0_s1_clken),                  //                                     .clken
		.PixelStream_0_slave_parameters_address        (mm_interconnect_1_pixelstream_0_slave_parameters_address),     //       PixelStream_0_slave_parameters.address
		.PixelStream_0_slave_parameters_write          (mm_interconnect_1_pixelstream_0_slave_parameters_write),       //                                     .write
		.PixelStream_0_slave_parameters_read           (mm_interconnect_1_pixelstream_0_slave_parameters_read),        //                                     .read
		.PixelStream_0_slave_parameters_readdata       (mm_interconnect_1_pixelstream_0_slave_parameters_readdata),    //                                     .readdata
		.PixelStream_0_slave_parameters_writedata      (mm_interconnect_1_pixelstream_0_slave_parameters_writedata),   //                                     .writedata
		.PixelStream_0_slave_parameters_waitrequest    (mm_interconnect_1_pixelstream_0_slave_parameters_waitrequest), //                                     .waitrequest
		.rightdial_pio_s1_address                      (mm_interconnect_1_rightdial_pio_s1_address),                   //                     rightdial_pio_s1.address
		.rightdial_pio_s1_readdata                     (mm_interconnect_1_rightdial_pio_s1_readdata),                  //                                     .readdata
		.video_memory_s1_address                       (mm_interconnect_1_video_memory_s1_address),                    //                      video_memory_s1.address
		.video_memory_s1_write                         (mm_interconnect_1_video_memory_s1_write),                      //                                     .write
		.video_memory_s1_readdata                      (mm_interconnect_1_video_memory_s1_readdata),                   //                                     .readdata
		.video_memory_s1_writedata                     (mm_interconnect_1_video_memory_s1_writedata),                  //                                     .writedata
		.video_memory_s1_byteenable                    (mm_interconnect_1_video_memory_s1_byteenable),                 //                                     .byteenable
		.video_memory_s1_chipselect                    (mm_interconnect_1_video_memory_s1_chipselect),                 //                                     .chipselect
		.video_memory_s1_clken                         (mm_interconnect_1_video_memory_s1_clken)                       //                                     .clken
	);

	clarvi_soc_irq_mapper irq_mapper (
		.clk        (),                                  //       clk.clk
		.reset      (),                                  // clk_reset.reset
		.sender_irq (clarvi_0_interrupt_receiver_0_irq)  //    sender.irq
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("deassert"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (1),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller (
		.reset_in0      (~reset_reset_n),                     // reset_in0.reset
		.clk            (pll_outclk0_clk),                    //       clk.clk
		.reset_out      (rst_controller_reset_out_reset),     // reset_out.reset
		.reset_req      (rst_controller_reset_out_reset_req), //          .reset_req
		.reset_req_in0  (1'b0),                               // (terminated)
		.reset_in1      (1'b0),                               // (terminated)
		.reset_req_in1  (1'b0),                               // (terminated)
		.reset_in2      (1'b0),                               // (terminated)
		.reset_req_in2  (1'b0),                               // (terminated)
		.reset_in3      (1'b0),                               // (terminated)
		.reset_req_in3  (1'b0),                               // (terminated)
		.reset_in4      (1'b0),                               // (terminated)
		.reset_req_in4  (1'b0),                               // (terminated)
		.reset_in5      (1'b0),                               // (terminated)
		.reset_req_in5  (1'b0),                               // (terminated)
		.reset_in6      (1'b0),                               // (terminated)
		.reset_req_in6  (1'b0),                               // (terminated)
		.reset_in7      (1'b0),                               // (terminated)
		.reset_req_in7  (1'b0),                               // (terminated)
		.reset_in8      (1'b0),                               // (terminated)
		.reset_req_in8  (1'b0),                               // (terminated)
		.reset_in9      (1'b0),                               // (terminated)
		.reset_req_in9  (1'b0),                               // (terminated)
		.reset_in10     (1'b0),                               // (terminated)
		.reset_req_in10 (1'b0),                               // (terminated)
		.reset_in11     (1'b0),                               // (terminated)
		.reset_req_in11 (1'b0),                               // (terminated)
		.reset_in12     (1'b0),                               // (terminated)
		.reset_req_in12 (1'b0),                               // (terminated)
		.reset_in13     (1'b0),                               // (terminated)
		.reset_req_in13 (1'b0),                               // (terminated)
		.reset_in14     (1'b0),                               // (terminated)
		.reset_req_in14 (1'b0),                               // (terminated)
		.reset_in15     (1'b0),                               // (terminated)
		.reset_req_in15 (1'b0)                                // (terminated)
	);

endmodule
