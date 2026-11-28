// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |

module counter
	#(parameter WIDTH = 25,
		MAX_COUNT = 10_000_000) (
	input   logic   clk, nrst, en,
	output  logic   [WIDTH-1:0] count
);
	
	always_ff @(posedge clk) begin
			if (~nrst) begin
				count <= '0;
				end
			else if (en) begin
				if (count >= (MAX_COUNT-1)) begin //for exact timing MAX_COUNT - 1 to account for the cycle it takes to register that it hit max
					count <= '0;
				end
				else begin
					count <= count + 1'b1;
				end
			end
	end

endmodule