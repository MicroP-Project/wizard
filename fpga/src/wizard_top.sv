// wizard_top.sv
// Ellen Yu & Jessica Li  |  ellyu@g.hmc.edu & jesli@g.hmc.edu  |  11/16/2026
//
// Top level that joins the MCU -> FPGA SPI link (fpga_spi.sv) with the HUB75
// display driver (display_s1.sv). The panel stays dark until the MCU reports a
// +x cast, then shows the display_s1 pattern for SHOW_CYCLES clocks (1 s default).
// Another +x cast while it is lit restarts the timer.
//
// MCU side: mcu/src/spi_test.c sends packet 16'h0001 when user_input == 1 (+x).
//
// How the display is gated: display_s1 keeps running its FM6124 init + shift
// sequence the whole time (so the driver chips stay configured), and this module
// only forces OE high (OE is active low = all LEDs off) while no cast is active.

module wizard_top #(
    parameter logic [15:0] CAST_PLUS_X = 16'h0001,     // packet that means "+x cast"
    parameter int          SHOW_CYCLES = 48_000_000    // how long to light the panel (48 MHz clks)
) (
    input  logic nrst_in,                  // active-low reset (P34)

    // SPI from the MCU (SPI3: SCK=PB3, MOSI=PB5, MISO=PB4, CS=PA8)
    input  logic sck,
    input  logic sdi,
    output logic sdo,
    input  logic cs,

    // HUB75 panel
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE,

    output logic led                       // debug: high while the panel is enabled
);
    //**************** internal connections ***********************************
    logic        int_osc;
    logic        nrst_sync;
    logic        clk_shift;
    logic        oe_display;               // OE as display_s1 wants it
    logic [15:0] packet;
    logic [2:0]  cs_sync;
    logic        cs_rise;
    logic [$clog2(SHOW_CYCLES+1)-1:0] show_count;
    logic        show;

    //**************** clock + reset (same as display_top) ********************
    HSOSC #(.CLKHF_DIV("0b00")) u_hfosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(int_osc)
    );

    sync #(.WIDTH(1)) u_sync (
        .clk(int_osc),
        .d(nrst_in),
        .q(nrst_sync)
    );

    clk_div #(.DIVISOR(16)) u_clk_div (
        .clk_in(int_osc),
        .nrst(nrst_sync),
        .clk_out(clk_shift)
    );

    //**************** display driver *****************************************
    display_s1 u_display_s1 (
        .clk_shift(clk_shift),
        .nrst(nrst_sync),
        .R1(R1), .G1(G1), .B1(B1),
        .R2(R2), .G2(G2), .B2(B2),
        .A(A), .B(B), .C(C), .D(D), .E(E),
        .CLK(CLK),
        .LAT(LAT),
        .OE(oe_display)
    );

    //**************** SPI receiver *******************************************
    // packet updates on the rising edge of cs (end of each MCU transaction)
    fpga_spi #(.N(16)) u_spi (
        .sck(sck),
        .sdi(sdi),
        .sdo(sdo),
        .cs(cs),
        .packet(packet)
    );

    // bring cs into the 48 MHz domain: 2 flops for metastability + 1 for edge detect.
    // By the time cs_rise fires, packet was latched a couple of clocks ago and is stable.
    always_ff @(posedge int_osc)
        cs_sync <= {cs_sync[1:0], cs};

    assign cs_rise = cs_sync[1] & ~cs_sync[2];

    //**************** +x cast -> enable timer ********************************
    always_ff @(posedge int_osc) begin
        if (!nrst_sync)
            show_count <= '0;
        else if (cs_rise && packet == CAST_PLUS_X)
            show_count <= SHOW_CYCLES - 1;
        else if (show_count != 0)
            show_count <= show_count - 1'b1;
    end

    assign show = (show_count != 0);

    // OE is active low: force it high (blank) unless a +x cast is active
    assign OE  = oe_display | ~show;
    assign led = show;

endmodule
