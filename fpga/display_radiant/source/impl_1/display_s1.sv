// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 static row driver with exact FM6124EJ initialization sequence

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
    // FM6124 Register 1 (11-clock LAT): enable all outputs, max current gain
    localparam logic [15:0] FM6124_REG1 = 16'h7FFF;
    // FM6124 Register 2 (12-clock LAT): default configuration
    localparam logic [15:0] FM6124_REG2 = 16'h0040;

    // Sequence phases:
    // 0: 64 clocks for REG1 (LAT high for last 11 clocks of each 16-bit chip: 53..63)
    // 1: 64 clocks for REG2 (LAT high for last 12 clocks of each 16-bit chip: 52..63)
    // 2: Normal running mode (shift 64 '1's into R1, then pulse LAT for 1 cycle)
    logic [1:0] phase;
    logic [6:0] cnt;

    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst) begin
            phase <= 2'd0;
            cnt   <= 7'd0;
        end else begin
            if (phase == 2'd0) begin
                if (cnt == 7'd63) begin
                    phase <= 2'd1;
                    cnt   <= 7'd0;
                end else begin
                    cnt <= cnt + 7'd1;
                end
            end else if (phase == 2'd1) begin
                if (cnt == 7'd63) begin
                    phase <= 2'd2;
                    cnt   <= 7'd0;
                end else begin
                    cnt <= cnt + 7'd1;
                end
            end else begin
                // Continuous 65-cycle frame: 64 shifts (0..63) + 1 latch cycle (64)
                if (cnt == 7'd64)
                    cnt <= 7'd0;
                else
                    cnt <= cnt + 7'd1;
            end
        end
    end

    // Row address 00000 selects Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;

    // Gate CLK during the 1-cycle latch state (cnt == 64) in phase 2
    assign CLK = (phase == 2'd2 && cnt == 7'd64) ? 1'b0 : clk_shift;

    wire [3:0] bit_idx = 4'd15 - cnt[3:0];

    always_comb begin
        if (phase == 2'd0) begin
            // Write REG1 across all 4 cascaded 16-bit FM6124 chips
            R1  = FM6124_REG1[bit_idx];
            G1  = FM6124_REG1[bit_idx];
            B1  = FM6124_REG1[bit_idx];
            R2  = FM6124_REG1[bit_idx];
            G2  = FM6124_REG1[bit_idx];
            B2  = FM6124_REG1[bit_idx];
            // FM6124 REG1 latch width = 11 clock cycles (cnt 53..63)
            LAT = (cnt >= 7'd53);
            OE  = 1'b1;
        end else if (phase == 2'd1) begin
            // Write REG2 across all 4 cascaded 16-bit FM6124 chips
            R1  = FM6124_REG2[bit_idx];
            G1  = FM6124_REG2[bit_idx];
            B1  = FM6124_REG2[bit_idx];
            R2  = FM6124_REG2[bit_idx];
            G2  = FM6124_REG2[bit_idx];
            B2  = FM6124_REG2[bit_idx];
            // FM6124 REG2 latch width = 12 clock cycles (cnt 52..63)
            LAT = (cnt >= 7'd52);
            OE  = 1'b1;
        end else begin
            // Normal display: shift '1's into R1 for 64 clocks, then pulse LAT on cycle 64
            R1  = 1'b1;
            G1  = 1'b0;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = (cnt == 7'd64);
            OE  = 1'b0;
        end
    end

endmodule