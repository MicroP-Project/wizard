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
	//**************** internal connections declarations ***********************
    logic int_osc;
    logic nrst_sync;
    logic clk_shift;

    // **************** SUBMODULE INSTANTIATIONS *******************************
	 // Internal high-speed oscillator (48 MHz) clk generated in FPGA by HSOSC primitive
    HSOSC #(
        .CLKHF_DIV("0b00")
    ) u_hfosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );

    // 2-FF synchronizer on Pin 34 (nrst_in)
    sync #(.WIDTH(1)) u_sync (
        .clk(int_osc),
        .d(nrst_in),
        .q(nrst_sync)
    );

    // Divide 48 MHz down to 3 MHz shift clock
    clk_div #(.DIVISOR(16)) u_clk_div (
        .clk_in(int_osc),
        .nrst(nrst_sync),
        .clk_out(clk_shift)
    );

    // Stage 1 static driver with FM6124 initialization
    display_s1 u_display_s1 (
        .clk_shift(clk_shift),
        .nrst(nrst_sync),
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .A(A), .B(B), .C(C), .D(D), .E(E),
        .CLK(CLK),
        .LAT(LAT),
        .OE(OE)
    );

endmodule
