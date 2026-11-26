// fpga_spi.sv
// SPI receiver for MCU -> FPGA game-state packets (MCU uses SPI3: SCK=PB3, MOSI=PB5, MISO=PB4, CS=PA8).
//
// Protocol (MCU side, see STM32L432KC_SPI3.h):
//   pull cs low
//   apply 8 sclks to shift in the packet, starting with packet[7] (MSB first)
//   wait for the last bit to finish, then raise cs
//   the packet is latched into `packet` on the rising edge of cs
//
// SPI mode 3 (cpol = 1, cpha = 1): sck idles high, the MCU changes MOSI on the falling
// edge and the FPGA samples on the rising edge. (Mode 0 also samples on the rising edge,
// so this module works for either.)
//
// sdo is "garbage": it just echoes back the bits received, delayed by N. The MCU ignores
// it, but it is handy for a loopback test on a logic analyzer.

module fpga_spi #(parameter N = 8)        // packet width in bits (1 byte)
              (input  logic         sck,
               input  logic         sdi,  // MOSI from MCU
               output logic         sdo,  // MISO to MCU (garbage)
               input  logic         cs,   // active-low chip select from MCU
               output logic [N-1:0] packet);

    logic [N-1:0] shiftreg;
    logic         sdodelayed;

    // shift in one bit on every rising edge of sck while cs is low, MSB first
    always_ff @(posedge sck)
        if (!cs) shiftreg <= {shiftreg[N-2:0], sdi};

    // cs rising = end of packet: copy the last N bits received to the output
    always_ff @(posedge cs)
        packet <= shiftreg;

    // sdo should change on the negative edge of sck
    always_ff @(negedge sck)
        sdodelayed <= shiftreg[N-1];

    assign sdo = sdodelayed;
endmodule
