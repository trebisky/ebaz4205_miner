# Makefile for EBAZ4205 bare metal project
# Tom Trebisky  11-8-2018
# tom@mmto.org

BOARD = ebaz
#CROSS_COMPILE = arm-linux-gnu-
CROSS_COMPILE = arm-none-eabi-

# -------------------------------------

OBJS = start.o fish.o

TARGET = $(BOARD).bin

# CFLAGS		:=	-g -Wall -Wextra -ffreestanding -fno-builtin -mlittle-endian
#CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin -mlittle-endian
CFLAGS		:=	-g -Wall -ffreestanding -fno-builtin
CFLAGS		+= -march=armv7-a
CFLAGS		+= -I.

LDFLAGS		:=	-T spl.lds \
			-nostdlib \
			-z noexecstack --no-warn-rwx-segments

CC			=	$(CROSS_COMPILE)gcc $(CFLAGS)
#LD 			=	$(CROSS_COMPILE)gcc $(LDFLAGS)
LD 			=	$(CROSS_COMPILE)ld $(LDFLAGS)
OBJCOPY			=	$(CROSS_COMPILE)objcopy
DUMP			=	$(CROSS_COMPILE)objdump

# This gives us dependencies in .d files.
# CFLAGS		+= -MMD
# This gives us a map file.
# CFLAGS		+= -Wl,-Map=$(BOARD).map,--cref \

.c.o:
	@echo " [CC]   $<"
	@$(CC) $< -c -o $@

.S.o:
	@echo " [CC]   $<"
	@$(CC) $< -c -o $@

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
	@#echo " [IMG]  $(TARGET)
	@$(OBJCOPY) -O binary $(BOARD).elf $(TARGET)

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
