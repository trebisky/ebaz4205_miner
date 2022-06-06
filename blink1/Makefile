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
OBJS = start.o main.o prf.o uart.o timer.o arm_irq.o arm_regs.o gic.o cache.o zpl.o pl.o pl_data.o

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

bits:
	cp blink.bin /var/lib/tftpboot/blink.bin

blink.bin:
	bitread -e blink.bit

blink.bit:
	cp /home/tom/vivado/ebaz_blink_2/ebaz_blink_2.runs/impl_1/clocks_wrapper.bit blink.bit

pl:
	openFPGALoader -c xvc-client --file-type bit blink.bit

# -----------------------------

# as an experiment, pull in a file from the u-boot sources,
# then a subset of files needed to make it compile and hack away
# for a while.
# WATCH OUT -- beware invoking this a second time and having
# the copy overwrite any changes in your copy.
# Renaming files (but not here) would be good insurance.
# And of course commenting out the line after it has been run.
# This is just a way of keeping a record of where files came from.

getu:
	##cp /u1/Projects/FPGA/u-boot-xlnx/drivers/fpga/zynqpl.c .
	# rename the above to zpl.c
	#cp /u1/Projects/FPGA/u-boot-xlnx/arch/arm/mach-zynq/include/mach/hardware.h .
	##cp /u1/Projects/FPGA/u-boot-xlnx/include/fpga.h .
	#cp /u1/Projects/FPGA/u-boot-xlnx/include/xilinx.h .
	#cp /u1/Projects/FPGA/u-boot-xlnx/arch/arm/mach-zynq/slcr.c .

# -----------------------------
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
