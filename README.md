# Etch A Sketch on a RISC-V soft core

An [Etch A Sketch](https://en.wikipedia.org/wiki/Etch_A_Sketch) built on a
Terasic DE1-SoC FPGA board: two rotary dials draw a line on an LCD panel, and
pressing either dial clears the screen.

The interesting part is what sits underneath. Rather than drawing the line in
hardware, the design synthesises a **32-bit RISC-V processor** onto the FPGA and
runs a bare-metal C program on it. The dials, buttons, hex displays and
framebuffer are all memory-mapped peripherals on an Avalon bus, so the sketch
itself is about sixty lines of ordinary C.

Undergraduate coursework for the University of Cambridge
[ECAD and Architecture practical classes](https://www.cl.cam.ac.uk/teaching/1617/ECAD+Arch/),
November 2017. Published as an archive — see [Status](#status) before trying to
build it.

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
third_party/
  clarvi/         the RISC-V core, as a pinned git submodule
docs/             architecture notes and build instructions
```

Clone with submodules, or the core will be missing:

```sh
git clone --recurse-submodules https://github.com/jehelmich/Etch_a_Sketch.git
```

## Building

Short version, once the prerequisites are in place:

```sh
cd software && make          # build the program image
make update-mem              # write it into the FPGA bitfile
make download                # program the board over JTAG
```

This needs a `riscv32-unknown-elf` GCC, Quartus Prime, the DE1-SoC board and the
Cambridge display board, and a one-off Qsys generation step. The full
instructions are in [docs/building.md](docs/building.md).

## Status

This is an archived university project, restructured and documented in 2026 but
not otherwise revived. Two things are worth knowing before you spend time on it:

- **The generated Qsys output is not in the repository.** Quartus and Qsys emit
  tens of megabytes of derived files, including Intel IP that is not mine to
  redistribute. You must regenerate the systems in Qsys before the project will
  compile. [docs/building.md](docs/building.md) covers this.
- **The 2017 build has not been reproduced.** The design compiled cleanly back
  then — 2,803 ALMs, 9% of the Cyclone V, with 64% of its block RAM given over
  to the framebuffer — but I no longer have the board or the toolchain, so the
  restructured tree here is unverified on hardware. Known discrepancies are
  listed in [docs/building.md](docs/building.md#known-issues).

## Licence

My own work is BSD 2-Clause — see [LICENSE](LICENSE). The repository also
contains and references code from the Computer Laboratory and from Intel, which
stays under its original terms; [THIRD_PARTY.md](THIRD_PARTY.md) says what came
from where.
