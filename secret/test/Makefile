# makefile
#
# "lots" demo for the Zynq.
# This is intended to run in lieu of the FSBL
# It is as simple as possible to test the secret uart loader
#
# Eventually, when the loader is working, I want to
# investigate the SWAP bit in the OCM configuration register.
#
# Tom Trebisky  5-27-2024
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

OBJS = start.o main.o

TARGET = lots.bin

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

%.o: %.c
	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c $<

.S.o:
	        $(CC) -o $@ -c $<

# ==============================================================
all: $(TARGET)
#all: install

#install: $(TARGET)
#	cp $(TARGET) /var/lib/tftpboot/bitcoin.bin

DUMP = arm-linux-gnu-objdump -d

dump:       lots.elf
	$(DUMP) lots.elf >lots.dump

clean:
	rm -rf *.bin *.elf *.o *.dump

obj:
	$(CROSS)objdump -h main.o

version:
	$(CC) --version

#LINKER = -Wl,-N,--build-id=none
LINKER = -Wl,--build-id=none -Wl,--no-warn-rwx-segments

lots.elf: $(OBJS)
	$(CC) -o $@ -nostdlib $(OBJS) -T fsbl.lds $(LINKER) $(LIBS)
