// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/28/2026
// 64x64 HUB75 LED matrix scanner (uses row_driver submodule to drive 32 row pairings)

module matrix_driver (
    input  logic       clk_shift,
    input  logic       nrst,
    input  logic       display_en,
    input  logic [2:0] top_rgb,
    input  logic [2:0] bot_rgb,
    output logic [4:0] row_addr,
    output logic [5:0] col_addr,
    output logic       R1, G1, B1,
    output logic       R2, G2, B2,
    output logic       A, B, C, D, E,
    output logic       CLK,
    output logic       LAT,
    output logic       OE
);
    logic row_done;

    // Advance row_addr (0..31) each time row_driver finishes a 129-clock row cycle
    counter #(
        .WIDTH     (5),
        .MAX_COUNT (32)
    ) u_row_cnt (
        .clk   (~clk_shift),
        .nrst  (nrst),
        .en    (row_done),
        .count (row_addr)
    );

    assign {E, D, C, B, A} = row_addr;

    row_driver u_row_driver (
        .clk_shift  (clk_shift),
        .nrst       (nrst),
        .display_en (display_en),
        .top_rgb    (top_rgb),
        .bot_rgb    (bot_rgb),
        .col_addr   (col_addr),
        .row_done   (row_done),
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .CLK        (CLK),
        .LAT        (LAT),
        .OE         (OE)
    );

endmodule