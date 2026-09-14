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
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
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

// Fraction of each frame spent simulating; the rest goes on presenting it and
// on leaving the machine some headroom. Sized against the frame period actually
// observed rather than a constant, so this behaves whether frames arrive every
// 16.7 ms on a 60 Hz display or every 8.3 ms on a 120 Hz one.
static const double SIM_BUDGET_FRACTION = 0.8;

// Frames per second to present at natively. The browser build ignores this:
// requestAnimationFrame already paces it to the display. 0 means free-running.
static const int TARGET_FPS = 60;

// A frame period outside this range is a stall or a spike, not a refresh rate.
static const double MIN_FRAME_PERIOD = 1.0 / 240.0;
static const double MAX_FRAME_PERIOD = 1.0 / 30.0;

// Never budget for a frame longer than the display's own period, even if the
// last frame took longer. Sizing purely off the measured period is a trap: miss
// one frame with vsync on and the period doubles, so the next budget doubles,
// so that frame is missed too, and the rate sticks at half speed. Capping the
// budget at the refresh interval lets it recover.
static const double DEFAULT_BUDGET_PERIOD = 1.0 / 60.0;

// The Gray sequence an encoder walks through, one step per quarter detent.
static const int GRAY[4] = {0b00, 0b01, 0b11, 0b10};

#ifdef __EMSCRIPTEN__
// Touch controls call this. Keeping it an explicit entry point rather than
// synthesising keyboard events means the page does not have to pretend to be a
// keyboard, and the canvas does not need focus -- which it cannot get by touch.
// left and right are -1..1; anything non-zero turns that dial at that fraction
// of full speed.
static double g_in_left = 0.0, g_in_right = 0.0;
static int g_in_clear = 0;

extern "C" EMSCRIPTEN_KEEPALIVE void etch_input(double left, double right, int clear) {
	g_in_left = left;
	g_in_right = right;
	g_in_clear = clear;
}
#endif

struct Dial {
	int phase = 0;
	double pending = 0.0;
	int direction = 0;   // -1, 0 or +1
	double speed = 1.0;  // 0..1; a key is always 1, a drag is proportional

	int quarter_steps(double dt) {
		if (direction != 0)
			pending += DETENTS_PER_SECOND * speed * dt;
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
	bool stats = false;      // also report the rate on stdout
	bool vsync = false;      // the display is pacing us; do not also sleep
	double budget_cap = DEFAULT_BUDGET_PERIOD; // set from the display refresh
	bool scripted = false;   // canned input instead of the keyboard
	std::string shot;
	bool running = true;
	int frame = 0;
	size_t drawn = 0;
	int failures = 0;

	std::chrono::steady_clock::time_point last, last_report;
	uint64_t last_cycles = 0;
	int frames_since_report = 0;
	double measured_hz = 10e6;     // cycles per wall second: what is displayed
	double measured_fps = 60.0;
	int target_fps = TARGET_FPS;

	// Cycles per second of time actually spent simulating. This is a property
	// of the machine, and is what sizes each frame's budget. Using the
	// wall-clock rate here instead would be a feedback loop: capping the frame
	// rate adds idle time, which lowers the apparent rate, which shrinks the
	// budget, which lowers it further.
	double sim_throughput = 10e6;
	double sim_seconds = 0.0;
	std::chrono::steady_clock::time_point next_frame;

	void step() {
		auto frame_start = std::chrono::steady_clock::now();
		double dt = std::chrono::duration<double>(frame_start - last).count();
		last = frame_start;

		// Clamp before using it for anything: the first frame, and any frame
		// after the window was dragged or the tab was hidden, reports nonsense.
		if (dt < MIN_FRAME_PERIOD) dt = MIN_FRAME_PERIOD;
		if (dt > MAX_FRAME_PERIOD) dt = MAX_FRAME_PERIOD;
		if (scripted) dt = HEADLESS_DT;

		double budget_period = dt < budget_cap ? dt : budget_cap;
		uint64_t frame_cycles =
		    scripted ? HEADLESS_CYCLES_PER_FRAME
		             : (uint64_t) (sim_throughput * budget_period * SIM_BUDGET_FRACTION);

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
			left.speed = right.speed = 1.0;
			bool clear = k[SDL_SCANCODE_SPACE];

#ifdef __EMSCRIPTEN__
			// A dial the keyboard is not already turning takes its direction
			// and speed from the touch controls instead.
			if (left.direction == 0 && g_in_left != 0.0) {
				left.direction = g_in_left > 0 ? 1 : -1;
				left.speed = std::fabs(g_in_left);
			}
			if (right.direction == 0 && g_in_right != 0.0) {
				right.direction = g_in_right > 0 ? 1 : -1;
				right.speed = std::fabs(g_in_right);
			}
			clear = clear || g_in_clear;
#endif
			sim.dut->buttons = clear ? BUTTON_DIALL_CLICK : 0;
		}

		auto sim_start = std::chrono::steady_clock::now();
		run_frame(sim, left, right, left.quarter_steps(dt), right.quarter_steps(dt),
		          frame_cycles);
		sim_seconds += std::chrono::duration<double>(
		                   std::chrono::steady_clock::now() - sim_start).count();

		SDL_UpdateTexture(tex, nullptr, sim.pixels(), WIDTH * 2);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);

