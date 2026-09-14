# Simulation and lint for the custom peripherals and the whole SoC.
#
#   make lint      static checks over the hand-written RTL
#   make test      build and run the unit testbenches
#   make soc       run the firmware on the simulated SoC, write a PNG
#   make waves     as above, but also dump build/soc.fst
#   make clean

VERILATOR ?= verilator
IP        := ../hardware/ip
CLARVI    := ../third_party/clarvi
BUILD     := build
WAIVERS   := waivers.vlt

VFLAGS    := -Wall --cc --exe --build -j 0 -Mdir $(BUILD)/obj -CFLAGS -I$(CURDIR)/common

# --- the hand-written peripherals, and the sources each testbench needs ------
LINT_TOPS := debounce rotary shiftregctl EightBitsToSevenSeg

src_debounce           := $(IP)/rotary_ctl/debounce.sv
src_rotary             := $(IP)/rotary_ctl/rotary.sv $(IP)/rotary_ctl/debounce.sv
src_shiftregctl        := $(IP)/shift_reg_ctl/shiftregctl.sv
src_EightBitsToSevenSeg := $(IP)/seven_seg/EightBitsToSevenSeg.sv

UNITS     := debounce rotary shiftregctl seven_seg
top_debounce    := debounce
top_rotary      := rotary
GFLAGS_rotary   := -GDEBOUNCE_COUNTER_WIDTH=4
top_shiftregctl := shiftregctl
top_seven_seg   := EightBitsToSevenSeg

.PHONY: all lint test soc waves clean
all: lint test

# ---------------------------------------------------------------------------
lint:
	@rc=0; for t in $(LINT_TOPS); do \
	  case $$t in \
	    debounce)           s="$(src_debounce)";; \
	    rotary)             s="$(src_rotary)";; \
	    shiftregctl)        s="$(src_shiftregctl)";; \
	    EightBitsToSevenSeg) s="$(src_EightBitsToSevenSeg)";; \
	  esac; \
	  if $(VERILATOR) --lint-only -Wall --top-module $$t $(WAIVERS) $$s >$(BUILD)/$$t.lint 2>&1; then \
	    printf "  lint ok   %s\n" "$$t"; \
	  else \
	    printf "  LINT FAIL %s\n" "$$t"; cat $(BUILD)/$$t.lint; rc=1; \
	  fi; \
	done; exit $$rc
lint: | $(BUILD)

# ---------------------------------------------------------------------------
define UNIT_rule
$(BUILD)/V$(top_$(1)): tb/tb_$(1).cpp common/tb.h $$(src_$(top_$(1))) $(WAIVERS) | $(BUILD)
	@$(VERILATOR) $(VFLAGS) $$(GFLAGS_$(1)) --top-module $(top_$(1)) $(WAIVERS) $$(src_$(top_$(1))) tb/tb_$(1).cpp \
	  -o $(CURDIR)/$$@ > $(BUILD)/$(1).buildlog 2>&1 || { cat $(BUILD)/$(1).buildlog; exit 1; }
endef
$(foreach u,$(UNITS),$(eval $(call UNIT_rule,$(u))))

test: $(foreach u,$(UNITS),$(BUILD)/V$(top_$(u)))
	@rc=0; for u in $(UNITS); do \
	  case $$u in \
	    debounce)    b=Vdebounce;; rotary) b=Vrotary;; \
	    shiftregctl) b=Vshiftregctl;; seven_seg) b=VEightBitsToSevenSeg;; \
	  esac; \
	  $(BUILD)/$$b || rc=1; \
	done; exit $$rc

# ---------------------------------------------------------------------------
SOC_SRCS := rtl/soc_sim.sv \
            $(IP)/rotary_ctl/rotary.sv $(IP)/rotary_ctl/debounce.sv \
            $(IP)/shift_reg_ctl/shiftregctl.sv \
            $(CLARVI)/clarvi_avalon.sv $(CLARVI)/clarvi.sv
SOC_VFLAGS := $(VFLAGS) -I$(CLARVI) --top-module soc_sim

$(BUILD)/Vsoc_sim: rtl/soc_sim.sv tb/tb_soc.cpp common/png.h common/tb.h | $(BUILD)
	@$(VERILATOR) $(SOC_VFLAGS) $(WAIVERS) $(SOC_SRCS) tb/tb_soc.cpp \
	  -LDFLAGS -lz -o $(CURDIR)/$@ > $(BUILD)/soc.buildlog 2>&1 || { cat $(BUILD)/soc.buildlog; exit 1; }

$(BUILD)/Vsoc_sim_trace: rtl/soc_sim.sv tb/tb_soc.cpp common/png.h common/tb.h | $(BUILD)
	@$(VERILATOR) $(SOC_VFLAGS) --trace-fst -CFLAGS -DTRACE $(WAIVERS) $(SOC_SRCS) tb/tb_soc.cpp \
	  -LDFLAGS -lz -Mdir $(BUILD)/obj_trace -o $(CURDIR)/$@ > $(BUILD)/soctrace.buildlog 2>&1 || { cat $(BUILD)/soctrace.buildlog; exit 1; }

FIRMWARE := ../software/build/mem.txt
$(FIRMWARE):
	@echo "Firmware image missing. Build it first:"; echo "    cd ../software && make"; exit 1

soc: $(BUILD)/Vsoc_sim $(FIRMWARE)
	$(BUILD)/Vsoc_sim +mem=$(FIRMWARE) +png=$(BUILD)/sketch.png

waves: $(BUILD)/Vsoc_sim_trace $(FIRMWARE)
	$(BUILD)/Vsoc_sim_trace +mem=$(FIRMWARE) +png=$(BUILD)/sketch.png +fst=$(BUILD)/soc.fst

$(BUILD):
	@mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
