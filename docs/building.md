# Building and running

## What you need

- **A Terasic DE1-SoC board** (Cyclone V `5CSEMA5F31C6`) and the **Cambridge
  display board** that plugs into GPIO1. The pin assignments and timing
  constraints in the Quartus project describe that pair of boards specifically;
  nothing else will work without reassigning pins.
- **Quartus Prime** with Platform Designer (Qsys). The project was built with
  16.1 Lite. Newer versions will offer to upgrade the Qsys systems and the Intel
  IP inside them — see [Known issues](#known-issues).
- **A RISC-V cross-compiler** targeting bare-metal RV32I.

### The cross-compiler

The Makefile looks for `riscv32-unknown-elf-gcc`. A 64-bit multilib toolchain
works just as well, since the ABI is passed explicitly:

```sh
# Debian/Ubuntu
sudo apt install gcc-riscv64-unknown-elf
make RISCV_PREFIX=riscv64-unknown-elf-

# macOS
brew tap riscv-software-src/riscv
brew install riscv-gnu-toolchain
make RISCV_PREFIX=riscv64-unknown-elf-
```

Or build [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain)
with `--with-arch=rv32i --with-abi=ilp32` to get the `riscv32-` prefix the
Makefile expects by default.

## First time: regenerate the Qsys systems

The repository holds the `.qsys` system descriptions but not the HDL Qsys
produces from them. That output is derived, runs to tens of megabytes, and
includes Intel IP that is not mine to redistribute. Generate it once:

1. Make sure the submodule is present, or the Clarvi component will not resolve:

   ```sh
   git submodule update --init
   ```

2. Open `hardware/quartus/clarvi_fpga.qpf` in Quartus.
3. Open **Tools → Platform Designer**, load `clarvi_soc.qsys`, and click
   **Generate HDL**. Accept the defaults.
4. Repeat for `hex_led.qsys`.

Platform Designer finds the five custom components through the
`IP_SEARCH_PATHS` setting in `clarvi_fpga.qsf`, which points at
`hardware/ip/`. If a component shows as missing, check that setting first.

## Build the hardware

From Quartus, **Processing → Start Compilation**. Or from the command line:

```sh
cd software
make build_fpga
```

This writes `hardware/quartus/output_files/clarvi_fpga.sof`.

## Build the software and run it

```sh
cd software
make              # compile and link, then produce build/mem.hex
make update-mem   # rewrite the on-chip RAM contents inside the .sof
make download     # program the board over JTAG
```

`make` produces, in `software/build/`:

| File | What it is |
|---|---|
| `mem.hex` | Intel HEX memory image, written into the FPGA's on-chip RAM |
| `mem.txt` | the same image as ASCII hex, one word per line, for simulation |
| `program.dump` | full disassembly, useful when something does not run |
| `program.elf`, `mem.bin` | intermediates |

`make update-mem` is the quick path: it patches the memory image into an
already-compiled bitfile via `quartus_cdb --update_mif`, so changing the C does
not mean re-synthesising the whole design. `make clean` removes the build
directory.

## Using it

Turn the left dial to move the cursor horizontally and the right dial to move it
vertically. Click either dial to clear the screen. `KEY[0]` on the DE1-SoC is the
system reset.

## Known issues

These are the things I know about but have not been able to verify, because I no
longer have the board or the 2017 toolchain. The design did compile and run at
the time.

- **The hex-display clock was wired to a non-existent signal.** The top level
  connected `hex_led`'s clock to `CLK_50`, but the DE1-SoC's clock port is called
  `CLOCK_50`. In SystemVerilog an undeclared identifier in a port connection
  becomes an implicit wire rather than an error, so this compiled while almost
  certainly leaving the hex displays dead. It is now wired to `CLOCK_50`,
  untested on hardware.
- **Newer Quartus versions will want to upgrade the IP.** The Qsys systems were
  written by 16.1. Opening them in a later release triggers an IP upgrade, which
  regenerates the Intel components. That is expected, but it does change the
  generated interconnect.
- **The LED ring PIO is wider than what it drives.** `led_pio` is 10 bits, but
  the top level connects its output to the single-bit `LEDRINGn` pin. Only the
  low bit does anything.
- **The debouncers never reset.** `rotary.sv` instantiates both `debounce`
  modules with their reset tied low, so they rely on the counter settling after
  power-up rather than on an explicit reset. It works, but it is not what the
  module intends.
- **SignalTap was disabled.** The project referenced a `stp1.stp` capture file
  that was never committed, which stops the build. Both SignalTap assignments
  have been removed from the `.qsf`.
- **There is no testbench.** Clarvi's own repository has a test suite; this
  project has none of its own.

## What changed in 2026

The repository was restructured before publishing. If you are comparing against
the original coursework submission:

- Build output (`db/`, `incremental_db/`, `output_files/`, generated Qsys
  directories, `software/build/`) was removed from the working tree and purged
  from the git history, taking the repository from 106 MB to under 1 MB.
- Sources moved into `hardware/` and `software/`; the duplicated `qsys/` tree and
  the superseded lab-exercise Qsys systems were removed. They remain in history.
- `clarvi_soc2.qsys` was a byte-identical copy of `clarvi_soc.qsys`. The copy is
  what the build actually used, so it was renamed to `clarvi_soc.qsys` and the
  top-level instantiation updated to match.
- The vendored Clarvi core became a pinned git submodule; the files were
  byte-identical to upstream.
- The Makefile pointed at `../quartus` and a project called `clarvi`, neither of
  which existed. It now points at the real paths.
- `txt2hex.py` was Python 2 only. The Python 3 port reproduces the 2017
  `mem.hex` byte-for-byte from the archived `mem.txt`.
- The C was tidied: `#import` became `#include`, dead code went, the framebuffer
  primitives moved into `display.c`, and headers were renamed to match their
  implementations. Behaviour is unchanged.
