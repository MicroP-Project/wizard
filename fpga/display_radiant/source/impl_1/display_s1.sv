// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 static row driver with exact FM6124EJ initialization sequence

// Stage 1 static row driver with verified FM6124EJ startup sequence
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
    // FM6124 Register 2 (12-clock LAT): clear configuration/blanking bits first
    localparam logic [15:0] FM6124_REG2 = 16'h0000;
    // FM6124 Register 1 (11-clock LAT): enable all 16 constant-current outputs
    localparam logic [15:0] FM6124_REG1 = 16'h7FFF;

    // Phases:
    // 0: Wait ~100 ms (300,000 clocks @ 3 MHz) for 5V power supply to stabilize
    // 1: Shift 64 bits of REG2 with 12-clock LAT pulse (cnt 52..63)
    // 2: Shift 64 bits of REG1 with 11-clock LAT pulse (cnt 53..63)
    // 3: Normal display loop (shift 64 pixels, then 1-clock LAT pulse)
    logic [1:0]  phase;
    logic [18:0] cnt;

    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst) begin
            phase <= 2'd0;
            cnt   <= 19'd0;
        end else begin
            case (phase)
                2'd0: begin
                    if (cnt == 19'd300_000) begin
                        phase <= 2'd1;
                        cnt   <= 19'd0;
                    end else begin
                        cnt <= cnt + 19'd1;
                    end
                end
                2'd1: begin
                    if (cnt == 19'd63) begin
                        phase <= 2'd2;
                        cnt   <= 19'd0;
                    end else begin
                        cnt <= cnt + 19'd1;
                    end
                end
                2'd2: begin
                    if (cnt == 19'd63) begin
                        phase <= 2'd3;
                        cnt   <= 19'd0;
                    end else begin
                        cnt <= cnt + 19'd1;
                    end
                end
                2'd3: begin
                    if (cnt == 19'd64)
                        cnt <= 19'd0;
                    else
                        cnt <= cnt + 19'd1;
                end
            endcase
        end
    end

    // Row address 00000 selects Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;

    // Gate CLK during the 1-cycle latch state in phase 3
    assign CLK = (phase == 2'd3 && cnt == 19'd64) ? 1'b0 : clk_shift;

    wire [3:0] bit_idx = 4'd15 - cnt[3:0];

    always_comb begin
        if (phase == 2'd0) begin
            // Idle during 100 ms power-up stabilization
            {R1, G1, B1, R2, G2, B2} = 6'b000000;
            LAT = 1'b0;
            OE  = 1'b1;
        end else if (phase == 2'd1) begin
            // Step 1: Write REG2 (12-clock LAT pulse)
            R1  = FM6124_REG2[bit_idx];
            G1  = FM6124_REG2[bit_idx];
            B1  = FM6124_REG2[bit_idx];
            R2  = FM6124_REG2[bit_idx];
            G2  = FM6124_REG2[bit_idx];
            B2  = FM6124_REG2[bit_idx];
            LAT = (cnt >= 19'd52);
            OE  = 1'b1;
        end else if (phase == 2'd2) begin
            // Step 2: Write REG1 (11-clock LAT pulse) to enable outputs
            R1  = FM6124_REG1[bit_idx];
            G1  = FM6124_REG1[bit_idx];
            B1  = FM6124_REG1[bit_idx];
            R2  = FM6124_REG1[bit_idx];
            G2  = FM6124_REG1[bit_idx];
            B2  = FM6124_REG1[bit_idx];
            LAT = (cnt >= 19'd53);
            OE  = 1'b1;
        end else begin
            // Step 3: Light up all 64 pixels in Row 0
            R1  = 1'b1;
            G1  = 1'b0;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = (cnt == 19'd64);
            OE  = 1'b0;
        end
    end

endmodule