# Makefile
#
# blink1 - download a blink bitstream into the PL and run it
#
# Tom Trebisky  6-3-2022

CROSS = arm-linux-gnu-

CC = $(CROSS)gcc

GCCVERSION = `$(CC) -dumpversion`

ABI = -isystem /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION)/include
LIBS = -L /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION) -lgcc

#ARM_ELF_FLAGS = -Os -marm -fpic -Wall
#ARM_ELF_FLAGS = -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -Wall
ARM_ELF_FLAGS = -O2 -marm

ARM_ELF_FLAGS += -fno-common -fno-builtin -ffreestanding -nostdinc -fno-strict-aliasing
ARM_ELF_FLAGS += -mno-thumb-interwork -fno-stack-protector -fno-toplevel-reorder
ARM_ELF_FLAGS += -Wstrict-prototypes -Wno-format-nonliteral -Wno-format-security
ARM_ELF_FLAGS += -Wno-implicit-function-declaration
ARM_ELF_FLAGS += -Wno-unused-function $(ABI)


# OBJS = start.o main.o prf.o serial.o gpio.o timer.o bbb_irq.o show_regs.o gic.o
OBJS = start.o main.o prf.o uart.o timer.o arm_irq.o arm_regs.o gic.o devcfg.o pl_data.o

TARGET = blink1.bin

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

%.o: %.c
	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c $<

.S.o:
	        $(CC) -o $@ -c $<

all: install

pl_data.o:	blink.bit
	bitread -c blink.bit >pl_data.c
	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c pl_data.c

install: $(TARGET)
	cp $(TARGET) /var/lib/tftpboot/bitcoin.bin

blink.bit:
	cp /home/tom/vivado/ebaz_blink_2/ebaz_blink_2.runs/impl_1/clocks_wrapper.bit blink.bit


DUMP = arm-linux-gnu-objdump -d

dump:       blink1.elf
	$(DUMP) blink1.elf >blink1.dump

clean:
	rm -rf *.bin *.elf *.o *.dump
	rm -rf pl_data.c

obj:
	$(CROSS)objdump -h main.o

version:
	$(CC) --version

#LINKER = -Wl,-N,--build-id=none
LINKER = -Wl,--build-id=none

blink1.elf: $(OBJS)
	$(CC) -o $@ -nostdlib $(OBJS) -T basic.lds $(LINKER) $(LIBS)