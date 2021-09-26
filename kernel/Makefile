ASM = nasm
CC	= gcc
LD  = ld
OUTPUT_DIR = ../build/obj

ASMBFLAGS	= -f elf -w-orphan-labels
CFLAGS		= -c -Os -Wall -Wshadow -W -Wconversion -Wno-sign-conversion  -fno-stack-protector -fomit-frame-pointer -fno-builtin -fno-common  -ffreestanding  -Wno-unused-parameter -Wunused-variable

MAIN32_OBJS := main_32bit.o entry.o serial32.o cpuid.o
MAIN64_OBJS := main_64bit.o serial64.o
32BIT_SOURCE := main_32bit.c serial.C
64BIT_SOURCE := main_64bit.c serial.c

all: payload.o main64.img


payload.o: main32.img.o
	$(LD) -T payload.ld ../build/obj/$< -o ../build/obj/$@

main32.img: $(MAIN32_OBJS)
	@cp guest32.ld $(OUTPUT_DIR)
	@cd $(OUTPUT_DIR) && $(LD) -T guest32.ld -m elf_i386 $(MAIN32_OBJS) -o $@

main64.img: $(MAIN64_OBJS)
	@cp guest64.ld $(OUTPUT_DIR)
	@cd $(OUTPUT_DIR) && $(LD) -T guest64.ld  $(MAIN64_OBJS) -o $@

%.img.o: %.img
	@cd $(OUTPUT_DIR) && $(LD) -b binary -r $^ -o $@

%.o : %.asm $(OUTPUT_DIR)
	@echo "compling asm..."
	$(ASM) $(ASMBFLAGS) -o $(OUTPUT_DIR)/$@ $<

#%.32.o : $(32BIT_SOURCE) $(OUTPUT_DIR)
#	@echo "compling 32bit c source..."
#	$(CC) $(CFLAGS) -m32 -o $(OUTPUT_DIR)/$@ $<

#%.64.o : $(64BIT_SOURCE) $(OUTPUT_DIR)
#	@echo "compling 64bit c source..."
#	$(CC) $(CFLAGS) -m64 -o $(OUTPUT_DIR)/$@ $<

main_32bit.o: main_32bit.c
	$(CC) $(CFLAGS) -m32 -o $(OUTPUT_DIR)/$@ $<
serial32.o: serial.c
	$(CC) $(CFLAGS) -m32 -o $(OUTPUT_DIR)/$@ $<

main_64bit.o:main_64bit.c
	$(CC) $(CFLAGS) -m64 -o $(OUTPUT_DIR)/$@ $<
serial64.o: serial.c
	$(CC) $(CFLAGS) -m64 -o $(OUTPUT_DIR)/$@ $<
