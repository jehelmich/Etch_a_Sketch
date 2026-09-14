// The debouncer must ignore a bouncing contact and only pass a new level once
// it has held steady for the full window.
#include "Vdebounce.h"
#include "tb.h"
#include <memory>

static const int WINDOW = 1 << 15; // must match COUNTER_WIDTH in debounce.sv

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	auto dut = std::make_unique<Vdebounce>();

	dut->rst = 1;
	dut->bouncy_in = 0;
	tb::tick(dut.get(), 2);
	dut->rst = 0;
	tb::tick(dut.get(), WINDOW + 8);
	tb::check_eq(dut->clean_out, 0, "output low after reset settles");

	// A bounce train: the input flaps for a while before settling high. The
	// output must not move while it is flapping.
	bool moved_during_bounce = false;
	for (int i = 0; i < 40; i++) {
		dut->bouncy_in = i & 1;
		tb::tick(dut.get(), 50); // each burst is far shorter than the window
		if (dut->clean_out != 0)
			moved_during_bounce = true;
	}
	tb::check(!moved_during_bounce, "output holds while the contact bounces");

	// Now hold it high. Two flops of synchroniser plus the settle window.
	dut->bouncy_in = 1;
	tb::tick(dut.get(), WINDOW / 2);
	tb::check_eq(dut->clean_out, 0, "output still low half way through the window");
	tb::tick(dut.get(), WINDOW / 2 + 8);
	tb::check_eq(dut->clean_out, 1, "output follows once the input is stable");

	// And back down again.
	dut->bouncy_in = 0;
	tb::tick(dut.get(), WINDOW + 8);
	tb::check_eq(dut->clean_out, 0, "output returns low");

	// A glitch shorter than the window must be swallowed entirely.
	dut->bouncy_in = 1;
	tb::tick(dut.get(), WINDOW / 4);
	dut->bouncy_in = 0;
	tb::tick(dut.get(), WINDOW + 8);
	tb::check_eq(dut->clean_out, 0, "a short glitch never reaches the output");

	dut->final();
	return tb::report("debounce");
}
