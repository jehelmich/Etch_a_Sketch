#ifndef PERIPHERALS_H
#define PERIPHERALS_H

// Raw 32-bit accesses to Avalon-MM slaves. Addresses come from avalon_addr.h.
int  avalon_read(unsigned int address);
void avalon_write(unsigned int address, int data);

#endif // PERIPHERALS_H
