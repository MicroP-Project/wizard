// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |
// drives one row-pair top half and bottom half (64 shift clocks + 1 latch clock + 64 dwell clocks)

module row_driver (
    input  logic       clk_shift,
    input  logic       nrst,
    input  logic       display_en,
    input  logic [2:0] top_rgb,
    input  logic [2:0] bot_rgb,
    output logic [5:0] col_addr,
    output logic       row_done,
    output logic       R1, G1, B1,
    output logic       R2, G2, B2,
    output logic       CLK,
    output logic       LAT,
    output logic       OE
);
    logic [7:0] cnt;

    // Count 0..128 on negedge clk_shift (~clk_shift) so data settles before posedge CLK
    counter #(
        .WIDTH     (8),
        .MAX_COUNT (129)
    ) u_col_cnt (
        .clk   (~clk_shift),
        .nrst  (nrst),
        .en    (1'b1),
        .count (cnt)
    );

    wire shifting = (cnt < 8'd64); // still shifting info bits into the row of 64 pixels
    wire dwelling = (cnt >= 8'd65); // hold OE (turn LEDs on for that row for as long as this)

    assign col_addr     = cnt[5:0];
    assign row_done     = (cnt == 8'd128);
    assign CLK          = shifting ? clk_shift : 1'b0;
    assign LAT          = (cnt == 8'd64);
    assign OE           = (dwelling && display_en) ? 1'b0 : 1'b1;

    assign {R1, G1, B1} = shifting ? top_rgb : 3'b000;
    assign {R2, G2, B2} = shifting ? bot_rgb : 3'b000;

endmodule
