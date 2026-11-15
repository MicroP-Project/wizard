// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026

// Stage 1 top-level wrapper with guaranteed internal power-on reset
module display_top (
    input  logic nrst_in,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
    logic int_osc;
    logic clk_shift;

    // Internal 48 MHz high-speed oscillator
    HSOSC #(
        .CLKHF_DIV("0b00")
    ) u_hfosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );

    // Internal Power-On Reset (holds reset low for ~1.36 ms after HSOSC starts,
    // and also resets whenever external nrst_in is pulled low)
    logic [15:0] por_cnt = 16'd0;
    logic        nrst_int;
	logic nrst_sync;

	sync #(.WIDTH(1)) u_sync (
		.clk(int_osc),
		.d(nrst_in),
		.q(nrst_sync)
	);

	always_ff @(posedge int_osc) begin
		if (!nrst_sync)
			por_cnt <= 16'd0;
		else if (por_cnt != 16'hFFFF)
			por_cnt <= por_cnt + 16'd1;
	end

    assign nrst_int = (por_cnt == 16'hFFFF);

    // Divide 48 MHz down to 3 MHz shift clock
    clk_div #(.DIVISOR(16)) u_clk_div (
        .clk_in(int_osc),
        .nrst(nrst_int),
        .clk_out(clk_shift)
    );

    // Stage 1 static driver with FM6124 initialization
    display_s1 u_display_s1 (
        .clk_shift(clk_shift),
        .nrst(nrst_int),
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .A(A), .B(B), .C(C), .D(D), .E(E),
        .CLK(CLK),
        .LAT(LAT),
        .OE(OE)
    );

endmodule

/* module display_top(
    input  logic nrst_in,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
   //**************** internal connections declarations ***********************
   logic int_osc;
   logic nrst_sync;
   logic clk_3m;

   // **************** SUBMODULE INSTANTIATIONS *******************************
	 // 1. Internal high-speed oscillator (48 MHz) clk generated in FPGA by HSOSC primitive
	 HSOSC hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(int_osc));

   // 2. Synchronize external active-low reset into the 48 MHz clock domain
   sync #(
      .WIDTH(1)
   ) u_rst_sync (
       .clk(int_osc),
       .d  (nrst_in),
       .q  (nrst_sync)
   );

   // 3. Divide 48 MHz down to 3 MHz shift clock
   clk_div #(
      .DIVISOR(16)
   ) u_clk_div (
       .clk_in (int_osc),
       .nrst   (nrst_sync),
       .clk_out(clk_3m)
   );

   // 4. Stage 1 static row driver
   display_s1 u_display_s1 (
        .clk_shift(clk_shift),
        .nrst(nrst_sync),        // <--- ONLY NEW LINE ADDED
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .A(A), .B(B), .C(C), .D(D), .E(E),
        .CLK(CLK),
        .LAT(LAT),
        .OE(OE)
    );

endmodule */


