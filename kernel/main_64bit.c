#include <stddef.h>
#include <stdint.h>

#include "include/serial.h"

#define NUM(a) a - 48

void
__attribute__((noreturn))
__attribute__((section(".text")))
main(void) {

	uint8_t choice;
	printf("Welcome to Vmm_Kernel_64bit\n");

	for (;;) {
	
		printf("\nMain menu:\n===========\n");
		printf("1. Dump Guest Register\n2. Halt VM\n3. Test Code\n");
		printf("Your choice:\n");

		choice = scanf(SERIAL_PORT);

		switch (NUM(choice))
		{
		case 1:
			dump_register();
			break;
		case 2:
			printf("\nHalting VM\n");
			asm("hlt" : /* empty */ : "a" (42) : "memory");
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