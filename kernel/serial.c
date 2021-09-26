#include "include/serial.h"

uint8_t scanf(uint16_t port) {
	unsigned char v;
	asm("inb %w1,%0" : "=a" (v): "Nd" (port));
	return v;
}

void outb(uint16_t port, uint8_t value) {
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

void printf(const char *s)
{
	for ( ; *s; ++s)
		outb(SERIAL_PORT, *s);
}

void dump_register() {
	uint8_t value = ' ';
	uint16_t port = DUMP_REGISTER;
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}

void set_longmode() {
	uint8_t value = ' ';
	uint16_t port = SET_LONGMODE;
	asm("outb %0,%1" : /* empty */ : "a" (value), "Nd" (port) : "memory");
}