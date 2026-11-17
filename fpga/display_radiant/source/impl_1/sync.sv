// Jessica Li & Ellen Yu  |  jesli@g.hmc.edu & ellyu@g.hmc.edu  |  11/15/2026

// Submodule for flop synchronizer, parameterized for width of input and output signals.
module sync #(
    parameter int WIDTH = 3
) (
    input  logic             clk,
    input  logic [WIDTH-1:0] d,
    output logic [WIDTH-1:0] q
);
    logic [WIDTH-1:0] n1;

    always_ff @(posedge clk) begin
        n1 <= d;
        q  <= n1;
    end
endmodule
