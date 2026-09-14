// An interactive Etch A Sketch, with no FPGA underneath it.
//
// The window shows the simulated framebuffer. Turning a "dial" with the arrow
// keys synthesises quadrature on the same pins the encoders drive on the board;
// the simulated Clarvi core polls them and plots pixels; the framebuffer is
// blitted to the screen. Everything between the keypress and the pixel is the
// design's own RTL.
//
//   arrow keys / WASD   turn the dials
//   space               clear the screen
//   Q or escape         quit (native build only)
//
// Builds natively against SDL2, and for the browser through Emscripten, where
// the frame loop is driven by requestAnimationFrame instead of a while loop.
#include "Vsoc_sim.h"
#include "Vsoc_sim___024root.h"
#include <SDL.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <pthread.h>
#include <sched.h>

// Verilator's host-introspection helper takes its Linux path under Emscripten,
// because Emscripten's headers define CPU_ZERO, and then calls an affinity
// function the runtime does not provide unless pthreads are enabled. Enabling
// pthreads would mean SharedArrayBuffer and COOP/COEP response headers, which
// a static host will not necessarily send. A browser tab has one thread, so
// answer the question directly.
extern "C" {
int pthread_getaffinity_np(pthread_t, size_t, struct cpu_set_t *set) {
	CPU_ZERO(set);
	CPU_SET(0, set);
	return 0;
}
int pthread_setaffinity_np(pthread_t, size_t, const struct cpu_set_t *) {
	return 0; // nothing to pin a thread to
}
int sched_getcpu(void) {
	return 0; // there is only one
}
}
#else
#include "png.h"
#endif

static const int WIDTH = 480;
static const int HEIGHT = 272;
static const int SCALE = 2;

// Buttons, from software/src/avalon_addr.h.
static const uint16_t BUTTON_DIALL_CLICK = 0x2;

// How fast a held key turns a dial, in detents per second. One detent moves the
// cursor one pixel.
static const double DETENTS_PER_SECOND = 140.0;

// Wall-clock time to spend simulating per displayed frame. The rest of the
// frame goes on presenting it. Simulated time advances at whatever rate the
// host manages, which is a fraction of the real board's 50 MHz.
static const double SIM_BUDGET_SECONDS = 0.012;

// The Gray sequence an encoder walks through, one step per quarter detent.
static const int GRAY[4] = {0b00, 0b01, 0b11, 0b10};

struct Dial {
	int phase = 0;
	double pending = 0.0;
	int direction = 0;

	int quarter_steps(double dt) {
		if (direction != 0)
			pending += DETENTS_PER_SECOND * dt;
		int detents = (int) pending;
		pending -= detents;
		return detents * 4;
	}
	int step(int dir) {
		phase = (phase + (dir > 0 ? 1 : 3)) & 3;
		return GRAY[phase];
	}
};

struct Sim {
	std::unique_ptr<Vsoc_sim> dut = std::make_unique<Vsoc_sim>();
	uint64_t cycles = 0;

	void tick(uint64_t n) {
		for (uint64_t i = 0; i < n; i++) {
			dut->clock = 0;
			dut->eval();
			dut->clock = 1;
			dut->eval();
		}
		cycles += n;
	}
	// The framebuffer is 16-bit RGB565, two pixels per 32-bit word, which is
	// exactly what SDL wants -- no conversion, just a pointer.
	const uint16_t *pixels() const {
		return reinterpret_cast<const uint16_t *>(&dut->rootp->soc_sim__DOT__fb[0]);
	}
	void reset() {
		dut->dial_l = GRAY[0];
		dut->dial_r = GRAY[0];
		dut->buttons = 0;
		dut->probe_addr = 0;
		dut->reset = 1;
		tick(16);
		dut->reset = 0;
		tick(20000); // let the firmware reach its polling loop
	}
};

static size_t count_lit(const Sim &sim) {
	const uint16_t *px = sim.pixels();
	size_t n = 0;
	for (size_t i = 0; i < (size_t) WIDTH * HEIGHT; i++)
		if (px[i]) n++;
	return n;
}

