#include "main_header.h"

#define PIXEL16(r,g,b) (((r & 0x1F)<<11) | ((g & 0x3F)<<5) | ((b & 0x1F)<<0))
#define PIXEL24(r,g,b) PIXEL16((r>>3), (g>>2), (b>>3))

#define PIXEL_WHITE PIXEL24(0xFF, 0xFF, 0xFF)
int myfunction(int x, int y)
{
	return x+y;
}

int main(void)
{
	// declare some variables
	//etch_a_sketch();
	//vid_set_pixel(10,10,PIXEL_WHITE);
	//clean_screen();
	etch_a_sketch();
	return 1;
}
