
#import "etch_a_sketch_header.h"
#import "avalon_addr.h"

// our pixel format in memory is 5 bits of red, 6 bits of green, 5 bits of blue
	#define PIXEL16(r,g,b) (((r & 0x1F)<<11) | ((g & 0x3F)<<5) | ((b & 0x1F)<<0))
	// ... but for ease of programming we refer to colours in 8/8/8 format and discard the lower bits
	#define PIXEL24(r,g,b) PIXEL16((r>>3), (g>>2), (b>>3))

	#define PIXEL_WHITE PIXEL24(0xFF, 0xFF, 0xFF)
	#define PIXEL_BLACK PIXEL24(0x00, 0x00, 0x00)
	#define PIXEL_RED   PIXEL24(0xFF, 0x00, 0x00)
	#define PIXEL_GREEN PIXEL24(0x00, 0xFF, 0x00)
	#define PIXEL_BLUE  PIXEL24(0x00, 0x00, 0xFF)

	#define DISPLAY_WIDTH	480
	#define DISPLAY_HEIGHT	272

	void vid_set_pixel(int x, int y, int colour)
	{
		// derive a pointer to the framebuffer described as 16 bit integers
		volatile short *framebuffer = (volatile short *) (FRAMEBUFFER_BASE);

		// make sure we don't go past the edge of the screen
		if ((x<0) || (x>DISPLAY_WIDTH-1))
			return;
		if ((y<0) || (y>DISPLAY_HEIGHT-1))
			return;

		framebuffer[x+y*DISPLAY_WIDTH] = colour;
	}

	void clean_screen() {
		int a = 0;
        int b = 0;
        while (a < DISPLAY_WIDTH) {
            while (b < DISPLAY_HEIGHT) {
                vid_set_pixel(a,b,PIXEL_BLACK);
                b = b + 1;
            }
            b = 0;
            a = a + 1;
        }	
	}
	
	
void etch_a_sketch() {
	int leftPos = DISPLAY_WIDTH/2;
    int rightPos = DISPLAY_HEIGHT/2;
	int lastLeft = 10;
    int lastRight = 10;
	vid_set_pixel(leftPos,rightPos,PIXEL_WHITE);
	
	while (1) {
		int buttons = avalon_read(PIO_BUTTONS);
		if ((buttons & 6) > 0) {

            // Loop through all the possible pixels
            clean_screen();


        } else {
			int left = avalon_read(PIO_ROTARY_L);
			int right = avalon_read(PIO_ROTARY_R);
            
			if (left != lastLeft) {
				if (left > lastLeft) {
					leftPos = leftPos + 1;
				} else if (left < lastLeft) {
					leftPos = leftPos - 1;
				}
				if (leftPos < 0) {
					leftPos = 0;
				} else if (leftPos > DISPLAY_WIDTH-1) {
					leftPos = DISPLAY_WIDTH-1;
				}
			}
			
			if (right != lastRight) {
				if (right > lastRight) {
					rightPos = rightPos + 1;
				} else if (right < lastRight) {
					rightPos = rightPos - 1;
				}
				if (rightPos < 0) {
					rightPos = 0;
				} else if (rightPos > DISPLAY_HEIGHT-1) {
					rightPos = DISPLAY_HEIGHT-1;
				}
			}

			lastLeft = left;
			lastRight = right;
			
            vid_set_pixel(leftPos,rightPos,PIXEL_WHITE);
        }
		
	}
}

