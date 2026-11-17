// wizard_top_tb.sv
// Testbench for wizard_top: checks that OE stays high (panel dark) until the MCU
// sends the +x packet (16'h0001), that the panel then turns on, that a non-+x
// packet does not turn it on, and that it turns off again after SHOW_CYCLES.
//
// Needs a model of the Lattice HSOSC primitive. In Questa/ModelSim, compile with the
// iCE40UP library, e.g.:
//   vlog -sv -L iCE40UP ../src/wizard_top.sv ../src/fpga_spi.sv \
//        ../display_radiant/source/impl_1/{clk_div,display_s1,sync}.sv wizard_top_tb.sv
//   vsim -L iCE40UP wizard_top_tb
// SHOW_CYCLES is shrunk so the sim finishes quickly.

`timescale 1ns/1ps

module wizard_top_tb;
    localparam int SHOW_CYCLES = 20_000;           // ~417 us at 48 MHz
    localparam real SCK_HALF   = 200.0;            // 2.5 MHz SPI clock

    logic nrst_in, sck, sdi, cs;
    logic sdo, R1, G1, B1, R2, G2, B2, A, B, C, D, E, CLK, LAT, OE, led;
    int   errors = 0;

    wizard_top #(.SHOW_CYCLES(SHOW_CYCLES)) dut (.*);

    // mode 3 transaction, MSB first: sck idles high, change sdi on falling, FPGA samples on rising
    task automatic send_packet(input logic [15:0] p);
        cs = 0; #(SCK_HALF);
        for (int i = 15; i >= 0; i--) begin
            sck = 0; sdi = p[i]; #(SCK_HALF);
            sck = 1;             #(SCK_HALF);
        end
        #(SCK_HALF); cs = 1; #(SCK_HALF);
    endtask

    // display_s1's OE period is ~10 ms, too long to wait for each check, so check the
    // internal enable here; the always block below checks OE itself the whole time.
    task automatic expect_show(input logic want, input string what);
        #1000;
        if (dut.show !== want) begin
            $display("FAIL: %s (show=%b, expected %b) at %0t", what, dut.show, want, $time);
            errors++;
        end else
            $display("ok:   %s", what);
    endtask

    // whenever the panel is supposed to be off, OE must be high
    always @(OE) if (nrst_in && !dut.show && OE !== 1'b1) begin
        $display("FAIL: OE low while panel disabled at %0t", $time);
        errors++;
    end

    initial begin
        sck = 1; sdi = 0; cs = 1; nrst_in = 0;
        #2000 nrst_in = 1;

        expect_show(0, "dark after reset");

        send_packet(16'h0002);                       // -x cast: should be ignored
        expect_show(0, "-x packet ignored");

        send_packet(16'h0001);                       // +x cast
        expect_show(1, "+x packet turns panel on");
        if (led !== 1'b1) begin $display("FAIL: led not on"); errors++; end

        #((SHOW_CYCLES / 2) * 20.833);
        send_packet(16'h0001);                       // re-cast restarts the timer
        #((SHOW_CYCLES * 3 / 4) * 20.833);
        expect_show(1, "re-cast extended on-time");

        #((SHOW_CYCLES / 2) * 20.833);
        expect_show(0, "panel off after timeout");

        if (errors == 0) $display("ALL TESTS PASSED");
        else             $display("%0d ERRORS", errors);
        $stop;
    end
endmodule
