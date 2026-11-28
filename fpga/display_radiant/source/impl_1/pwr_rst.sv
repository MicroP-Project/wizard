// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/28/2026
// wait time when power on/reset to make sure everything is stabalized before moving on (SPRAM has a brief wake up delay

module pwr_rst #(
	parameter WIDTH = 16,
	parameter MAX_COUNT = 65536 // count for 48 Mhz means a ~1.365 ms hold
	) (
	input   logic   clk, nrst_in,
	output  logic nrst_out
	);
	
	logic   [WIDTH-1:0] cnt;
	assign  nrst_out = (cnt == MAX_COUNT-1); // reset released once counter reaches max count
	
	counter #(
        .WIDTH     (WIDTH),
        .MAX_COUNT (MAX_COUNT)
    ) u_pwr_cnt (
        .clk   (clk),
        .nrst  (nrst_in),
        .en    (~nrst_out),
        .count (cnt)
    );

endmodule