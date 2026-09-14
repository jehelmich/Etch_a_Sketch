#include "avalon_addr.h"
#include "display.h"

void vid_set_pixel(int x, int y, int colour)
{
	// the framebuffer is a flat array of 16-bit pixels, row-major
	volatile short *framebuffer = (volatile short *) (FRAMEBUFFER_BASE);

	// make sure we don't go past the edge of the screen
	if ((x < 0) || (x > DISPLAY_WIDTH - 1))
		return;
	if ((y < 0) || (y > DISPLAY_HEIGHT - 1))
		return;

	framebuffer[x + y * DISPLAY_WIDTH] = colour;
}

void clear_screen(void)
{
	for (int x = 0; x < DISPLAY_WIDTH; x++)
		for (int y = 0; y < DISPLAY_HEIGHT; y++)
			vid_set_pixel(x, y, PIXEL_BLACK);
}
