#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 272

// Pixels are stored as 5 bits of red, 6 of green and 5 of blue...
#define PIXEL16(r, g, b) ((((r) & 0x1F) << 11) | (((g) & 0x3F) << 5) | (((b) & 0x1F) << 0))
// ...but it is easier to write colours as 8/8/8 and discard the low-order bits.
#define PIXEL24(r, g, b) PIXEL16((r) >> 3, (g) >> 2, (b) >> 3)

#define PIXEL_BLACK PIXEL24(0x00, 0x00, 0x00)
#define PIXEL_WHITE PIXEL24(0xFF, 0xFF, 0xFF)
#define PIXEL_RED   PIXEL24(0xFF, 0x00, 0x00)
#define PIXEL_GREEN PIXEL24(0x00, 0xFF, 0x00)
#define PIXEL_BLUE  PIXEL24(0x00, 0x00, 0xFF)

// Writes one pixel into the framebuffer. Coordinates outside the panel are ignored.
void vid_set_pixel(int x, int y, int colour);

// Fills the whole framebuffer with black.
void clear_screen(void);

#endif // DISPLAY_H
