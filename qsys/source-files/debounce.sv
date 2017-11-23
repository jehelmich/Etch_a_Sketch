module debounce (
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
	reg [14:0] counter;
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