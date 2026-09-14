// Runs the compiled firmware on the simulated SoC.
//
// Nothing here knows what the program does. It turns the dials, presses a
// button, and reads the framebuffer back out -- exactly the inputs and outputs
// the real board has. The picture that comes out is drawn by the RISC-V code in
// software/src/ executing on the Clarvi core.
#include "Vsoc_sim.h"
#include "png.h"
#include "tb.h"
#include <memory>
#include <string>
#include <vector>
#ifdef TRACE
#include "verilated_fst_c.h"
#endif

static const int WIDTH = 480;
static const int HEIGHT = 272;

// Buttons, from software/src/avalon_addr.h.
static const uint16_t BUTTON_DIALR_CLICK = 0x4;

// How long each quadrature phase is held. This has to outlast the debouncer
// and give the polling loop time to notice every count; the firmware moves the
// cursor one pixel per observed change, however many counts have gone by.
static const int PHASE_CLOCKS = 250;

struct Soc {
	std::unique_ptr<Vsoc_sim> dut = std::make_unique<Vsoc_sim>();
	uint64_t cycles = 0;
#ifdef TRACE
	VerilatedFstC *fst = nullptr;
#endif

	void tick(int n = 1) {
		for (int i = 0; i < n; i++) {
			dut->clock = 0;
			dut->eval();
#ifdef TRACE
			if (fst) fst->dump(cycles * 10);
#endif
			dut->clock = 1;
			dut->eval();
#ifdef TRACE
			if (fst) fst->dump(cycles * 10 + 5);
#endif
			cycles++;
		}
	}

	// One detent of a quadrature encoder. `forward` picks the direction.
	void turn(CData &dial, bool forward, int detents = 1) {
		static const int fwd[4] = {0b01, 0b11, 0b10, 0b00};
		static const int rev[4] = {0b10, 0b11, 0b01, 0b00};
		for (int d = 0; d < detents; d++)
			for (int i = 0; i < 4; i++) {
				dial = forward ? fwd[i] : rev[i];
				tick(PHASE_CLOCKS);
			}
	}

	uint16_t pixel(int x, int y) {
		size_t half_word = (size_t) y * WIDTH + x;
		dut->probe_addr = (uint32_t) (half_word >> 1);
		dut->eval();
		uint32_t w = dut->probe_data;
		return (half_word & 1) ? (uint16_t) (w >> 16) : (uint16_t) w;
	}

	size_t count_lit() {
		size_t n = 0;
		for (int y = 0; y < HEIGHT; y++)
			for (int x = 0; x < WIDTH; x++)
				if (pixel(x, y) != 0) n++;
		return n;
	}
};

static std::string plusarg(const char *name, const char *fallback) {
	const char *v = Verilated::commandArgsPlusMatch(name);
	if (v && *v) {
		std::string s(v);
		size_t eq = s.find('=');
		if (eq != std::string::npos) return s.substr(eq + 1);
	}
	return fallback;
}

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	Soc soc;
	auto *dut = soc.dut.get();

#ifdef TRACE
	Verilated::traceEverOn(true);
	std::string fstpath = plusarg("fst", "");
	if (!fstpath.empty()) {
		soc.fst = new VerilatedFstC;
		dut->trace(soc.fst, 4);
		soc.fst->open(fstpath.c_str());
	}
#endif

	dut->dial_l = 0;
	dut->dial_r = 0;
	dut->buttons = 0;
	dut->probe_addr = 0;
	dut->reset = 1;
	soc.tick(16);
	dut->reset = 0;

	// Let the program reach its polling loop and place the starting pixel.
	uint32_t pc_after_reset = dut->debug_pc;
	soc.tick(20000);
	tb::check(dut->debug_pc != pc_after_reset, "the core is fetching instructions");
	tb::check_eq(soc.count_lit(), (size_t) 1, "firmware lights exactly the starting pixel");

	// Trace a rectangle. The left dial moves the cursor in x, the right in y.
	const int W = 80, H = 50;
	soc.turn(dut->dial_l, true, W);
	soc.turn(dut->dial_r, true, H);
	soc.turn(dut->dial_l, false, W);
	soc.turn(dut->dial_r, false, H);

	size_t lit = soc.count_lit();
	std::printf("  drew %zu pixels in %llu cycles\n", lit,
	            (unsigned long long) soc.cycles);

	// A closed rectangle of these dimensions has 2*(W+H) edge pixels. Allow a
	// little slack: the first poll nudges the cursor before the loop settles.
	tb::check(lit >= (size_t) (2 * (W + H) - 8) && lit <= (size_t) (2 * (W + H) + 8),
	          "pixel count matches a closed rectangle outline");

	// Save the drawing before clearing it.
	std::vector<uint8_t> rgb((size_t) WIDTH * HEIGHT * 3, 0);
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++) {
			uint16_t p = soc.pixel(x, y);
			uint8_t *o = &rgb[((size_t) y * WIDTH + x) * 3];
			o[0] = (uint8_t) (((p >> 11) & 0x1F) * 255 / 31); // 5 bits red
			o[1] = (uint8_t) (((p >> 5) & 0x3F) * 255 / 63);  // 6 bits green
			o[2] = (uint8_t) (((p >> 0) & 0x1F) * 255 / 31);  // 5 bits blue
		}
	std::string out = plusarg("png", "sketch.png");
	tb::check(png::write_rgb(out, WIDTH, HEIGHT, rgb), "framebuffer written as PNG");
	std::printf("  wrote %s\n", out.c_str());

	// Clicking either dial clears the screen. This also exercises the button
	// path: the shift register scanner, not just the PIO.
	dut->buttons = BUTTON_DIALR_CLICK;
	soc.tick(6000000);
	dut->buttons = 0;
	tb::check_eq(soc.count_lit(), (size_t) 0, "pressing a dial clears the screen");

#ifdef TRACE
	if (soc.fst) { soc.fst->close(); delete soc.fst; }
#endif
	dut->final();
	return tb::report("soc");
}
