// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 static row driver with exact FM6124EJ initialization sequence

// Stage 1 static row driver for FM6124EJ (all 64 LEDs lit continuously)
module display_s1 (
    input  logic clk_shift,
    input  logic nrst,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
    // FM6124 Register 2 (12-clock LAT): clear fault/blanking flags
    localparam logic [15:0] FM6124_REG2 = 16'h0000;
    // FM6124 Register 1 (11-clock LAT): all 16 outputs enabled (MSB=1), reduced gain for static DC test
    localparam logic [15:0] FM6124_REG1 = 16'hFF0F;

    // Repeats every ~10 ms (30,000 clocks @ 3 MHz):
    // 0..63:    Write REG2 (12-clock LAT on 52..63)
    // 64..127:  Write REG1 (11-clock LAT on 117..127)
    // 128..191: Shift 64 '1's into R1 (1-clock LAT on 191)
    // 192..29999: Hold Row 0 lit with OE = 0
    logic [14:0] cnt;

    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst)
            cnt <= 15'd0;
        else if (cnt == 15'd29_999)
            cnt <= 15'd0;
        else
            cnt <= cnt + 15'd1;
    end

    // Row address 00000 selects Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;
    assign CLK = clk_shift;

    wire [3:0] bit_idx = 4'd15 - cnt[3:0];

    always_comb begin
        if (cnt < 15'd64) begin
            // Phase 1: Write REG2 across all 4 cascaded FM6124 drivers
            R1  = FM6124_REG2[bit_idx];
            G1  = FM6124_REG2[bit_idx];
            B1  = FM6124_REG2[bit_idx];
            R2  = FM6124_REG2[bit_idx];
            G2  = FM6124_REG2[bit_idx];
            B2  = FM6124_REG2[bit_idx];
            LAT = (cnt >= 15'd52);
            OE  = 1'b1;
        end else if (cnt < 15'd128) begin
            // Phase 2: Write REG1 across all 4 cascaded FM6124 drivers
            R1  = FM6124_REG1[bit_idx];
            G1  = FM6124_REG1[bit_idx];
            B1  = FM6124_REG1[bit_idx];
            R2  = FM6124_REG1[bit_idx];
            G2  = FM6124_REG1[bit_idx];
            B2  = FM6124_REG1[bit_idx];
            LAT = (cnt >= 15'd117);
            OE  = 1'b1;
        end else if (cnt < 15'd192) begin
            // Phase 3: Shift 64 '1's into R1 and latch on cycle 191
            R1  = 1'b1;
            G1  = 1'b0;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = (cnt == 15'd191);
            OE  = 1'b1;
        end else begin
            // Phase 4: Hold Row 0 continuously lit
            R1  = 1'b1;
            G1  = 1'b0;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = 1'b0;
            OE  = 1'b0;
        end
    end

endmodule