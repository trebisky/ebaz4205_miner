# Makefile
#
# blink2 - download a blink bitstream into the PL and run it
#
# This builds on blink1.  We designed an AXI gpio based fancy
# LED control thing in the FPGA, now we need an axi gpio driver
# and some code to put it to use.
#
# Tom Trebisky  6-7-2022

#CROSS = arm-linux-gnu-
CROSS = arm-none-eabi-

#ABI = -isystem /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION)/include
#LIBS = -L /usr/lib/gcc/arm-linux-gnueabi/$(GCCVERSION) -lgcc

ABI = -isystem /usr/lib/gcc/arm-none-eabi/$(GCCVERSION)/include
LIBS = -L /usr/lib/gcc/arm-none-eabi/$(GCCVERSION) -lgcc


#CFLAGS          := -g -Wall
CFLAGS          := -ffreestanding -fno-builtin
CFLAGS          += -march=armv7-a
CFLAGS		+= -Wno-implicit-function-declaration
CFLAGS          += -I.
CFLAGS          += $(ABI)

CC = $(CROSS)gcc $(CFLAGS)
LD = $(CROSS)ld

GCCVERSION = `$(CROSS)gcc -dumpversion`

#ARM_ELF_FLAGS = -Os -marm -fpic -Wall
#ARM_ELF_FLAGS = -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -fpic -Wall
#ARM_ELF_FLAGS = -O2 -marm -Wall

# Used with arm-linux-gnu
ARM_ELF_FLAGS = -O2 -marm
ARM_ELF_FLAGS += -fno-common -fno-builtin -ffreestanding -nostdinc -fno-strict-aliasing
ARM_ELF_FLAGS += -mno-thumb-interwork -fno-stack-protector -fno-toplevel-reorder
ARM_ELF_FLAGS += -Wstrict-prototypes -Wno-format-nonliteral -Wno-format-security
ARM_ELF_FLAGS += -Wno-implicit-function-declaration
ARM_ELF_FLAGS += -Wno-unused-function $(ABI)

# OBJS = start.o main.o prf.o serial.o gpio.o timer.o bbb_irq.o show_regs.o gic.o
OBJS = start.o main.o prf.o uart.o timer.o arm_irq.o arm_regs.o gic.o cache.o zpl.o pl.o pl_data.o axi_gpio.o

TARGET = blink2.bin

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

#	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c $<
%.o: %.c
	$(CC) -c -nostdlib -c $<

.S.o:
	        $(CC) -o $@ -c $<

all: install

#	$(CC) -c $(ARM_ELF_FLAGS) -nostdlib -c pl_data.c
pl_data.o:	ledio.bit
	./bitread -c ledio.bit >pl_data.c
	$(CC) -c -nostdlib -c pl_data.c

install: $(TARGET)
	cp $(TARGET) /var/lib/tftpboot/bitcoin.bin

ledio.bit:
	cp /home/tom/vivado/ebaz_blink_4/ebaz_blink_4.runs/impl_1/ledio_wrapper.bit ledio.bit

# We make a copy, just to have it here alongside the code that uses it.
# For reference.  Vivado would be needed to install it into a bitstream.
verilog:
	cp /home/tom/vivado/ebaz_blink_4/ebaz_blink_4.srcs/sources_1/new/fancy.v .

pl:
	openFPGALoader -c xvc-client --file-type bit ledio.bit

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

dump:       blink2.elf
	$(DUMP) blink2.elf >blink2.dump

clean:
	rm -rf *.bin *.elf *.o *.dump
	rm -rf pl_data.c

obj:
	$(CROSS)objdump -h main.o

version:
	$(CC) --version

#LINKER = -Wl,-N,--build-id=none
#LINKER = -Wl,--build-id=none

LDFLAGS = -nostdlib -z noexecstack --no-warn-rwx-segments

#	$(CC) -o $@ -nostdlib $(OBJS) -T basic.lds $(LINKER) $(LIBS)
blink2.elf: $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) -T basic.lds $(LIBS)

# THE END
