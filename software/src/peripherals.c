// this file contains the locations of the registers as described in the table above
	// copy it into your example-c/src folder from the clarvi distribution
	#include "avalon_addr.h"

	int avalon_read(unsigned int address)
	{
		volatile int *pointer = (volatile int *) address;
		return pointer[0];
	}

	void avalon_write(unsigned int address, int data)
	{
		volatile int *pointer = (volatile int *) address;
		pointer[0] = data;
	}