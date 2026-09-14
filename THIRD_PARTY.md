# Third-party components

The [LICENSE](LICENSE) at the root of this repository covers my own work: the
bare-metal software under [`software/`](software/), the custom Qsys peripherals
under [`hardware/ip/rotary_ctl/`](hardware/ip/rotary_ctl/),
[`hardware/ip/shift_reg_ctl/`](hardware/ip/shift_reg_ctl/) and
[`hardware/ip/seven_seg/`](hardware/ip/seven_seg/), and the modifications to the
top-level design.

Everything listed below was supplied by the University of Cambridge Computer
Laboratory as part of the ECAD and Architecture practical classes, or by Intel
(then Altera) with the Quartus toolchain. It remains under its original
copyright and licence.

## Clarvi RISC-V core — fetched, not vendored

| | |
|---|---|
| Upstream | <https://github.com/ucam-comparch/clarvi> |
| Location | `third_party/clarvi` (git submodule, pinned to `2b4cb95`) |
| Copyright | © 2016 Robert Eady |
| Licence | BSD 2-Clause |

Clarvi ("Computer LAboratory RISC-V Implementation") is a 6-stage in-order
RV32I core written for teaching at the Computer Laboratory. This repository
originally carried a copy of `clarvi.sv`, `clarvi_avalon.sv` and `riscv.svh`;
those files were byte-identical to upstream, so they have been replaced by a
pinned submodule.

[`hardware/ip/clarvi/clarvi_hw.tcl`](hardware/ip/clarvi/clarvi_hw.tcl) is a
local copy of the upstream Qsys component description, with its source paths
adjusted to point into the submodule. It is kept in this repository rather than
used from the submodule so that Qsys discovers exactly one `clarvi` component.

## PixelStream video controller — vendored

| | |
|---|---|
| Location | [`hardware/ip/pixel_stream/`](hardware/ip/pixel_stream/) |
| Copyright | University of Cambridge Computer Laboratory |
| Origin | ECAD and Architecture practical classes |

An Avalon-MM bus master that streams a framebuffer out to the LCD panel,
written in Bluespec SystemVerilog. Both the `.bsv` sources and the Verilog the
Bluespec compiler produced from them are included, because building the Verilog
requires a Bluespec licence that is not generally available. I am not aware of a
public upstream repository, so these files are vendored rather than fetched.

## DE1-SoC top-level template — vendored and modified

| | |
|---|---|
| Location | [`hardware/quartus/clarvi_fpga.sv`](hardware/quartus/clarvi_fpga.sv) |
| Copyright | © 2015 A. Theodore Markettos |
| Licence | BSD 2-Clause (retained in the file header) |

The port list, pin names and comments describing the DE1-SoC and the Cambridge
display board are from the lab template. The body of the module — the SoC and
hex-display instantiations — is mine.

## Intel / Altera IP

The Qsys systems instantiate stock Intel FPGA IP (`altera_avalon_pio`,
`altera_avalon_onchip_memory2`, `altera_pll` and the Avalon interconnect that
Qsys generates around them). None of it is redistributed here: Qsys regenerates
it from your Quartus installation, under the terms of the Intel Program License
Subscription Agreement. This is one reason the generated system directories are
listed in [`.gitignore`](.gitignore).

## Pin assignments

The pin assignments in
[`hardware/quartus/clarvi_fpga.qsf`](hardware/quartus/clarvi_fpga.qsf) and the
timing constraints in
[`hardware/quartus/toplevel.sdc`](hardware/quartus/toplevel.sdc) describe the
Terasic DE1-SoC board and the Cambridge display board, and come from the lab
template.