		frames_since_report++;
		double since = std::chrono::duration<double>(frame_start - last_report).count();
		if (since >= 1.0) {
			measured_hz = (sim.cycles - last_cycles) / since;
			measured_fps = frames_since_report / since;
			if (sim_seconds > 0.0)
				sim_throughput = (sim.cycles - last_cycles) / sim_seconds;
			sim_seconds = 0.0;
			frames_since_report = 0;
			char title[160];
			std::snprintf(title, sizeof title,
			              "Etch A Sketch - Clarvi RV32I at %.1f MHz, %.0f fps",
			              measured_hz / 1e6, measured_fps);
			SDL_SetWindowTitle(win, title);
			if (stats) { std::printf("  %s\n", title); std::fflush(stdout); }
#ifdef __EMSCRIPTEN__
			// Show the rate on the page, where there is no title bar.
			EM_ASM({
				var el = document.getElementById('rate');
				if (el) el.textContent = UTF8ToString($0);
			}, title);
#endif
			last_report = frame_start;
			last_cycles = sim.cycles;
		}

#ifndef __EMSCRIPTEN__
		// Hold the presentation rate steady. Without this the loop free-runs at
		// whatever the machine happens to manage, which drifts with load.
		// In the browser requestAnimationFrame does this already, and sleeping
		// inside its callback would be the wrong thing entirely.
		if (!headless && !vsync && target_fps > 0) {
			// Schedule against a running deadline rather than sleeping for a
			// per-frame delta, so rounding error does not accumulate.
			auto period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
			    std::chrono::duration<double>(1.0 / target_fps));
			next_frame += period;
			auto now2 = std::chrono::steady_clock::now();
			if (next_frame < now2)
				next_frame = now2; // fell behind; do not try to catch up
			else
				std::this_thread::sleep_until(next_frame);
		}
#endif

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
		else if (a.rfind("--fps=", 0) == 0) app.target_fps = std::stoi(a.substr(6));
		else if (a == "--stats") app.stats = true;
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
	// Prefer to be paced by the display: vsync is exact, and sleeping for the
	// remainder of a frame is not -- the OS routinely overshoots a short sleep
	// by a millisecond or two, which shows up as a rate that will not sit still.
	// Fall back to no vsync, and then to the manual limiter, if it is refused.
	//
	// Do not force SDL_RENDERER_ACCELERATED: it fails wherever WebGL is
	// unavailable -- headless browsers, blocklisted GPUs -- and nothing here
	// troubles a software renderer.
	app.ren = SDL_CreateRenderer(app.win, -1,
	                             app.headless ? 0 : SDL_RENDERER_PRESENTVSYNC);
	if (!app.ren)
		app.ren = SDL_CreateRenderer(app.win, -1, 0);
	SDL_RendererInfo rinfo;
	if (app.ren && SDL_GetRendererInfo(app.ren, &rinfo) == 0)
		app.vsync = (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
	app.tex = SDL_CreateTexture(app.ren, SDL_PIXELFORMAT_RGB565,
	                            SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
	if (!app.win || !app.ren || !app.tex) {
		std::fprintf(stderr, "SDL setup failed: %s\n", SDL_GetError());
		return 1;
	}

	// Size the per-frame budget against the display we are actually on: a 50 Hz
	// panel gives 20 ms to work with, a 120 Hz one only 8.3 ms.
	SDL_DisplayMode mode;
	int hz = (SDL_GetCurrentDisplayMode(0, &mode) == 0) ? mode.refresh_rate : 0;
	if (hz >= 30 && hz <= 240)
		app.budget_cap = 1.0 / hz;
	if (app.stats)
		std::printf("  renderer %s, vsync %s, display %d Hz\n",
		            rinfo.name ? rinfo.name : "?", app.vsync ? "on" : "off", hz);

	app.sim.reset();
	app.last = app.last_report = app.next_frame = std::chrono::steady_clock::now();

#ifdef __EMSCRIPTEN__
	// Interactively, let the browser schedule frames: requestAnimationFrame
	// paces to the display, which is what we want. A fixed rate can be asked
	// for with ?fps=N -- the counterpart of the native --fps flag -- and the
	// canned demo uses it. Emscripten drives that from a timer, which also
	// runs where requestAnimationFrame does not, such as a headless browser
	// taking a screenshot.
	int fps = EM_ASM_INT({
		// No regex here: EM_ASM bodies go through the C preprocessor, which
		// mangles the backslash in a character class.
		var q = location.search;
		var i = q.indexOf('fps=');
		if (i < 0) return 0;
		var n = parseInt(q.substring(i + 4), 10);
		return (n > 0 && n <= 240) ? n : 0;
	});
	if (app.scripted && fps <= 0)
		fps = 60;
	emscripten_set_main_loop(frame_callback, fps, 1);
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
