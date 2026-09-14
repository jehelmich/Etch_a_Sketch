# Architecture

## The SoC

[`hardware/quartus/clarvi_soc.qsys`](../hardware/quartus/clarvi_soc.qsys)
describes the system that Qsys assembles. A PLL takes the board's 50 MHz
oscillator and produces a 50 MHz system clock and a 9 MHz pixel clock for the
LCD panel.

| Component | Kind | Role |
|---|---|---|
| `clarvi_0` | Clarvi | RV32I core; separate instruction and data masters |
| `onchip_memory2_0` | Intel on-chip RAM | 64 KiB, holds code, data and stack |
| `video_memory` | Intel on-chip RAM | 261,120 B framebuffer (480 × 272 × 2) |
| `PixelStream_0` | Bluespec | Avalon bus master; streams the framebuffer to the LCD |
| `leftdial_pio` / `rightdial_pio` | Intel PIO | 8-bit inputs, dial positions |
| `buttons_pio` | Intel PIO | 16-bit input, display-board buttons |
| `hex_pio` | Intel PIO | 24-bit output, six hex digits |
| `led_pio` | Intel PIO | 10-bit output, LED ring |
| `RotaryCtl2_left` / `RotaryCtl2_right` | custom | quadrature decoders |
| `ShiftRegCtl_0` | custom | reads the button shift register |

The hex displays sit in a second, much smaller Qsys system
([`hex_led.qsys`](../hardware/quartus/hex_led.qsys)) containing three
`EightBitsToSevenSeg` instances. The top level splits the 24-bit `hex_pio` output
into three bytes and feeds one to each.

## Memory map

The core sees a flat 32-bit address space. These constants are defined in
[`software/src/avalon_addr.h`](../software/src/avalon_addr.h) and must match the
base addresses in the Qsys system.

| Address | Size | Peripheral | Access |
|---|---|---|---|
| `0x00000000` | 64 KiB | code, data and stack RAM | read/write |
| `0x04000000` | 10 bits | LED ring PIO | write |
| `0x04000080` | 24 bits | hex display PIO | write |
| `0x04000100` | 8 bits | left dial position | read |
| `0x04000200` | 8 bits | right dial position | read |
| `0x04000300` | 16 bits | buttons | read |
| `0x04001000` | — | PixelStream control registers | read/write |
| `0x08000000` | 255 KiB | framebuffer | read/write |

`avalon_addr.h` also defines `DEBUG_PRINT` at `0x07000000`. That is a simulator
convenience and is not present in this SoC.

The stack pointer is set to the top of the 64 KiB RAM by
[`software/src/init.s`](../software/src/init.s), using the `__sp` symbol that
[`software/link.ld`](../software/link.ld) hardcodes to `0x10000`. If you resize
`onchip_memory2_0`, change the linker script to match.

## Peripherals

### RotaryCtl2 — quadrature decoding

[`hardware/ip/rotary_ctl/`](../hardware/ip/rotary_ctl/)

Each dial produces two phase signals. `debounce.sv` synchronises each phase into
the clock domain through a two-flop synchroniser, then requires it to hold steady
for 2^15 clocks (about 650 µs at 50 MHz) before accepting the new value.
`rotary.sv` watches the debounced pair for transitions and increments or
decrements an 8-bit `rotary_pos` counter according to which phase moved first. It
also emits single-cycle `rot_cw` / `rot_ccw` pulses, which this design leaves
unconnected.

The counter is free-running and wraps at 8 bits. The software never reads it as
an absolute position — only as a value to compare against the previous reading.

### ShiftRegCtl — reading the buttons

[`hardware/ip/shift_reg_ctl/`](../hardware/ip/shift_reg_ctl/)

The display board's sixteen buttons hang off a parallel-in serial-out shift
register. `shiftregctl.sv` divides the 50 MHz clock down by 512, pulses
`shiftreg_loadn` to latch the button states, then clocks sixteen bits out and
presents them as a 16-bit word. The bit assignments are the `BUTTONS_MASK_*`
constants in `avalon_addr.h`; the sketch uses `DIALL_CLICK` and `DIALR_CLICK`.

### EightBitsToSevenSeg

[`hardware/ip/seven_seg/`](../hardware/ip/seven_seg/)

A combinational lookup from one byte to two seven-segment digit patterns,
inverted because the DE1-SoC's displays are active low. Qsys components must
have a clock and reset, so it takes both and uses neither.

### PixelStream

[`hardware/ip/pixel_stream/`](../hardware/ip/pixel_stream/)

Supplied by the lab, written in Bluespec SystemVerilog. It is an Avalon bus
master: it burst-reads the framebuffer and drives the LCD panel's RGB, sync and
data-enable signals at the 9 MHz pixel clock, without involving the processor.
See [THIRD_PARTY.md](../THIRD_PARTY.md).

## The software

[`software/src/`](../software/src/) is four small translation units:

| File | Contents |
|---|---|
| `init.s` | startup: sets up the stack, installs a trap handler, calls `main` |
| `main.c` | calls `etch_a_sketch()` |
| `etch_a_sketch.c` | the sketch loop |
| `display.c` | framebuffer primitives and the 16-bit pixel format |
| `peripherals.c` | 32-bit Avalon reads and writes |

There is no C library and no operating system. `link.ld` places `.text.init`
first so that the reset vector lands on `init.s`, and everything is compiled
`-march=rv32i -O0`.

`init.s` installs a handler at CSR `0x305` that spins, because on real hardware
`ECALL` does not stop the processor the way it does in the simulator. The sketch
loop never returns anyway.

### The sketch loop

Each iteration reads the buttons, then both dial counters:

- if either dial's click button is down, clear the screen and start over;
- otherwise compare each counter against the previous reading, move the cursor
  one pixel in the corresponding direction, clamp it to the panel, and write a
  white pixel.

Polling is deliberately simple — there are no interrupts in this design. The
dials advance far more slowly than the loop runs, so a turn is never missed, and
the hardware counter absorbs anything that happens between polls.
