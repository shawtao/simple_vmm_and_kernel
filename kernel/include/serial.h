#ifndef _SERIAL_H
#define _SERIAL_H

#include <stddef.h>
#include <stdint.h>

#define SERIAL_PORT     0x3f8
#define DUMP_REGISTER   0x207
#define SET_LONGMODE    0x208    

uint8_t scanf(uint16_t port);
void outb(uint16_t port, uint8_t value);
void printf(const char *s);
void dump_register();
void set_longmode();

#endif