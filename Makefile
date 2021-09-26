.PHONY: all clean

all: kernel
	@gcc main.c serial.c vmm.c setmode.c build/obj/payload.o -o build/vmm -lpthread
#	@nasm monitor.asm -f bin -o build/monitor

mkdir:
	@mkdir build; mkdir build/obj

kernel: mkdir
	make -C kernel


run: all
	@build/vmm

clean:
	rm -rf build
