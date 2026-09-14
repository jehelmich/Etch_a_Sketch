// A small PNG writer, enough to save the simulated framebuffer.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <zlib.h>

namespace png {

inline void be32(std::vector<uint8_t> &v, uint32_t x) {
	v.push_back(x >> 24); v.push_back(x >> 16); v.push_back(x >> 8); v.push_back(x);
}

inline void chunk(FILE *f, const char *type, const std::vector<uint8_t> &data) {
	std::vector<uint8_t> hdr;
	be32(hdr, (uint32_t) data.size());
	std::fwrite(hdr.data(), 1, hdr.size(), f);
	std::fwrite(type, 1, 4, f);
	std::fwrite(data.data(), 1, data.size(), f);

	uLong crc = crc32(0, (const Bytef *) type, 4);
	if (!data.empty())
		crc = crc32(crc, data.data(), (uInt) data.size());
	std::vector<uint8_t> tail;
	be32(tail, (uint32_t) crc);
	std::fwrite(tail.data(), 1, tail.size(), f);
}

// rgb is width*height*3 bytes.
inline bool write_rgb(const std::string &path, int width, int height,
                      const std::vector<uint8_t> &rgb) {
	FILE *f = std::fopen(path.c_str(), "wb");
	if (!f)
		return false;

	static const uint8_t sig[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	std::fwrite(sig, 1, 8, f);

	std::vector<uint8_t> ihdr;
	be32(ihdr, width);
	be32(ihdr, height);
	ihdr.push_back(8); // 8 bits per channel
	ihdr.push_back(2); // truecolour RGB
	ihdr.push_back(0); ihdr.push_back(0); ihdr.push_back(0);
	chunk(f, "IHDR", ihdr);

	// Each scanline is prefixed with a filter byte; 0 means no filtering.
	std::vector<uint8_t> raw;
	raw.reserve((size_t) height * (1 + (size_t) width * 3));
	for (int y = 0; y < height; y++) {
		raw.push_back(0);
		const uint8_t *row = rgb.data() + (size_t) y * width * 3;
		raw.insert(raw.end(), row, row + (size_t) width * 3);
	}

	uLongf bound = compressBound((uLong) raw.size());
	std::vector<uint8_t> z(bound);
	if (compress2(z.data(), &bound, raw.data(), (uLong) raw.size(), 9) != Z_OK) {
		std::fclose(f);
		return false;
	}
	z.resize(bound);
	chunk(f, "IDAT", z);
	chunk(f, "IEND", {});

	std::fclose(f);
	return true;
}

} // namespace png
