// The display board's buttons hang off a parallel-in serial-out shift
// register. This testbench plays the part of that chip: it latches a pattern
// when the controller pulls load low, then shifts a bit out on every rising
// shift clock. Whatever pattern goes in should come back out of `buttons`.
#include "Vshiftregctl.h"
#include "tb.h"
#include <memory>

struct ShiftRegister {
	uint16_t shadow = 0;
	uint16_t parallel = 0;
	int last_loadn = 1;
	int last_clk = 0;

	// Called once per simulated clock, after eval().
	void update(int loadn, int clk) {
		if (last_loadn == 1 && loadn == 0)
			shadow = parallel;         // parallel load while load is low
		if (last_clk == 0 && clk == 1)
			shadow >>= 1;              // shift out on the rising edge
		last_loadn = loadn;
		last_clk = clk;
	}
	int serial_out() const { return shadow & 1; }
};

// Runs one full scan and returns what the controller reported.
static uint16_t scan(Vshiftregctl *dut, ShiftRegister &sr, uint16_t pattern) {
	sr.parallel = pattern;
	// A scan is 19 states at 512 clocks each; two scans guarantee we observe a
	// complete one that began after the pattern was set.
	for (int i = 0; i < 19 * 512 * 2; i++) {
		dut->clock_50m = 0;
		dut->eval();
		dut->clock_50m = 1;
		dut->eval();
		sr.update(dut->shiftreg_loadn, dut->shiftreg_clk);
		dut->shiftreg_out = sr.serial_out();
	}
	return dut->buttons;
}

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	auto dut = std::make_unique<Vshiftregctl>();
	ShiftRegister sr;

	dut->reset = 1;
	dut->shiftreg_out = 0;
	for (int i = 0; i < 4; i++) {
		dut->clock_50m = 0; dut->eval();
		dut->clock_50m = 1; dut->eval();
	}
	dut->reset = 0;

	const uint16_t patterns[] = {0x0000, 0xFFFF, 0x0001, 0x8000, 0xA5A5, 0x1234};
	for (uint16_t p : patterns)
		tb::check_eq(scan(dut.get(), sr, p), p,
		             "pattern 0x" + std::to_string(p) + " survives the round trip");

	dut->final();
	return tb::report("shiftregctl");
}
