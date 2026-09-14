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

## The three things you can run

```sh
make -C sim lint     # static checks over the hand-written RTL
make -C sim test     # unit-test each peripheral
make -C sim soc      # run the firmware on the whole SoC, write a PNG
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
