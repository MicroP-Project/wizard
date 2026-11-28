// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/28/2026
// Top-level wrapper for Target 1 continuous 64x64 right-arrow display

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
    logic        int_osc;
    logic        nrst_sync;
    logic [15:0] por_cnt = 16'd0;
    logic        nrst_int;
    logic        clk_shift;

    logic [4:0]  row_addr;
    logic [5:0]  col_addr;
    logic [2:0]  top_rgb;
    logic [2:0]  bot_rgb;
	
	// **************** SUBMODULE INSTANTIATIONS *******************************
    // 1. 48 MHz internal high-speed oscillator
    HSOSC #(
        .CLKHF_DIV("0b00")
    ) u_hfosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );

    // 2. 2-FF synchronizer on Pin 34 external reset button
    sync #(.WIDTH(1)) u_sync (
        .clk(int_osc),
        .d(nrst_in),
        .q(nrst_sync)
    );

    // 3. Power-On Reset + Button Reset (~1.36 ms hold @ 48 MHz)
    always_ff @(posedge int_osc) begin
        if (!nrst_sync)
            por_cnt <= 16'd0;
        else if (por_cnt != 16'hFFFF)
            por_cnt <= por_cnt + 16'd1;
    end

    assign nrst_int = (por_cnt == 16'hFFFF);

    // 4. Divide 48 MHz down to 3 MHz shift clock
    clk_div #(
        .DIVISOR(16)
    ) u_clk_div (
        .clk_in(int_osc),
        .nrst(nrst_int),
        .clk_out(clk_shift)
    );

    // 5. Pixel source 
    fake_SPRAM u_fake_SPRAM (
        .row_addr(row_addr),
        .col_addr(col_addr),
        .top_rgb(top_rgb),
        .bot_rgb(bot_rgb)
    );

    // 6. 64x64 HUB75 LED matrix driver
    matrix_driver u_matrix_driver (
        .clk_shift(clk_shift),
        .nrst(nrst_int),
        .display_en(1'b1),
        .top_rgb(top_rgb),
        .bot_rgb(bot_rgb),
        .row_addr(row_addr),
        .col_addr(col_addr),
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .A(A), .B(B), .C(C), .D(D), .E(E),
        .CLK(CLK),
        .LAT(LAT),
        .OE(OE)
    );

endmodule
