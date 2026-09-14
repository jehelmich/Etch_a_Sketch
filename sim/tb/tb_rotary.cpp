// A rotary encoder puts out two phases in quadrature. Turning one way gives
// the Gray sequence 00 -> 01 -> 11 -> 10 -> 00, turning the other way runs it
// backwards. The decoder must produce one count per full cycle, and count in
// opposite directions for the two.
//
// Built with a short debounce window (-GDEBOUNCE_COUNTER_WIDTH) so the test
// does not have to spend 32768 clocks on every phase change.
#include "Vrotary.h"
#include "tb.h"
#include <memory>

static const int SETTLE = 48; // comfortably longer than the shortened window

static void phase(Vrotary *dut, int value) {
	dut->rotary_in = value;
	tb::tick(dut, SETTLE);
}

// One detent in the 00 -> 01 -> 11 -> 10 -> 00 direction.
static void step_forward(Vrotary *dut) {
	phase(dut, 0b01);
	phase(dut, 0b11);
	phase(dut, 0b10);
	phase(dut, 0b00);
}

// One detent the other way round.
static void step_back(Vrotary *dut) {
	phase(dut, 0b10);
	phase(dut, 0b11);
	phase(dut, 0b01);
	phase(dut, 0b00);
}

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	auto dut = std::make_unique<Vrotary>();

	dut->rotary_in = 0;
	dut->rst = 1;
	tb::tick(dut.get(), 2);
	dut->rst = 0;
	phase(dut.get(), 0b00);

	int start = dut->rotary_pos;

	// Ten detents forward.
	for (int i = 0; i < 10; i++)
		step_forward(dut.get());
	int forward = (uint8_t)(dut->rotary_pos - start);
	tb::check_eq(forward, 10, "ten detents forward give ten counts");

	// Standing still must not produce counts.
	int held = dut->rotary_pos;
	tb::tick(dut.get(), SETTLE * 8);
	tb::check_eq(dut->rotary_pos, held, "counter is stable while the dial is still");

	// Ten detents back returns to where we started.
	for (int i = 0; i < 10; i++)
		step_back(dut.get());
	tb::check_eq(dut->rotary_pos, start, "ten detents back cancel ten forward");

	// The counter is only 8 bits wide and is meant to wrap; the software reads
	// it as a delta, never as an absolute position.
	for (int i = 0; i < 300; i++)
		step_forward(dut.get());
	tb::check_eq(dut->rotary_pos, (uint8_t)(start + 300), "counter wraps at 8 bits");

	dut->final();
	return tb::report("rotary");
}
