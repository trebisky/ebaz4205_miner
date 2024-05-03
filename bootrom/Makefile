# Zynq XC7Z010-CLG400A bootrom project

#DUMP = arm-linux-gnu-objdump
DUMP = arm-none-eabi-objdump
MACH = -marm

ROM = zynq_bootrom.bin

all:	bootrom.dis

bootrom.elf: wrap
	./wrap $(ROM) bootrom.elf

bootrom.raw: bootrom.elf
	$(DUMP) $(MACH) -d bootrom.elf -z >bootrom.raw

bootrom.dis: bootrom.raw
	./fixaddr >bootrom.dis

bootrom.odx: $(ROM)
	odx $(ROM) >bootrom.odx

# should yield: md5: ADF639AFE9855EE86C8FAAD216C970D9
sum:
	md5sum $(ROM)

wrap:   wrap.c
	cc -o wrap wrap.c

clean:
	rm -f bootrom.elf
	rm -f bootrom.raw
	rm -f bootrom.dis
	rm -f wrap

# THE END
