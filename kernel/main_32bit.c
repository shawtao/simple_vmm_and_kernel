#include <stddef.h>
#include <stdint.h>

#include "include/serial.h"

#define NUM(a) a - 48

extern void print_cpuid();
extern void print_cpu_brand_string();

void print_cpu_details(void)
{
	printf("\nHere is your CPU information:\n");
	printf("Vendor ID:\t");
	print_cpuid();
	printf("\nBrand string:\t");
	print_cpu_brand_string();
	printf("\n");
}

void
__attribute__((noreturn))
__attribute__((section(".text")))
main(void) {
	//const char *p;

	/*for (p = "Hello, world!\n"; *p; ++p)
		outb(0xE9, *p); */

	uint8_t choice;
	printf("Welcome to Vmm_Kernel_32bit\n");

	for (;;) {
	
		printf("\nMain menu:\n===========\n");
		printf("1. CPU Info\n2. Dump Guest Register\n3. Halt VM\n4. Enter 64bit kernel\n");
		printf("Your choice:\n");

		choice = scanf(SERIAL_PORT);

		switch (NUM(choice))
		{
		case 1:
			print_cpu_details();
			break;
		case 2:
			dump_register();
			break;
		case 3:
			printf("\nHalting VM\n");
			asm("hlt" : /* empty */ : "a" (42) : "memory");
			break;
		case 4:
			set_longmode();
			break;
	
		default:
			printf("\nUnknown choice!\n");		
			break;	
		}
		printf("\nPress any keys to continue...\n");
		scanf(SERIAL_PORT);
	}

	//*(long *) 0x400 = 42;

	for (;;)
		asm("hlt" : /* empty */ : "a" (42) : "memory");
}
