// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026

// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 row driver reading a 64-pixel color pattern from iCE40UP5K SPRAM

// Stage 1/2 driver reading a 64-pixel pattern from iCE40UP5K SP256K SPRAM
// Preserves exact 4-phase FM6124 timing & 1/64 OE PWM duty cycle

module display_s1 (
    input  logic clk_shift,
    input  logic nrst,
    input  logic cast_active,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
    localparam logic [15:0] FM6124_REG2 = 16'h0000; // Clear fault/blanking flags
    localparam logic [15:0] FM6124_REG1 = 16'h8000; // Lowest current gain (prevents thermal shutoff)

    logic [14:0] cnt;
    logic       spram_we;
    logic [5:0] spram_addr;
    logic [2:0] spram_rgb;

    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst)
            cnt <= 15'd0;
        else if (cnt == 15'd29_999)
            cnt <= 15'd0;
        else
            cnt <= cnt + 15'd1;
    end

    // Write 0..63 into SPRAM during Phase 1 (cnt 0..63); read during Phase 3 (cnt 128..191)
    assign spram_we   = (cnt < 15'd64);
    assign spram_addr = cnt[5:0];

    spram_row_test u_spram_row (
        .clk     (clk_shift), // Samples on posedge clk_shift (180 deg offset from cnt)
        .init_we (spram_we),
        .addr    (spram_addr),
        .rgb_out (spram_rgb)
    );

    // Row 0 at idle, Row 16 when valid movement is active
    assign {E, D, C, B, A} = cast_active ? 5'b10000 : 5'b00000;
    assign CLK = clk_shift;

    wire [3:0] bit_idx = 4'd15 - cnt[3:0];

    always_comb begin
        if (cnt < 15'd64) begin
            // Phase 1: Write REG2 (also writes 64 pixels into SPRAM)[cite: 5]
            R1  = FM6124_REG2[bit_idx];
            G1  = FM6124_REG2[bit_idx];
            B1  = FM6124_REG2[bit_idx];
            R2  = FM6124_REG2[bit_idx];
            G2  = FM6124_REG2[bit_idx];
            B2  = FM6124_REG2[bit_idx];
            LAT = (cnt >= 15'd52);
            OE  = 1'b1;
        end else if (cnt < 15'd128) begin
            // Phase 2: Write REG1[cite: 5]
            R1  = FM6124_REG1[bit_idx];
            G1  = FM6124_REG1[bit_idx];
            B1  = FM6124_REG1[bit_idx];
            R2  = FM6124_REG1[bit_idx];
            G2  = FM6124_REG1[bit_idx];
            B2  = FM6124_REG1[bit_idx];
            LAT = (cnt >= 15'd117);
            OE  = 1'b1;
        end else if (cnt < 15'd192) begin
            // Phase 3: Shift 64 pixels from SPRAM while OE is blanked (1'b1), then latch on 191[cite: 5]
            {R1, G1, B1} = cast_active ? 3'b001 : spram_rgb;
            {R2, G2, B2} = 3'b000;
            LAT = (cnt == 15'd191);
            OE  = 1'b1;
        end else begin
            // Phase 4: Hold latched row lit at 1/64 duty cycle (no shifting blur, no overheating)[cite: 5]
            {R1, G1, B1} = 3'b000;
            {R2, G2, B2} = 3'b000;
            LAT = 1'b0;
            OE  = (cnt[5:0] != 6'b000000);
        end
    end

endmodule