// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026

// Stage 1/2 driver for FM6124EJ:
// - Runs at 1/64 duty cycle (cnt[5:0] == 0) + minimum current gain (16'h8000) so it NEVER overheats
// - Switches cleanly from Row 0 (one color) at idle to Row 16 (different color) on cast_active
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
    // FM6124 Register 2 (12-clock LAT): clear fault/blanking flags
    localparam logic [15:0] FM6124_REG2 = 16'h0000;
    // FM6124 Register 1 (11-clock LAT): all 16 channels enabled (MSB=1), minimum current gain (0x000)
    localparam logic [15:0] FM6124_REG1 = 16'h8000;

    logic [14:0] cnt;

    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst)
            cnt <= 15'd0;
        else if (cnt == 15'd29_999)
            cnt <= 15'd0;
        else
            cnt <= cnt + 15'd1;
    end

    // Row 0 at idle, Row 16 when valid movement is active
    assign {E, D, C, B, A} = cast_active ? 5'b10000 : 5'b00000;
    assign CLK = clk_shift;

    wire [3:0] bit_idx = 4'd15 - cnt[3:0];

    always_comb begin
        if (cnt < 15'd64) begin
            // Phase 1: Write REG2
            R1  = FM6124_REG2[bit_idx];
            G1  = FM6124_REG2[bit_idx];
            B1  = FM6124_REG2[bit_idx];
            R2  = FM6124_REG2[bit_idx];
            G2  = FM6124_REG2[bit_idx];
            B2  = FM6124_REG2[bit_idx];
            LAT = (cnt >= 15'd52);
            OE  = 1'b1;
        end else if (cnt < 15'd128) begin
            // Phase 2: Write REG1
            R1  = FM6124_REG1[bit_idx];
            G1  = FM6124_REG1[bit_idx];
            B1  = FM6124_REG1[bit_idx];
            R2  = FM6124_REG1[bit_idx];
            G2  = FM6124_REG1[bit_idx];
            B2  = FM6124_REG1[bit_idx];
            LAT = (cnt >= 15'd117);
            OE  = 1'b1;
        end else if (cnt < 15'd192) begin
            // Phase 3: Shift 64 pixels (R1 at idle, G1 on valid movement)
            R1  = ~cast_active;
            G1  = cast_active;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = (cnt == 15'd191);
            OE  = 1'b1;
        end else begin
            // Phase 4: Hold row lit at 1/64 duty cycle (matches normal 1/32 scan thermal load)
            R1  = ~cast_active;
            G1  = cast_active;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = 1'b0;
            OE  = (cnt[5:0] != 6'b000000);
        end
    end

endmodule

