# Makefile
#
# This began as the EBAZ4205 "timer" project
#
# Tom Trebisky  1-13-2021

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

OBJS = start.o main.o prf.o uart.o timer.o arm_irq.o arm_regs.o gic.o cores.o

TARGET = amp

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

%.o: %.c
	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c $<

.S.o:
	        $(CC) -o $@ -c $<

all: $(TARGET).bin $(TARGET).dump install

install: $(TARGET).bin
	cp $(TARGET).bin /var/lib/tftpboot/bitcoin.bin

DUMP = arm-linux-gnu-objdump -d

dump:	$(TARGET).dump

dump2:
	$(DUMP) prf.o >extra1.dump
	$(DUMP) hack.out >extra.dump


$(TARGET).dump:       $(TARGET).elf
	$(DUMP) $(TARGET).elf >$(TARGET).dump

clean:
	rm -rf *.bin *.elf *.o *.dump

obj:
	$(CROSS)objdump -h main.o

version:
	$(CC) --version

#LINKER = -Wl,-N,--build-id=none
LINKER = -Wl,--build-id=none

$(TARGET).elf: $(OBJS)
	$(CC) -o $@ -nostdlib $(OBJS) -T basic.lds $(LINKER) $(LIBS)

hack:	hack.c
	cc -o hack hack.c