// Simulates `cycles`, applying the dials' quarter-steps spread evenly across
// them so each new level has time to get through the debouncers and be seen by
// the polling loop.
static void run_frame(Sim &sim, Dial &left, Dial &right, int lsteps, int rsteps,
                      uint64_t cycles) {
	int slices = std::max(lsteps, rsteps) + 1;
	uint64_t per_slice = cycles / (uint64_t) slices;
	if (per_slice < 2048)
		per_slice = 2048; // never shorter than the debouncers need

	for (int s = 0; s < slices; s++) {
		if (s > 0) {
			if (s <= lsteps) sim.dut->dial_l = left.step(left.direction);
			if (s <= rsteps) sim.dut->dial_r = right.step(right.direction);
		}
		sim.tick(per_slice);
	}
}

#ifndef __EMSCRIPTEN__
static void save_png(const Sim &sim, const std::string &path) {
	std::vector<uint8_t> rgb((size_t) WIDTH * HEIGHT * 3);
	const uint16_t *px = sim.pixels();
	for (size_t i = 0; i < (size_t) WIDTH * HEIGHT; i++) {
		uint16_t p = px[i];
		rgb[i * 3 + 0] = (uint8_t) (((p >> 11) & 0x1F) * 255 / 31);
		rgb[i * 3 + 1] = (uint8_t) (((p >> 5) & 0x3F) * 255 / 63);
		rgb[i * 3 + 2] = (uint8_t) (((p >> 0) & 0x1F) * 255 / 31);
	}
	png::write_rgb(path, WIDTH, HEIGHT, rgb);
	std::printf("  wrote %s\n", path.c_str());
}
#endif

// Headless runs are fixed-step so they are reproducible: same frame count, same
// cycles per frame, same picture every time.
static const double HEADLESS_DT = 1.0 / 60.0;
static const uint64_t HEADLESS_CYCLES_PER_FRAME = 150000;

struct App {
	Sim sim;
	Dial left, right;
	SDL_Window *win = nullptr;
	SDL_Renderer *ren = nullptr;
	SDL_Texture *tex = nullptr;

	bool headless = false;   // dummy video driver, exits with a verdict
	bool scripted = false;   // canned input instead of the keyboard
	std::string shot;
	bool running = true;
	int frame = 0;
	size_t drawn = 0;
	int failures = 0;

	std::chrono::steady_clock::time_point last, last_report;
	uint64_t last_cycles = 0;
	double measured_hz = 10e6; // starting guess, corrected once a second

	void step() {
		auto now = std::chrono::steady_clock::now();
		double dt = std::chrono::duration<double>(now - last).count();
		last = now;
		if (dt > 0.1) dt = 0.1; // do not let a stall become a huge jump
		if (scripted) dt = HEADLESS_DT;

		uint64_t frame_cycles = scripted ? HEADLESS_CYCLES_PER_FRAME
		                                 : (uint64_t) (measured_hz * SIM_BUDGET_SECONDS);

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = false;
			if (e.type == SDL_KEYDOWN &&
			    (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))
				running = false;
		}

