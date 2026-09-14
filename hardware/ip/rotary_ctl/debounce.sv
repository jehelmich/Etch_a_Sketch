// The input must hold steady for 2**COUNTER_WIDTH clocks before a new value is
// accepted. At 50 MHz the default of 15 gives about 655 us, comfortably longer
// than the contacts bounce. Simulation overrides it to keep runs short.
module debounce #(
        parameter COUNTER_WIDTH = 15
) (
        input wire       clk,       // 50MHz clock input
        input wire       rst,       // reset input (positive)
        input wire       bouncy_in, // bouncy asynchronous input
        output reg       clean_out  // clean debounced 
   );

        /* Add wire and register definitions */
	reg prev_syncbouncy;
	reg metastable;
	reg syncbouncy;
	reg [15:0] numbounces;
	reg [COUNTER_WIDTH-1:0] counter;
	wire counterAtMax = &counter;
        /* Add synchronous debouncing logic */
	always_ff @(posedge clk or posedge rst)
		if(rst) begin
			counter <= 0;
			numbounces <= 0;
			prev_syncbouncy <= 0;
			syncbouncy <= 0;
			metastable <= 0;
			clean_out <= 0;
		end else begin
			metastable <= bouncy_in;
			syncbouncy <= metastable;
			prev_syncbouncy <= syncbouncy;
			if (syncbouncy != prev_syncbouncy) begin // detect change
				counter <= 0;
				numbounces <= numbounces+1;
			end else if (!counterAtMax) // no bouncing, so keep counting
				counter <= counter+1;
			else // output clean signal
				clean_out <= syncbouncy;
		end
endmodule
