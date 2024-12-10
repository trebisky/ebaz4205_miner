# makefile
#
# This is the old Orange Pi interrupts_kyu project
# hacked into shape for the EBAZ4205
#
# Tom Trebisky  1-13-2021

# Changed 5-27-2024, then reverted
#CROSS = arm-linux-gnu-
CROSS = arm-none-eabi-

CC = $(CROSS)gcc

#ABI = -isystem /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION)/include
#LIBS = -L /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION) -lgcc

ABI = -isystem /usr/lib/gcc/arm-none-eabi/$(GCCVERSION)/include
LIBS = -L /usr/lib/gcc/arm-none-eabi/$(GCCVERSION) -lgcc

#CFLAGS          := -g -Wall
CFLAGS          := -ffreestanding -fno-builtin
CFLAGS          += -march=armv7-a
CFLAGS          += -Wno-implicit-function-declaration
CFLAGS          += -I.
CFLAGS          += $(ABI)

CC = $(CROSS)gcc $(CFLAGS)
LD = $(CROSS)ld

GCCVERSION = `$(CROSS)gcc -dumpversion`

# Used with arm-linux-gn
#ARM_ELF_FLAGS = -Os -marm -fpic -Wall
#ARM_ELF_FLAGS = -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -Wall
ARM_ELF_FLAGS = -O2 -marm

# Used with arm-linux-gn
ARM_ELF_FLAGS += -fno-common -fno-builtin -ffreestanding -nostdinc -fno-strict-aliasing
ARM_ELF_FLAGS += -mno-thumb-interwork -fno-stack-protector -fno-toplevel-reorder
ARM_ELF_FLAGS += -Wstrict-prototypes -Wno-format-nonliteral -Wno-format-security
ARM_ELF_FLAGS += -Wno-implicit-function-declaration
ARM_ELF_FLAGS += -Wno-unused-function $(ABI)

#OBJS = start.o main.o prf.o serial.o gpio.o timer.o bbb_irq.o show_regs.o gic.o
OBJS = start.o main.o prf.o uart.o timer.o arm_irq.o arm_regs.o gic.o

TARGET = interrupts.bin

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

%.o: %.c
	$(CC) -c -nostdlib -c $<

.S.o:
	        $(CC) -o $@ -c $<


all: install

install: $(TARGET)
	cp $(TARGET) /var/lib/tftpboot/bitcoin.bin

DUMP = arm-linux-gnu-objdump -d

dump:       interrupts.elf
	$(DUMP) interrupts.elf >interrupts.dump

clean:
	rm -rf *.bin *.elf *.o *.dump

obj:
	$(CROSS)objdump -h main.o

version:
	$(CC) --version

#LINKER = -Wl,-N,--build-id=none
#LINKER = -Wl,--build-id=none -Wl,--no-warn-rwx-segments

LDFLAGS = -nostdlib -z noexecstack --no-warn-rwx-segments

#	$(LD) -o $@ -nostdlib $(OBJS) -T basic.lds $(LINKER) $(LIBS)
interrupts.elf: $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) -T basic.lds $(LIBS)

# THE END
