# Running it without the board

The DE1-SoC and the Cambridge display board are hard to come by, so the design
also runs under [Verilator](https://verilator.org). The simulation executes the
same firmware, on the same processor RTL, driving the same peripherals, and
writes out what the program drew.

![The rectangle the simulation drew](images/sketch.png)

That picture is not a mock-up. It is the framebuffer contents after the
simulated RISC-V core executed `software/src/etch_a_sketch.c` while the
testbench turned the dials.

## Prerequisites

```sh
# Debian / Ubuntu
sudo apt install verilator zlib1g-dev gcc-riscv64-unknown-elf

# macOS
brew install verilator riscv64-elf-gcc
```

Verilator 5 or later. The RISC-V compiler is only needed to build the firmware;
the peripheral tests run without it.

## What you can run

```sh
make -C sim lint     # static checks over the hand-written RTL
make -C sim test     # unit-test each peripheral
make -C sim soc      # run the firmware on the whole SoC, write a PNG
make -C sim run      # the same, in a window, with working dials
make -C sim serve    # build for the browser and serve it on :8000
```

`make -C sim soc` needs a firmware image first:

```sh
cd software
make                                    # or: make RISCV_PREFIX=riscv64-elf-
cd ../sim && make soc
```

Expected output:

```
  drew 260 pixels in 280016 cycles
  wrote build/sketch.png
PASS  soc                      5 checks
```

Roughly six million simulated cycles, in about half a second.

## Driving it by hand

`make -C sim run` opens a window on the simulation. Arrow keys or WASD turn the
dials, space clears the screen, Q quits. It needs SDL2:

```sh
sudo apt install libsdl2-dev     # or: brew install sdl2
```

The title bar reports the simulated clock rate and the frame rate. On an M3 Pro
that is about 8.7 MHz natively and 7 MHz through WebAssembly, against the
board's 50 MHz.

Frames are paced by the display. Natively that means vsync, falling back to a
deadline-scheduled limiter if the renderer refuses it; in the browser
`requestAnimationFrame` does the same job. Each frame simulates a slice of the
refresh interval — 80% of it — sized from how many cycles the machine managed
per second of actual simulation, which is measured separately from wall-clock
time. Measuring it wall-clock instead is a feedback loop: pacing the frames adds
idle time, which lowers the apparent rate, which shrinks the next budget, which
lowers it further, and the whole thing winds down.

The budget is capped at the display's own refresh interval rather than the last
frame's duration, so that missing one frame does not double the next budget and
lock the rate at half speed.

```sh
sim/build/etch --stats          # print the rate, and what is pacing the frames
sim/build/etch --fps=30         # only used when vsync is unavailable
```
That sounds like a problem and is not: a dial detent takes milliseconds of
simulated time against a 655 us debounce window, and the polling loop runs tens
of thousands of times a second against a hand that manages a hundred. The one
visible consequence is that clearing the screen -- 130,560 pixels, written one
at a time by the processor -- takes about a quarter of a second.

`--headless` runs a canned input sequence with fixed timing instead of reading
the keyboard, draws a rectangle, clears it, and reports whether both worked.
That is what CI runs, since a runner has no display:

```sh
sim/build/etch --headless +mem=software/build/mem.txt
```

## In a browser

```sh
make -C sim serve      # then open http://localhost:8000/
```

Verilator emits C++, and Emscripten compiles that to WebAssembly; SDL2 comes
from Emscripten's own port, and the firmware image is baked into the virtual
file system so `$readmemh` still finds it.

The page has drag controls as well as the keyboard, so it works on a phone.
They do not synthesise key events: the page calls an exported function,
`etch_input(left, right, clear)`, with each dial's direction and a speed between
0 and 1 taken from how far the finger has travelled. That matters on a touch
device, where the canvas cannot take keyboard focus at all. Keys still win when
both are in play.

Two query parameters help when checking a build:

| | |
|---|---|
| `?demo` | run the canned input the headless self-test uses, no hands needed |
| `?fps=N` | drive the frame loop from a timer at N fps instead of `requestAnimationFrame`, which does not fire in a headless browser |

Two things needed working around, both worth knowing if you touch the build:

- Emscripten's headers define `CPU_ZERO`, so Verilator's host-introspection
  helper takes its Linux path and calls `pthread_getaffinity_np` and friends,
  which the runtime only provides when pthreads are enabled. Enabling pthreads
  would mean SharedArrayBuffer and COOP/COEP response headers, which a static
  host will not necessarily send. `sim/app/etch.cpp` answers those three
  functions directly instead — a browser tab has one thread.
- The renderer must not demand `SDL_RENDERER_ACCELERATED`. Where WebGL is
  unavailable the call fails and takes the whole program with it, and nothing
  here troubles a software renderer.

The page is deployed to GitHub Pages from `.github/workflows/pages.yml` on every
push to `main`.

## What is actually being simulated

On the board, Qsys generates the Avalon interconnect that ties everything
together. Qsys output is a build artifact and is not in this repository, so
[`sim/rtl/soc_sim.sv`](../sim/rtl/soc_sim.sv) takes its place: it decodes the
memory map from [`avalon_addr.h`](../software/src/avalon_addr.h) and attaches
the peripherals itself.

| Piece | In simulation |
|---|---|
| Clarvi RV32I core | the real RTL, from the submodule |
| `RotaryCtl2` × 2 | the real RTL, with a shortened debounce window |
| `ShiftRegCtl` | the real RTL, plus a model of the display board's shift register |
| Avalon interconnect | replaced by an address decoder in `soc_sim.sv` |
| Code RAM, framebuffer | behavioural memories; the RAM is loaded from `mem.txt` |
| PIOs | plain registers |
| PixelStream, PLL, LCD panel | not modelled — the framebuffer is read out directly |
| `EightBitsToSevenSeg` | not in the SoC model; covered by its own unit test |

Two details are worth knowing if you change the model.

**The core only tolerates a fixed one-cycle read latency.** `clarvi_avalon.sv`
says so in a comment, and means it: every read must present its data on the
next cycle with `readdatavalid` asserted, and `waitrequest` is only sampled
during an active transfer. A variable-latency slave will not work.

**Addresses on the bus are word addresses.** Inside the core,
`{high_bits, main_address, word_offset} = byte_address`, so `main_address` is
the byte address shifted right by two. The parameter `DATA_ADDR_WIDTH` has to be
wide enough to reach the top of the map — 28 bits here, to address the
framebuffer at `0x08000000`. The instruction port stays at 14 bits, matching the
64 KiB of code RAM.

## The firmware image

`make` in `software/` produces `build/mem.txt`: one 32-bit word of ASCII hex per
line. That is exactly the format `$readmemh` expects, which is how the program
gets into the simulated RAM. The same file feeds the FPGA flow, by way of
`mem.hex`.

## Waveforms

```sh
make -C sim waves
```

writes `sim/build/soc.vcd` alongside the PNG. Open it with
[GTKWave](https://gtkwave.sourceforge.net) or [Surfer](https://surfer-project.org).
Useful signals: `debug_pc` to follow execution, `main_address` / `main_write`
for bus traffic, and `rotary_pos_l` / `rotary_pos_r` for what the decoders think
the dials are doing.

A full run would produce a trace far too large to be useful, so only a window
is dumped — by default the first 20,000 cycles, which covers reset, the first
instruction fetches and the initial pixel. That is already about 24 MB. Move
the window to look at something else:

```sh
make -C sim waves                       # cycles 0..20000
build/Vsoc_sim_trace +mem=../software/build/mem.txt +png=build/sketch.png \
    +vcd=build/soc.vcd +trace_from=150000 +trace_cycles=5000
```

VCD rather than FST on purpose: FST needs lz4 headers that are not on the
include path in every Verilator install, and a waveform dump is not worth a
build dependency that fails on someone else's machine.

## Why the debounce window is a parameter

On hardware the debouncer waits 2^15 clocks — about 655 µs at 50 MHz — before
accepting a new level. Simulating that for every phase change of every dial
would dominate the run, so `debounce` takes a `COUNTER_WIDTH` parameter that the
testbenches shorten. The default is unchanged, so the synthesised hardware still
waits the full interval.

The stimulus also has to turn the dials slowly enough for the polling loop to
see every count: the firmware moves the cursor one pixel per observed change,
not one per count. `PHASE_CLOCKS` in
[`sim/tb/tb_soc.cpp`](../sim/tb/tb_soc.cpp) sets that pace.

## Lint waivers

`make -C sim lint` runs with `-Wall`. The exceptions live in
[`sim/waivers.vlt`](../sim/waivers.vlt) and are scoped to individual files, so
that the vendored core is held to its own standard without loosening anything
for the RTL written here. Each waiver says why it exists.
