// Bare-minimum SPRAM wrapper & 64-pixel split-row pattern (32 Red / 32 Green)

module spram_row_test (
    input  logic       clk,        // Connect to clk_shift
    input  logic       init_we,    // High during Phase 0/1 to populate RAM
    input  logic [5:0] addr,       // 0..63 pixel column index
    output logic [2:0] rgb_out     // {R, G, B} color read from SPRAM
);
    logic [15:0] wdata;
    logic [15:0] rdata;

    // Left half of row (addr 0..31)  -> Channel 1 (3'b100)
    // Right half of row (addr 32..63) -> Channel 2 (3'b010)
    always_comb begin
        if (addr[5] == 1'b0)
            wdata = 16'h0004;
        else
            wdata = 16'h0002;
    end

    SP256K u_spram (
        .AD       ({8'd0, addr}),
        .DI       (wdata),
        .MASKWE   (4'b1111),
        .WE       (init_we),
        .CS       (1'b1),
        .CK       (clk),
        .STDBY    (1'b0),
        .SLEEP    (1'b0),
        .PWROFF_N (1'b1),
        .DO       (rdata)
    );

    assign rgb_out = rdata[2:0];

endmodule