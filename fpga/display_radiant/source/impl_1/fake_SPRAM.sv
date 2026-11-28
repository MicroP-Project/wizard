// Target 1 pixel lookup: hardcoded 15x15 centered right-pointing arrow

module fake_SPRAM (
    input  logic [4:0] row_addr, // 0..31
    input  logic [5:0] col_addr, // 0..63
    output logic [2:0] top_rgb,  // {R1, G1, B1}
    output logic [2:0] bot_rgb   // {R2, G2, B2}
);
    function automatic logic is_arrow_pixel(input logic [5:0] r, input logic [5:0] c);
        logic in_shaft, in_head;
        begin
            // Horizontal shaft: rows 30..34, cols 24..32
            in_shaft = (r >= 6'd30 && r <= 6'd34) && (c >= 6'd24 && c <= 6'd32);
            // Arrowhead pointing right: cols 32..38, tip at (row=32, col=38)
            in_head  = (c >= 6'd32 && c <= 6'd38) &&
                       (r >= (6'd25 + (c - 6'd32))) &&
                       (r <= (6'd39 - (c - 6'd32)));
            is_arrow_pixel = in_shaft || in_head;
        end
    endfunction

    wire top_on = is_arrow_pixel({1'b0, row_addr}, col_addr);
    wire bot_on = is_arrow_pixel({1'b1, row_addr}, col_addr);

    assign top_rgb = {top_on, 1'b0, 1'b0};
    assign bot_rgb = {bot_on, 1'b0, 1'b0};

endmodule