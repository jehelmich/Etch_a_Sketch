// rotary decoder template
module rotary
  (
	input  wire clk,
	input  wire rst,
	input  wire [1:0] rotary_in,
	output logic [7:0] rotary_pos,
    output logic rot_cw,
    output logic rot_ccw
   );
   
		/* Add wire and register definitions */
		wire [1:0] db_rot_in;
		reg [1:0] prev_rot;

        /* Instantiate debouncing components */

		debounce db1(.clk(clk),.rst(0),.bouncy_in(rotary_in[0]),.clean_out(db_rot_in[0]));
		debounce db2(.clk(clk),.rst(0),.bouncy_in(rotary_in[1]),.clean_out(db_rot_in[1]));

        /* Synchronous output value manipulation logic */
		always_ff @(posedge rst or posedge clk)
			if(rst)
				begin
					rotary_pos <= 0;
					prev_rot[0] <= 0;
					prev_rot[1] <= 0;
					rot_cw <= 1'b0;
					rot_ccw <= 1'b0;
				end
			else if (db_rot_in[0] != prev_rot[0] || db_rot_in[1] != prev_rot[1])
				begin
				prev_rot <= db_rot_in;
				if (db_rot_in == 2'b00)
					if (prev_rot == 2'b10)
						begin
							rot_ccw <= 1;
							rotary_pos <= rotary_pos + 1;
						end
					else
						begin
							rot_cw <= 1;
							rotary_pos <= rotary_pos - 1;
						end
				else if (db_rot_in == 2'b01)
						if (prev_rot == 2'b00)
							begin
								rot_ccw <= 1;
							end
						else
							begin
								rot_cw <= 1;
							end
				else if (db_rot_in == 2'b11)
						if (prev_rot == 2'b01)
							begin
								rot_ccw <= 1;
							end
						else
							begin
								rot_cw <= 1;
							end
				else
						if (prev_rot == 2'b11)
							begin
								rot_ccw <= 1;
							end
						else
							begin
								rot_cw <= 1;
							end
				end
			else
				begin
				rot_ccw <= 1'b0;
				rot_cw <= 1'b0;
				end
					
					
		
		
			

endmodule // rotarydecoder
