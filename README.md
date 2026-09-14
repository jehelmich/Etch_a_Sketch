# Etch A Sketch on a RISC-V soft core

[![CI](https://github.com/jehelmich/Etch_a_Sketch/actions/workflows/ci.yml/badge.svg)](https://github.com/jehelmich/Etch_a_Sketch/actions/workflows/ci.yml)

An [Etch A Sketch](https://en.wikipedia.org/wiki/Etch_A_Sketch) built on a
Terasic DE1-SoC FPGA board: two rotary dials draw a line on an LCD panel, and
pressing either dial clears the screen.

The interesting part is what sits underneath. Rather than drawing the line in
hardware, the design synthesises a **32-bit RISC-V processor** onto the FPGA and
runs a bare-metal C program on it. The dials, buttons, hex displays and
framebuffer are all memory-mapped peripherals on an Avalon bus, so the sketch
itself is about sixty lines of ordinary C.

### [Draw on it in your browser →](https://jehelmich.github.io/Etch_a_Sketch/)

![A rectangle drawn by the simulated SoC](docs/images/sketch.png)

No FPGA involved, and no reimplementation in JavaScript. The SystemVerilog is
translated to C++ by [Verilator](https://verilator.org) and compiled to
WebAssembly, so the processor really is executing in the tab. Your keypresses
become quadrature transitions on the pins the rotary encoders drive on the
board; the core polls them and plots pixels. It runs at about 7 MHz there
against the hardware's 50 MHz, which for something driven by a human hand is
not noticeable.

The same thing runs natively, and headless:

```sh
sudo apt install verilator zlib1g-dev gcc-riscv64-unknown-elf libsdl2-dev
git clone --recurse-submodules https://github.com/jehelmich/Etch_a_Sketch.git
cd Etch_a_Sketch/software && make RISCV_PREFIX=riscv64-unknown-elf-
cd ../sim && make run      # a window, arrow keys turn the dials
cd ../sim && make soc      # no window: draws a rectangle, writes a PNG
```

```
  drew 260 pixels in 280016 cycles
  wrote build/sketch.png
PASS  soc                      5 checks
```

The browser page only lets you drive the device. To change it — the drawing
code, the peripherals, the processor — clone the repository:
[docs/simulation.md](docs/simulation.md) explains what is real RTL and what is
a model, and [docs/extending.md](docs/extending.md) walks through adding a
peripheral end to end.

Undergraduate coursework for the University of Cambridge
[ECAD and Architecture practical classes](https://www.cl.cam.ac.uk/teaching/1617/ECAD+Arch/),
November 2017. See [Status](#status) before trying to build the FPGA image.

## How it works

```
   left dial  ──▶ RotaryCtl2 ──┐
   right dial ──▶ RotaryCtl2 ──┤
   buttons    ──▶ ShiftRegCtl ─┤                  ┌──────────────┐
                               ├──▶ Avalon MM ◀──▶│ Clarvi RV32I │
   hex displays ◀── hex PIO ───┤     interconnect └──────────────┘
                               │
                               ├──▶ 64 KiB on-chip RAM  (code + data)
                               │
                               └──▶ 255 KiB framebuffer ──▶ PixelStream ──▶ LCD
```

A [Clarvi](https://github.com/ucam-comparch/clarvi) core — a 6-stage in-order
RV32I implementation written at the Computer Laboratory for teaching — executes
from 64 KiB of on-chip RAM. Its data bus reaches five PIO peripherals and a
second block of on-chip RAM used as a 480×272 framebuffer at 16 bits per pixel.
The PixelStream unit is a bus master: it burst-reads that framebuffer and
streams it out to the LCD panel at a 9 MHz pixel clock, independently of the
processor.

The two rotary dials are quadrature encoders. Each drives a `RotaryCtl2`
peripheral that debounces both phases and maintains a free-running 8-bit
position counter. The software polls both counters and compares them against the
previous reading, so it only needs to know which way each dial turned — see
[`software/src/etch_a_sketch.c`](software/src/etch_a_sketch.c).

Full details, including the memory map, are in
[docs/architecture.md](docs/architecture.md).

## Repository layout

```
hardware/
  quartus/        Quartus project, top-level SystemVerilog, Qsys systems, timing constraints
  ip/             Qsys component descriptions and their HDL
    rotary_ctl/     quadrature decoder + debouncer for the dials
    shift_reg_ctl/  reads the display board's buttons over a shift register
    seven_seg/      byte to a pair of seven-segment digits
    pixel_stream/   framebuffer-to-LCD streaming master (supplied by the lab)
    clarvi/         Qsys component description for the core in third_party/
software/
  src/            bare-metal C and the RISC-V startup assembly
  tools/          memory-image conversion helper
  Makefile        builds the program and writes it into the FPGA bitfile
sim/
  rtl/            stands in for the Qsys interconnect
  tb/             Verilator testbenches, per peripheral and for the whole SoC
  app/            interactive front end, SDL2 natively and in the browser
  web/            the page the WebAssembly build is served in
third_party/
  clarvi/         the RISC-V core, as a pinned git submodule
docs/             architecture, build, simulation and extension notes
```

## Building

Three things can be built independently.

```sh
make -C sim lint                 # static checks over the hand-written RTL
make -C sim test                 # unit-test each peripheral
make -C software                 # cross-compile the firmware
make -C sim soc                  # run that firmware on the simulated SoC
make -C sim run                  # the same, with a window and working dials
make -C sim serve                # build for the browser and serve it
```

For the FPGA image you need Quartus Prime, the DE1-SoC and the Cambridge
display board, and a one-off Qsys generation step —
[docs/building.md](docs/building.md).

## Status

Archived university work, restructured in 2026 and given a test suite it never
had. What is and is not verified:

- **The RTL and firmware run.** The peripherals are unit-tested, and the whole
  SoC executes the real firmware under Verilator on every push, natively and as
  WebAssembly. That covers the Clarvi core, both rotary decoders, the button
  scanner and the memory map.
- **The FPGA build has not been reproduced.** It compiled cleanly in 2017 —
  2,803 ALMs, 9% of the Cyclone V, with 64% of its block RAM given over to the
  framebuffer — but I no longer have the board or that toolchain. Known
  discrepancies are in [docs/building.md](docs/building.md#known-issues).
- **The generated Qsys output is not in the repository**, so the Quartus flow
  needs a Qsys generation step first. The simulation sidesteps this by modelling
  the interconnect directly.

## Licence

My own work is BSD 2-Clause — see [LICENSE](LICENSE). The repository also
contains and references code from the Computer Laboratory and from Intel, which
stays under its original terms; [THIRD_PARTY.md](THIRD_PARTY.md) says what came
from where.
