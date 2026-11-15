// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 static row driver: lights up Row 0 in solid red to verify HUB75 wiring & 5V power

module display_s1 (
    input  logic clk_shift,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
    // Drive only upper-half Red (R1) HIGH; keep all other color channels OFF
    assign R1 = 1'b1;
    assign G1 = 1'b0;
    assign B1 = 1'b0;

    assign R2 = 1'b0;
    assign G2 = 1'b0;
    assign B2 = 1'b0;

    // Row address 00000 selects Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;

    // Continuously toggle CLK to fill the 64-bit shift register with '1's,
    // keep LAT HIGH (transparent) and OE LOW (active-low output enabled)
    assign CLK = clk_shift;
    assign LAT = 1'b1;
    assign OE  = 1'b0;

endmodule