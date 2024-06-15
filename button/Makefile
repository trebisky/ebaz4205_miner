# Makefile for EBAZ4205 bare metal project
# Tom Trebisky  11-8-2018
# tom@mmto.org

BOARD = button
#CROSS_COMPILE = aarch64-linux-gnu-
#CROSS_COMPILE = arm-linux-gnu-
CROSS_COMPILE = arm-none-eabi-

# -------------------------------------

OBJS = start.o button.o

TARGET = $(BOARD).bin

# CFLAGS		:=	-g -Wall -Wextra -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin
CFLAGS		+= -march=armv7-a
#CFLAGS		+= -mlittle-endian
#CFLAGS		+= -mtune=cortex-a53
CFLAGS		+= -I.

# We don't need any of these linker flags
# -Wl,--start-group -Wl,--end-group \
# -Wl,--build-id=none \
# -Bstatic \
# Useless: --no-warn-rwx-segments

LDFLAGS		:= \
			-Tspl.lds \
			-nostdlib \
			-z noexecstack

CC			=	$(CROSS_COMPILE)gcc $(CFLAGS)
#LD 			=	$(CROSS_COMPILE)gcc $(LDFLAGS) -Wl,--no-warn-rwx-segments
LD 			=	$(CROSS_COMPILE)ld $(LDFLAGS)
OBJCOPY			=	$(CROSS_COMPILE)objcopy
DUMP			=	$(CROSS_COMPILE)objdump

LOAD			=	tools/loader -h64

# This gives us dependencies in .d files.
# CFLAGS		+= -MMD
# This gives us a map file.
# CFLAGS		+= -Wl,-Map=$(BOARD).map,--cref \

.c.o:
	$(CC) $< -c -o $@

.S.o:
	$(CC) $< -c -o $@

# -------------------------------------

all: install
#all: $(TARGET)

help:
	$(CC) --help

install: $(TARGET)
	cp $(TARGET) /var/lib/tftpboot/bitcoin.bin


$(BOARD).elf: $(OBJS)
	$(LD) $(OBJS) -o $(BOARD).elf

$(TARGET): $(BOARD).elf
	$(OBJCOPY) -O binary $(BOARD).elf $(TARGET)

dis: $(BOARD).elf
	$(DUMP) -d $(BOARD).elf -z >$(BOARD).dis

.PHONY: clean
clean:
	rm -f *.o
	rm -f *.img
	rm -f *.elf
	rm -f *.bin
	rm -f *.map
	rm -f *.dis
#	rm -f *.d

# THE END