		if (scripted) {
			// Trace a rectangle, save it, then hold clear and check it goes.
			int f = frame;
			left.direction = right.direction = 0;
			sim.dut->buttons = 0;
			if (f < 40) left.direction = 1;
			else if (f < 65) right.direction = 1;
			else if (f < 105) left.direction = -1;
			else if (f < 130) right.direction = -1;
			else if (f == 130) drawn = count_lit(sim);
			else if (headless) sim.dut->buttons = BUTTON_DIALL_CLICK;
			frame++;
		} else {
			const Uint8 *k = SDL_GetKeyboardState(nullptr);
			left.direction = (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D]) ? 1
			                 : (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A]) ? -1 : 0;
			right.direction = (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S]) ? 1
			                  : (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W]) ? -1 : 0;
			sim.dut->buttons = k[SDL_SCANCODE_SPACE] ? BUTTON_DIALL_CLICK : 0;
		}

		run_frame(sim, left, right, left.quarter_steps(dt), right.quarter_steps(dt),
		          frame_cycles);

		SDL_UpdateTexture(tex, nullptr, sim.pixels(), WIDTH * 2);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);

		double since = std::chrono::duration<double>(now - last_report).count();
		if (since >= 1.0) {
			measured_hz = (sim.cycles - last_cycles) / since;
			char title[128];
			std::snprintf(title, sizeof title,
			              "Etch A Sketch - Clarvi RV32I, simulated at %.1f MHz",
			              measured_hz / 1e6);
			SDL_SetWindowTitle(win, title);
#ifdef __EMSCRIPTEN__
			// Show the rate on the page, where there is no title bar.
			EM_ASM({
				var el = document.getElementById('rate');
				if (el) el.textContent = UTF8ToString($0);
			}, title);
#endif
			last_report = now;
			last_cycles = sim.cycles;
		}

#ifndef __EMSCRIPTEN__
		if (headless) {
			if (frame == 131 && !shot.empty()) {
				save_png(sim, shot); // save the drawing before the clear wipes it
				shot.clear();
			}
			if (frame >= 190) running = false;
		}
#endif
	}
};

static App *g_app = nullptr;
static void frame_callback() { g_app->step(); }

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	static App app;
	g_app = &app;

	for (int i = 1; i < argc; i++) {
		std::string a = argv[i];
		if (a == "--headless") app.headless = app.scripted = true;
		else if (a.rfind("--shot=", 0) == 0) app.shot = a.substr(7);
	}
	if (app.headless)
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "dummy");
#ifdef __EMSCRIPTEN__
	// ?demo on the URL runs the same canned input the native self-test uses,
	// so the browser build can be checked without a pair of hands.
	app.scripted = EM_ASM_INT({ return location.search.indexOf('demo') >= 0 ? 1 : 0; });
#endif

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}
	app.win = SDL_CreateWindow("Etch A Sketch - Clarvi RV32I, simulated",
	                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                           WIDTH * SCALE, HEIGHT * SCALE, 0);
	// Let SDL pick the backend. Forcing SDL_RENDERER_ACCELERATED fails wherever
	// WebGL is unavailable -- headless browsers, blocklisted GPUs -- and there
	// is nothing here that a software renderer cannot keep up with.
	app.ren = SDL_CreateRenderer(app.win, -1, 0);
	app.tex = SDL_CreateTexture(app.ren, SDL_PIXELFORMAT_RGB565,
	                            SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
	if (!app.win || !app.ren || !app.tex) {
		std::fprintf(stderr, "SDL setup failed: %s\n", SDL_GetError());
		return 1;
	}

	app.sim.reset();
	app.last = app.last_report = std::chrono::steady_clock::now();

#ifdef __EMSCRIPTEN__
	// Interactively, let the browser schedule frames. The canned demo asks for
	// a fixed 60, which Emscripten drives from a timer instead -- deterministic,
	// and it still runs where requestAnimationFrame does not, such as a headless
	// browser taking a screenshot.
	emscripten_set_main_loop(frame_callback, app.scripted ? 60 : 0, 1);
	return 0;
#else
	while (app.running)
		app.step();

	if (app.headless) {
		size_t left_lit = count_lit(app.sim);
		std::printf("  drew %zu pixels, %zu remain after clearing\n", app.drawn, left_lit);
		if (app.drawn < 200 || app.drawn > 400) {
			std::printf("FAIL  drawing does not look like a rectangle outline\n");
			app.failures++;
		}
		if (left_lit != 0) {
			std::printf("FAIL  clear left %zu pixels lit\n", left_lit);
			app.failures++;
		}
		std::printf("%s  interactive front end\n", app.failures ? "FAIL" : "PASS");
	}

	SDL_DestroyTexture(app.tex);
	SDL_DestroyRenderer(app.ren);
	SDL_DestroyWindow(app.win);
	SDL_Quit();
	app.sim.dut->final();
	return app.failures ? 1 : 0;
#endif
}
