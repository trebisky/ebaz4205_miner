`timescale 1ns / 1ps

// Verilog seems to let you use [0:31] or [31:0] interchangeably
// BUT they do mean something different.
// the left number is always the MSB.
// So, you have the choice of calling the MSB 0 or 31
// I am taking the road of calling the MSB 31

module fancy(
    input [31:0] xcmd,
    input clock,
    output led1,
    output led2
    );
    
    // for testing, I set values here
    // for launch, be sure this line is commented out
    // (or Vivado complains about multiple agents driving
    // the same input -- which makes sense.)
    // and change the name ycmd to xcmd
    // wire [31:0] ycmd = 32'h00000062;
    wire [3:0] choice1 = xcmd[3:0];
    wire [3:0] choice2 = xcmd[7:4];

    myled led01 ( r_slow, choice1, led1 );	// red
    myled led02 ( r_slow, choice2, led2 );	// green

    parameter p_CNT_2HZ = 25_000_000;

    reg [31:0] r_fast = 0;

    reg [2:0] r_slow = 0;

    always @ (posedge clock)
	begin
	  if (r_fast == p_CNT_2HZ-1)
	    begin
	      r_slow <= r_slow + 1;
	      r_fast    <= 0;
	    end
	  else
	    r_fast <= r_fast + 1;
	end

endmodule

// Writing a 0 turns the LED on.

module myled (
    input [2:0] clock,
    input [3:0] choice,
    output led
    );

    wire user = choice[3];
    wire phase = choice[2];
    wire [1:0] sel = choice[1:0];
    reg value;

    // I am reading curious things.
    // one is that case must always be inside always
    // next is that = (blocking), not <= should be
    // used inside an always @(*)
    always @(*)
	begin
	    case (sel)
		2'b00 : value = user;
		2'b01 : value = clock[0];	/* fast */
		2'b10 : value = clock[1];
		2'b11 : value = clock[2];	/* slow */
	    endcase
	end

    assign led = value ^ phase;

endmodule
