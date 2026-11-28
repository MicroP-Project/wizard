// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// A simple clock divider that takes the HSOSC input
// Submodule for a 50% duty-cycle clock divider
module clk_div 
	#(parameter int DIVISOR = 16  // # of clk_in ticks in 1 period of clk_out. Must be an even integer >= 2 (48 MHz / 16 = 3 MHz)
	) (
    input  logic clk_in,
    input  logic nrst,
    output logic clk_out
);
    localparam int HALF_DIV = DIVISOR / 2; // # of clk_in ticks until clk_out will toggle, must be an integer >= 1
    localparam int WIDTH    = (HALF_DIV > 1) ? $clog2(HALF_DIV) : 1; // width of counter, clog2 calculates the minimum number of bits needed to count to HALF_DIV (except edge case half_div is 1, where width = 1)

    logic [WIDTH-1:0] count;

    always_ff @(posedge clk_in) begin
        if (~nrst) begin
            count   <= '0;
            clk_out <= 1'b0; // clk_out starts low on reset
        end 
		else if (count >= HALF_DIV - 1) begin
            count   <= '0;
            clk_out <= ~clk_out;
        end 
		else begin
            count <= count + 1'b1;
        end
    end

endmodule
