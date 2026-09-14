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

.PHONY: all lint test soc waves app run web serve clean
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
	@$(VERILATOR) $(VFLAGS) $$(GFLAGS_$(1)) --top-module $(top_$(1)) $(WAIVERS) $$(src_$(top_$(1))) $(CURDIR)/tb/tb_$(1).cpp \
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
# -Wno-UNOPTFLAT: Clarvi's forwarding paths look like combinational loops to
# Verilator. It costs simulation speed, not correctness, and it is reported
# during a phase where the file-scoped waiver in waivers.vlt does not catch it
# on older Verilator releases.
SOC_VFLAGS := $(VFLAGS) -Wno-UNOPTFLAT -I$(CLARVI) --top-module soc_sim

$(BUILD)/Vsoc_sim: rtl/soc_sim.sv tb/tb_soc.cpp common/png.h common/tb.h | $(BUILD)
	@$(VERILATOR) $(SOC_VFLAGS) $(WAIVERS) $(SOC_SRCS) $(CURDIR)/tb/tb_soc.cpp \
	  -LDFLAGS -lz -o $(CURDIR)/$@ > $(BUILD)/soc.buildlog 2>&1 || { cat $(BUILD)/soc.buildlog; exit 1; }

$(BUILD)/Vsoc_sim_trace: rtl/soc_sim.sv tb/tb_soc.cpp common/png.h common/tb.h | $(BUILD)
	@$(VERILATOR) $(subst $(BUILD)/obj,$(BUILD)/obj_trace,$(SOC_VFLAGS)) --trace -CFLAGS -DTRACE $(WAIVERS) $(SOC_SRCS) $(CURDIR)/tb/tb_soc.cpp \
	  -LDFLAGS -lz -o $(CURDIR)/$@ > $(BUILD)/soctrace.buildlog 2>&1 || { cat $(BUILD)/soctrace.buildlog; exit 1; }

# ------------------------------------------------- interactive front end ---
SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
APP_VFLAGS := $(subst $(BUILD)/obj,$(BUILD)/obj_app,$(SOC_VFLAGS)) -O3

$(BUILD)/etch: app/etch.cpp rtl/soc_sim.sv common/png.h | $(BUILD)
	@test -n "$(SDL_CFLAGS)$(SDL_LIBS)" || { \
	  echo "SDL2 not found. Install it:  brew install sdl2   |   apt install libsdl2-dev"; exit 1; }
	@$(VERILATOR) $(APP_VFLAGS) $(WAIVERS) $(SOC_SRCS) $(CURDIR)/app/etch.cpp \
	  -CFLAGS "$(SDL_CFLAGS)" -LDFLAGS "$(SDL_LIBS) -lz" -o $(CURDIR)/$@ \
	  > $(BUILD)/app.buildlog 2>&1 || { cat $(BUILD)/app.buildlog; exit 1; }

FIRMWARE := ../software/build/mem.txt
$(FIRMWARE):
	@echo "Firmware image missing. Build it first:"; echo "    cd ../software && make"; exit 1

soc: $(BUILD)/Vsoc_sim $(FIRMWARE)
	$(BUILD)/Vsoc_sim +mem=$(FIRMWARE) +png=$(BUILD)/sketch.png

app: $(BUILD)/etch

run: $(BUILD)/etch $(FIRMWARE)
	$(BUILD)/etch +mem=$(FIRMWARE)

waves: $(BUILD)/Vsoc_sim_trace $(FIRMWARE)
	$(BUILD)/Vsoc_sim_trace +mem=$(FIRMWARE) +png=$(BUILD)/sketch.png +vcd=$(BUILD)/soc.vcd

# ------------------------------------------------------- browser build -----
# Verilator emits C++; Emscripten compiles that to WebAssembly. SDL2 comes from
# Emscripten's own port, and the firmware image is baked into the virtual file
# system so $readmemh finds it.
EMXX      ?= em++
VROOT     := $(shell $(VERILATOR) --getenv VERILATOR_ROOT)
WEB_OUT   := $(BUILD)/web
WEB_FLAGS := -O3 -std=c++17 -include cstdint \
             -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=0

web: $(WEB_OUT)/index.html

$(WEB_OUT)/index.html: app/etch.cpp web/shell.html rtl/soc_sim.sv $(FIRMWARE) | $(BUILD)
	@command -v $(EMXX) >/dev/null || { \
	  echo "Emscripten not found. Install it:  brew install emscripten  |  https://emscripten.org"; exit 1; }
	@mkdir -p $(WEB_OUT)
	@$(VERILATOR) --cc -O3 -Wno-fatal -Wno-UNOPTFLAT -Mdir $(BUILD)/obj_web \
	  -I$(CLARVI) --top-module soc_sim $(WAIVERS) $(SOC_SRCS) > $(BUILD)/web.buildlog 2>&1
	@$(EMXX) $(WEB_FLAGS) \
	  -I $(BUILD)/obj_web -I $(VROOT)/include -I $(VROOT)/include/vltstd \
	  $(BUILD)/obj_web/*.cpp $(VROOT)/include/verilated.cpp $(VROOT)/include/verilated_threads.cpp \
	  app/etch.cpp --embed-file $(FIRMWARE)@mem.txt --shell-file web/shell.html \
	  -o $@ >> $(BUILD)/web.buildlog 2>&1 || { cat $(BUILD)/web.buildlog; exit 1; }
	@echo "  built $@  ($$(du -h $(WEB_OUT)/index.wasm | cut -f1) of WebAssembly)"

serve: web
	@echo "http://localhost:8000/   (append ?demo to watch it draw by itself)"
	@cd $(WEB_OUT) && python3 -m http.server 8000

$(BUILD):
	@mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
