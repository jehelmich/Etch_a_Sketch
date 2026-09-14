#include "avalon_addr.h"
#include "display.h"
#include "etch_a_sketch.h"
#include "peripherals.h"

// Pressing either dial's click button clears the screen.
#define CLEAR_BUTTONS (BUTTONS_MASK_DIALL_CLICK | BUTTONS_MASK_DIALR_CLICK)

void etch_a_sketch(void)
{
	// The cursor starts in the middle of the panel. The left dial moves it
	// horizontally, the right dial vertically, one pixel per detent.
	int x = DISPLAY_WIDTH / 2;
	int y = DISPLAY_HEIGHT / 2;

	// Each RotaryCtl2 exposes a free-running 8-bit position counter; we only
	// care about the direction it moved since the last time we looked.
	int last_left = 10;
	int last_right = 10;

	vid_set_pixel(x, y, PIXEL_WHITE);

	while (1) {
		if (avalon_read(PIO_BUTTONS) & CLEAR_BUTTONS) {
			clear_screen();
			continue;
		}

		int left = avalon_read(PIO_ROTARY_L);
		int right = avalon_read(PIO_ROTARY_R);

		if (left > last_left)
			x++;
		else if (left < last_left)
			x--;

		if (right > last_right)
			y++;
		else if (right < last_right)
			y--;

		// clamp the cursor to the panel so it parks at the edge
		if (x < 0)
			x = 0;
		else if (x > DISPLAY_WIDTH - 1)
			x = DISPLAY_WIDTH - 1;

		if (y < 0)
			y = 0;
		else if (y > DISPLAY_HEIGHT - 1)
			y = DISPLAY_HEIGHT - 1;

		last_left = left;
		last_right = right;

		vid_set_pixel(x, y, PIXEL_WHITE);
	}
}
