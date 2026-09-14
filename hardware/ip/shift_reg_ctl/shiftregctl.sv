module shiftregctl (
  input logic clock_50m,
  input logic reset,
  output logic shiftreg_clk,
  output logic shiftreg_loadn,
  input logic shiftreg_out,
  output reg [15:0] buttons
);
  logic [4:0] state;
  logic [15:0] tmp;
  logic clk;
  
  logic [8:0] clk_div;
  
  logic last_clk;
  logic clk_inhibit;
  
  always_comb begin
    // states 0 and 16..18 are the idle and load phases, when the shift
    // register must not be clocked
    clk_inhibit = (state == 0 || state == 16 || state == 17 || state == 18);
    shiftreg_clk = clk && !clk_inhibit;
    shiftreg_loadn = !(state==17);
  end
  
  always_ff @(posedge clock_50m)
    if (reset)
      begin
        state <= 16;
        tmp <= 0;
      end
    else
      begin
        clk_div <= clk_div + 1;
  	last_clk <= clk;
  	clk <= clk_div[8];
        if(last_clk && !clk) begin
            if(state==18) begin
                state <= 0;
                buttons <= tmp;
                tmp <= 0;
            end
            else begin
                state <= state + 1;
                // only states 0..15 carry a data bit; 16 and 17 load
                if (state < 16)
                    tmp[state[3:0]] <= shiftreg_out;
            end
        end
      end
  
endmodule
