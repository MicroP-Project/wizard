// fpga_spi_test.sv
// Top-level test for the MCU -> FPGA SPI link.
// Lights `led` for 1 second every time a packet with the "right" flag (bit 0) is received
// (the MCU sends this when the user casts direction 1 = swing in +x).
// Pair with mcu/src/spi_test.c.
//
// Pins: sck <- PB3, sdi <- PB5 (MOSI), sdo -> PB4 (MISO, garbage), cs <- PA8, led -> any LED.

module fpga_spi_test(input  logic sck, // P21
                     input  logic sdi, // P19
                     output logic sdo, // P11
                     input  logic cs,  // P6
                     output logic led);// P38

    localparam int          ONE_SECOND  = 48_000_000;  // HSOSC = 48 MHz

    logic        clk;
    logic [7:0]  packet;
    logic [2:0]  cs_sync;
    logic        cs_rise;
    logic [25:0] counter;                              // 2^26 > 48,000,000

    // 48 MHz internal oscillator
    HSOSC #(.CLKHF_DIV(2'b00))
          hf_osc (.CLKHFPU(1'b1), .CLKHFEN(1'b1), .CLKHF(clk));

    // SPI receiver: packet updates on the rising edge of cs
    // packet = {2'b00, next_spell[2:0], timeout, wrong, right}
    fpga_spi #(.N(8)) spi(.sck, .sdi, .sdo, .cs, .packet);

    // bring cs into the clk domain (2 flops for metastability + 1 for edge detect)
    always_ff @(posedge clk)
        cs_sync <= {cs_sync[1:0], cs};

    // end of a packet; packet was latched on the real cs edge a few clks ago, so it is stable now
    assign cs_rise = cs_sync[1] & ~cs_sync[2];

    // start (or restart) a 1 s countdown whenever a packet with right = 1 arrives
    always_ff @(posedge clk)
        if (cs_rise && packet[0]) counter <= ONE_SECOND - 1;
        else if (counter != 0)                counter <= counter - 1;

    assign led = (counter != 0);   // active high; use ~(counter != 0) if your LED is active low
endmodule
