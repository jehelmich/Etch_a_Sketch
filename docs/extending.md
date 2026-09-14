# Adding a peripheral

A worked example: giving the SoC a free-running microsecond timer, so the
firmware has a timebase. It touches every layer — RTL, Qsys, the memory map,
the C, and the simulation — which is the point.

Read [simulation.md](simulation.md) first if you have not run anything yet.

## 1. Write the RTL

Peripherals live one directory each under `hardware/ip/`. Create
`hardware/ip/us_timer/us_timer.sv`:

```systemverilog
// A free-running microsecond counter, readable over Avalon-MM.
module us_timer #(
    parameter CLOCKS_PER_US = 50          // the SoC runs at 50 MHz
)(
    input  logic        clock,
    input  logic        reset,

    input  logic        avs_s0_read,
    output logic [31:0] avs_s0_readdata
);
    logic [31:0] micros;
    logic [$clog2(CLOCKS_PER_US)-1:0] divider;

    always_ff @(posedge clock) begin
        if (reset) begin
            micros  <= '0;
            divider <= '0;
        end else if (divider == CLOCKS_PER_US - 1) begin
            divider <= '0;
            micros  <= micros + 1;
        end else begin
            divider <= divider + 1;
        end

        // Clarvi requires a fixed one-cycle read latency.
        if (avs_s0_read)
            avs_s0_readdata <= micros;
    end
endmodule
```

The `avs_s0_` prefix is not decoration: Qsys reads it to work out that these
ports form an Avalon-MM slave called `s0`. The one-cycle latency is a hard
requirement of the core, not a style choice — see
[simulation.md](simulation.md#what-is-actually-being-simulated).

Check it before going further:

```sh
verilator --lint-only -Wall --top-module us_timer hardware/ip/us_timer/us_timer.sv
```

## 2. Test it

Add `sim/tb/tb_us_timer.cpp`:

```cpp
#include "Vus_timer.h"
#include "tb.h"
#include <memory>

int main(int argc, char **argv) {
	Verilated::commandArgs(argc, argv);
	auto dut = std::make_unique<Vus_timer>();

	dut->reset = 1;
	dut->avs_s0_read = 0;
	tb::tick(dut.get(), 2);   // note: tb::tick drives `clk`; see below
	dut->reset = 0;

	// 50 clocks per microsecond, so 5000 clocks is 100 us.
	tb::tick(dut.get(), 5000);
	dut->avs_s0_read = 1;
	tb::tick(dut.get(), 1);
	tb::check_eq(dut->avs_s0_readdata, 100, "counts 100 us in 5000 clocks");

	dut->final();
	return tb::report("us_timer");
}
```

`tb::tick` in [`sim/common/tb.h`](../sim/common/tb.h) drives a port called
`clk`. This module calls its clock `clock`, so either rename the port or clock
it by hand the way [`tb_shiftregctl.cpp`](../sim/tb/tb_shiftregctl.cpp) does.

Register it in [`sim/Makefile`](../sim/Makefile) by adding `us_timer` to
`UNITS` and `LINT_TOPS`, then:

```
src_us_timer := $(IP)/us_timer/us_timer.sv
top_us_timer := us_timer
```

`make -C sim test` will pick it up.

## 3. Describe it to Qsys

Qsys discovers components by scanning for `*_hw.tcl` under the path in
`IP_SEARCH_PATHS`, which `clarvi_fpga.qsf` sets to `../ip/**/*`. The quickest
route is to copy
[`hardware/ip/shift_reg_ctl/ShiftRegCtl_hw.tcl`](../hardware/ip/shift_reg_ctl/ShiftRegCtl_hw.tcl)
and edit the names, or to let Platform Designer generate one for you with
**File → New Component**.

The parts that matter:

```tcl
set_module_property NAME us_timer
set_module_property DISPLAY_NAME "Microsecond timer"

add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL us_timer
add_fileset_file us_timer.sv SYSTEM_VERILOG PATH us_timer.sv TOP_LEVEL_FILE

# one clock, one reset, one Avalon-MM slave with a single read-only register
add_interface clock      clock  end
add_interface reset      reset  end
add_interface s0         avalon end
set_interface_property s0 associatedClock clock
set_interface_property s0 associatedReset reset
set_interface_property s0 readLatency 1
```

`readLatency 1` has to match the RTL, and the RTL has to match what the core
expects. Getting this wrong produces a design that synthesises and then hangs.

Paths in `add_fileset_file` are relative to the `.tcl` file, which is why each
component keeps its HDL beside its description.

## 4. Add it to the SoC

Open `hardware/quartus/clarvi_soc.qsys` in Platform Designer, drop in the new
component, connect its clock and reset, and connect `s0` to the Clarvi `main`
master. Give it a base address in the peripheral region — the existing ones sit
at `0x04000000` and up, spaced well apart:

| Existing | Address |
|---|---|
| LED ring | `0x04000000` |
| Hex displays | `0x04000080` |
| Left dial | `0x04000100` |
| Right dial | `0x04000200` |
| Buttons | `0x04000300` |
| PixelStream control | `0x04001000` |

`0x04000400` is free. Then **Generate HDL** and recompile.

## 5. Put it in the memory map

Add it to [`software/src/avalon_addr.h`](../software/src/avalon_addr.h), which
is the one place the hardware and software agree on addresses:

```c
#define PIO_TIMER_US 0x04000400
```

## 6. Teach the simulation about it

The simulation does not use Qsys, so `soc_sim.sv` needs the same change by
hand. In [`sim/rtl/soc_sim.sv`](../sim/rtl/soc_sim.sv):

```systemverilog
localparam PIO_TIMER = 8'h00;  // word offset of byte 0x04000400 within its page
```

Instantiate the module, and add a case to the read multiplexer alongside
`PIO_DIALL` and the rest. Note that the decode uses *word* offsets: byte
`0x04000400` is word `0x04000400 >> 2`, so it lands on a different page of the
`main_address[7:0]` slice than the peripherals at `0x040000xx`. Widen the slice
if you add addresses that collide.

This duplication is the price of not committing Qsys output. It is a handful of
lines, and it keeps the simulation runnable by anyone without a Quartus install.

## 7. Use it

```c
#include "avalon_addr.h"
#include "peripherals.h"

unsigned int start = avalon_read(PIO_TIMER_US);
/* ... */
unsigned int elapsed = avalon_read(PIO_TIMER_US) - start;
```

Rebuild and run it:

```sh
cd software && make && cd ../sim && make soc
```

## Changing the firmware only

Most changes do not need any of the above. The sketch loop is
[`software/src/etch_a_sketch.c`](../software/src/etch_a_sketch.c); the
framebuffer primitives and the 16-bit pixel format are in
[`display.c`](../software/src/display.c). Colours are already defined —
`PIXEL_RED`, `PIXEL_GREEN`, `PIXEL_BLUE` — and nothing uses them yet.

```sh
cd software && make && cd ../sim && make soc
```

The assertions in [`tb_soc.cpp`](../sim/tb/tb_soc.cpp) expect a 260-pixel
rectangle, so expect to adjust them if you change what gets drawn.
