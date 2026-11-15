// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026
// Stage 1 static row driver: lights up Row 0 in solid red to verify HUB75 wiring & 5V power

/*
module display_s1 (
    input  logic clk_shift,
    output logic R1, G1, B1,
    output logic R2, G2, B2,
    output logic A, B, C, D, E,
    output logic CLK,
    output logic LAT,
    output logic OE
);
    // Drive only upper-half Red (R1) HIGH; keep all other color channels OFF
    assign R1 = 1'b1;
    assign G1 = 1'b0;
    assign B1 = 1'b0;

    assign R2 = 1'b0;
    assign G2 = 1'b0;
    assign B2 = 1'b0;

    // Row address 00000 selects Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;

    // Continuously toggle CLK to fill the 64-bit shift register with '1's,
    // keep LAT HIGH (transparent) and OE LOW (active-low output enabled)
    assign CLK = clk_shift;
    // assign LAT = 1'b1;
	assign LAT = ~clk_shift; // Change!!
    assign OE  = 1'b0;

endmodule */

// Stage 1 static row driver with FM6124 power-up initialization
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
    // Standard 16-bit enable word for FM6124 (all outputs enabled, standard gain)
    localparam logic [15:0] FM6124_REG1 = 16'h7FFF;

    logic [6:0] init_cnt;   // Counts 0..64 clock cycles for 4 cascaded 16-bit drivers
    logic       init_done;

    // Step through 64 clocks on negedge of clk_shift so data & LAT are stable on posedge
    always_ff @(negedge clk_shift or negedge nrst) begin
        if (!nrst) begin
            init_cnt  <= 7'd0;
            init_done <= 1'b0;
        end else if (!init_done) begin
            if (init_cnt == 7'd64) begin
                init_done <= 1'b1;
            end else begin
                init_cnt <= init_cnt + 7'd1;
            end
        end
    end

    // Select Row 0 (top half) and Row 32 (bottom half)
    assign {E, D, C, B, A} = 5'b00000;
    assign CLK = clk_shift;

    // During init (cycles 0..63): clock FM6124_REG1 into all color channels,
    // pulsing LAT high during the last 3 bits of every 16-bit word (bits 13, 14, 15).
    // After init: hold R1 = 1, pulse LAT on every cycle, and enable output (OE = 0).
    wire [3:0] bit_idx = 4'd15 - init_cnt[3:0];
    wire       cfg_bit = FM6124_REG1[bit_idx];

    always_comb begin
        if (!init_done) begin
            R1  = cfg_bit;
            G1  = cfg_bit;
            B1  = cfg_bit;
            R2  = cfg_bit;
            G2  = cfg_bit;
            B2  = cfg_bit;
            LAT = (init_cnt < 7'd64) && (init_cnt[3:0] >= 4'd13);
            OE  = 1'b1; // Keep display blanked during register config
        end else begin
            R1  = 1'b1;
            G1  = 1'b0;
            B1  = 1'b0;
            R2  = 1'b0;
            G2  = 1'b0;
            B2  = 1'b0;
            LAT = ~clk_shift;
            OE  = 1'b0; // Enable LED outputs
        end
    end

endmodule