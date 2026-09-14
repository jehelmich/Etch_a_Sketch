// Every one of the 256 input bytes must light the right segments, active low.
#include "VEightBitsToSevenSeg.h"
#include "tb.h"
#include <memory>

// Segment patterns for 0-F, active high, in the module's own bit order.
static const unsigned char LED[16] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
	0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71,
};

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	auto dut = std::make_unique<VEightBitsToSevenSeg>();

	for (int v = 0; v < 256; v++) {
		dut->hexval = v;
		dut->eval();
		int want0 = ~LED[v & 0xF] & 0x7F;        // the DE1-SoC displays are
		int want1 = ~LED[(v >> 4) & 0xF] & 0x7F; // wired active low
		tb::check_eq(dut->digit0, want0, "low nibble of 0x" + std::to_string(v));
		tb::check_eq(dut->digit1, want1, "high nibble of 0x" + std::to_string(v));
	}

	dut->final();
	return tb::report("seven_seg");
}
