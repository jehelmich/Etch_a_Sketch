// Minimal test scaffolding shared by the unit testbenches.
#pragma once
#include <cstdio>
#include <cstdlib>
#include <string>

namespace tb {

inline int checks = 0;
inline int failures = 0;

inline void check(bool ok, const std::string &what) {
	checks++;
	if (!ok) {
		failures++;
		std::printf("  FAIL  %s\n", what.c_str());
	}
}

template <typename A, typename B>
inline void check_eq(A got, B want, const std::string &what) {
	checks++;
	if (got != want) {
		failures++;
		std::printf("  FAIL  %s: got %lld, want %lld\n", what.c_str(),
		            (long long) got, (long long) want);
	}
}

inline int report(const char *name) {
	if (failures == 0) {
		std::printf("PASS  %-24s %d checks\n", name, checks);
		return 0;
	}
	std::printf("FAIL  %-24s %d of %d checks failed\n", name, failures, checks);
	return 1;
}

// Advances a Verilated model with an explicit `clk` port by whole cycles.
template <typename T>
inline void tick(T *dut, int cycles = 1) {
	for (int i = 0; i < cycles; i++) {
		dut->clk = 0;
		dut->eval();
		dut->clk = 1;
		dut->eval();
	}
}

} // namespace tb
