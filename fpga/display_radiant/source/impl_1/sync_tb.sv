// Jessica Li  |  jesli@g.hmc.edu  |  09/23/2026
// This is a test bench for the sync submodule 

`timescale 1 ns/1 ns

module sync_tb();
	// var declaration
	logic   osc;
	logic 	[3:0] d;
	logic   [3:0] q;
	
	// instantiation dut
	sync dut(.clk(osc), .d, .q );
	
	// generate fake 50 MHz clock for testing purposes
	always begin
		osc = 0;
		#10;
		osc = 1;
		#10;
	end
	
	// start timed tests
	initial begin
		d = 3'b000;
		#20; 
		d = 3'b010;
		#20 // offset so not on clk tick
		d = 3'b111;
		#40
		d = 3'b001;
		#40
		
		$stop;
	end
endmodule