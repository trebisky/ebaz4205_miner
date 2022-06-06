// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012-2013, Xilinx, Michal Simek
 *
 * (C) Copyright 2012
 * Joe Hershberger <joe.hershberger@ni.com>
 */

/* TJT says: I try to put all my hacks on this.
 */
#define TJT

#ifndef TJT
#include <common.h>
#include <console.h>
#include <cpu_func.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <fs.h>
#include <zynqpl.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#endif

#ifdef TJT
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int __u32;

typedef unsigned int size_t;

typedef unsigned long ulong;

#define NULL	(0)

#define mdelay(x)	ms_delay(x)

// From include/linux/kernel.h
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK((x),(typeof(x))(a)-1)

/* The `const' in roundup() prevents gcc-3.3 from calling __divdi3 */
#define roundup(x, y) (                                 \
{                                                       \
        const typeof(y) __y = y;                        \
        (((x) + (__y - 1)) / __y) * __y;                \
}                                                       \
)

/* In ms units
 */
#define CONFIG_SYS_FPGA_WAIT		100
#define CONFIG_SYS_FPGA_PROG_TIME	4000


// from: ./include/linux/sizes.h
// #define SZ_1M	(1024*1024)
#define SZ_1M				0x00100000

// See: arch/arm/include/asm/cache.h
#define CONFIG_SYS_CACHELINE_SIZE 64
#define ARCH_DMA_MINALIGN       CONFIG_SYS_CACHELINE_SIZE

#include "hardware.h"
#include "fpga.h"
// #include "xilinx.h"

// "debug" is defined in include/log.h

#define DEBUG

#ifdef DEBUG
#define _DEBUG  1
#else
#define _DEBUG  0
#endif

/* Define this at the top of a file to add a prefix to debug messages */
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define debug_cond(cond, fmt, args...)          \
({                                              \
        if (cond)                               \
                printf(pr_fmt(fmt), ##args);    \
})

/* Show a message if DEBUG is defined in a file */
#define debug(fmt, args...)                     \
        debug_cond(_DEBUG, fmt, ##args)


// From arch/arm/include/asm/io.h
// From arch/arm/include/asm/barriers.h
// From include/compiler.h

#define __raw_writel(v,a)       __arch_putl(v,a)
#define __raw_readl(a)          __arch_getl(a)

#define out_32(a,v)     __raw_writel(v,a)
#define in_32(a)        __raw_readl(a)

# define le32_to_cpu(x)         (x)
# define cpu_to_le32(x)         (x)

#define out_arch(type,endian,a,v)       __raw_write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)          endian##_to_cpu(__raw_read##type(a))

#define out_le32(a,v)   out_arch(l,le32,a,v)
#define in_le32(a)      in_arch(l,le32,a)

#define clrbits(type, addr, clear) \
        out_##type((addr), in_##type(addr) & ~(clear))


#define clrbits_le32(addr, clear) clrbits(le32, addr, clear)


#ifdef notdef
/* For ARM v6 */
#define CP15DMB asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))
#define CP15DSB asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))
#define CP15ISB asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
#define DMB     CP15DMB
#define DSB     CP15DSB
#define ISB     CP15ISB
#endif

#define DMB     asm volatile ("dmb sy" : : : "memory")
#define DSB     asm volatile ("dsb sy" : : : "memory")
#define ISB     asm volatile ("isb sy" : : : "memory")

#define dmb()   DMB
#define dsb()   DSB
#define isb()   ISB


#define __iormb()       dmb()
#define __iowmb()       dmb()

#define __arch_getb(a)                  (*(volatile unsigned char *)(a))
#define __arch_getw(a)                  (*(volatile unsigned short *)(a))
#define __arch_getl(a)                  (*(volatile unsigned int *)(a))

#define __arch_putb(v,a)                (*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)                (*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)                (*(volatile unsigned int *)(a) = (v))

#define writeb(v,c)     ({ u8  __v = v; __iowmb(); __arch_putb(__v,c); __v; })
#define writew(v,c)     ({ u16 __v = v; __iowmb(); __arch_putw(__v,c); __v; })
#define writel(v,c)     ({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })


#define readb(c)        ({ u8  __v = __arch_getb(c); __iormb(); __v; })
#define readw(c)        ({ u16 __v = __arch_getw(c); __iormb(); __v; })
#define readl(c)        ({ u32 __v = __arch_getl(c); __iormb(); __v; })

// From include/linux/byteorder/swab.h

#define ___swab32(x) \
        ((__u32)( \
                (((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
                (((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
                (((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
                (((__u32)(x) & (__u32)0xff000000UL) >> 24) ))


#  define __arch__swab32(x) ___swab32(x)

static __inline__ __attribute__((const))
__u32 __fswab32(__u32 x)
{
        return __arch__swab32(x);
}

#define __swab32(x) __fswab32(x)

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
#endif /* end TJT */

#define DEVCFG_CTRL_PCFG_PROG_B		0x40000000
#define DEVCFG_CTRL_PCFG_AES_EFUSE_MASK	0x00001000
#define DEVCFG_CTRL_PCAP_RATE_EN_MASK	0x02000000
#define DEVCFG_CTRL_PCFG_AES_EN_MASK	0x00000E00
#define DEVCFG_ISR_FATAL_ERROR_MASK	0x00740040
#define DEVCFG_ISR_ERROR_FLAGS_MASK	0x00340840
#define DEVCFG_ISR_RX_FIFO_OV		0x00040000
#define DEVCFG_ISR_DMA_DONE		0x00002000
#define DEVCFG_ISR_PCFG_DONE		0x00000004
#define DEVCFG_STATUS_DMA_CMD_Q_F	0x80000000
#define DEVCFG_STATUS_DMA_CMD_Q_E	0x40000000
#define DEVCFG_STATUS_DMA_DONE_CNT_MASK	0x30000000
#define DEVCFG_STATUS_PCFG_INIT		0x00000010
#define DEVCFG_MCTRL_PCAP_LPBK		0x00000010
#define DEVCFG_MCTRL_RFIFO_FLUSH	0x00000002
#define DEVCFG_MCTRL_WFIFO_FLUSH	0x00000001

#ifndef TJT
#ifndef CONFIG_SYS_FPGA_WAIT
#define CONFIG_SYS_FPGA_WAIT CONFIG_SYS_HZ/100	/* 10 ms */
#endif

#ifndef CONFIG_SYS_FPGA_PROG_TIME
#define CONFIG_SYS_FPGA_PROG_TIME	(CONFIG_SYS_HZ * 4) /* 4 s */
#endif
#endif

#define DUMMY_WORD	0xffffffff

/* Xilinx binary format header */
static const u32 bin_format[] = {
	DUMMY_WORD, /* Dummy words */
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	DUMMY_WORD,
	0x000000bb, /* Sync word */
	0x11220044, /* Sync word */
	DUMMY_WORD,
	DUMMY_WORD,
	0xaa995566, /* Sync word */
};

#define SWAP_NO		1
#define SWAP_DONE	2

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* TJT - throw this together,
 * see drivers/timer/tsc_timer.c
 * Also see code in my main.c
 */

unsigned long
get_timer(unsigned long base)
{
        return get_ms_timer() - base;
}

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* TJT - Copy the following from slcr.c */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */

#ifndef TJT
#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#endif

#define SLCR_LOCK_MAGIC		0x767B
#define SLCR_UNLOCK_MAGIC	0xDF0D

#define SLCR_NAND_L2_SEL		0x10
#define SLCR_NAND_L2_SEL_MASK		0x1F

#define SLCR_USB_L1_SEL			0x04

#define SLCR_IDCODE_MASK	0x1F000
#define SLCR_IDCODE_SHIFT	12

/*
 * zynq_slcr_mio_get_status - Get the status of MIO peripheral.
 *
 * @peri_name: Name of the peripheral for checking MIO status
 * @get_pins: Pointer to array of get pin for this peripheral
 * @num_pins: Number of pins for this peripheral
 * @mask: Mask value
 * @check_val: Required check value to get the status of  periph
 */
struct zynq_slcr_mio_get_status {
	const char *peri_name;
	const int *get_pins;
	int num_pins;
	u32 mask;
	u32 check_val;
};

static const int nand8_pins[] = {
	0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

static const int nand16_pins[] = {
	16, 17, 18, 19, 20, 21, 22, 23
};

static const int usb0_pins[] = {
	28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39
};

static const int usb1_pins[] = {
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

static const struct zynq_slcr_mio_get_status mio_periphs[] = {
	{
		"nand8",
		nand8_pins,
		ARRAY_SIZE(nand8_pins),
		SLCR_NAND_L2_SEL_MASK,
		SLCR_NAND_L2_SEL,
	},
	{
		"nand16",
		nand16_pins,
		ARRAY_SIZE(nand16_pins),
		SLCR_NAND_L2_SEL_MASK,
		SLCR_NAND_L2_SEL,
	},
	{
		"usb0",
		usb0_pins,
		ARRAY_SIZE(usb0_pins),
		SLCR_USB_L1_SEL,
		SLCR_USB_L1_SEL,
	},
	{
		"usb1",
		usb1_pins,
		ARRAY_SIZE(usb1_pins),
		SLCR_USB_L1_SEL,
		SLCR_USB_L1_SEL,
	},
};

static int slcr_lock = 1; /* 1 means locked, 0 means unlocked */

void zynq_slcr_lock(void)
{
	if (!slcr_lock) {
		writel(SLCR_LOCK_MAGIC, &slcr_base->slcr_lock);
		slcr_lock = 1;
	}
}

void zynq_slcr_unlock(void)
{
	if (slcr_lock) {
		writel(SLCR_UNLOCK_MAGIC, &slcr_base->slcr_unlock);
		slcr_lock = 0;
	}
}

/* Reset the entire system */
void zynq_slcr_cpu_reset(void)
{
	/*
	 * Unlock the SLCR then reset the system.
	 * Note that this seems to require raw i/o
	 * functions or there's a lockup?
	 */
	zynq_slcr_unlock();

	/*
	 * Clear 0x0F000000 bits of reboot status register to workaround
	 * the FSBL not loading the bitstream after soft-reboot
	 * This is a temporary solution until we know more.
	 */
	clrbits_le32(&slcr_base->reboot_status, 0xF000000);

	writel(1, &slcr_base->pss_rst_ctrl);
}

void zynq_slcr_devcfg_disable(void)
{
	u32 reg_val;

	zynq_slcr_unlock();

	/* Disable AXI interface by asserting FPGA resets */
	writel(0xF, &slcr_base->fpga_rst_ctrl);

	/* Disable Level shifters before setting PS-PL */
	reg_val = readl(&slcr_base->lvl_shftr_en);
	reg_val &= ~0xF;
	writel(reg_val, &slcr_base->lvl_shftr_en);

	/* Set Level Shifters DT618760 */
	writel(0xA, &slcr_base->lvl_shftr_en);

	zynq_slcr_lock();
}

void zynq_slcr_devcfg_enable(void)
{
	zynq_slcr_unlock();

	/* Set Level Shifters DT618760 */
	writel(0xF, &slcr_base->lvl_shftr_en);

	/* Enable AXI interface by de-asserting FPGA resets */
	writel(0x0, &slcr_base->fpga_rst_ctrl);

	zynq_slcr_lock();
}

u32 zynq_slcr_get_boot_mode(void)
{
	/* Get the bootmode register value */
	return readl(&slcr_base->boot_mode);
}

u32 zynq_slcr_get_idcode(void)
{
	return (readl(&slcr_base->pss_idcode) & SLCR_IDCODE_MASK) >>
							SLCR_IDCODE_SHIFT;
}

#ifndef TJT
/*
 * zynq_slcr_get_mio_pin_status - Get the MIO pin status of peripheral.
 *
 * @periph: Name of the peripheral
 *
 * Returns count to indicate the number of pins configured for the
 * given @periph.
 */
int zynq_slcr_get_mio_pin_status(const char *periph)
{
	const struct zynq_slcr_mio_get_status *mio_ptr;
	int val, j;
	int mio = 0;
	u32 i;

	for (i = 0; i < ARRAY_SIZE(mio_periphs); i++) {
		if (strcmp(periph, mio_periphs[i].peri_name) == 0) {
			mio_ptr = &mio_periphs[i];
			for (j = 0; j < mio_ptr->num_pins; j++) {
				val = readl(&slcr_base->mio_pin
						[mio_ptr->get_pins[j]]);
				if ((val & mio_ptr->mask) == mio_ptr->check_val)
					mio++;
			}
			break;
		}
	}

	return mio;
}
#endif

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* TJT - End of the copy from slcr.c */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* TJT - Now for ARM v7 cache flushing */
/* From arch/arm/cpu/armv7/cache_v7.c */
/* From arch/arm/lib/cache.c */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */

/* From arch/arm/include/asm/armv7.h */
#define CCSIDR_LINE_SIZE_OFFSET         0
#define CCSIDR_LINE_SIZE_MASK           0x7

static u32 get_ccsidr(void)
{
        u32 ccsidr;

        /* Read current CP15 Cache Size ID Register */
        asm volatile ("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));
        return ccsidr;
}

static void v7_dcache_clean_inval_range(u32 start, u32 stop, u32 line_len)
{
        u32 mva;

        /* Align start to cache line boundary */
        start &= ~(line_len - 1);
        for (mva = start; mva < stop; mva = mva + line_len) {
                /* DCCIMVAC - Clean & Invalidate data cache by MVA to PoC */
                asm volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
        }
}

static void v7_dcache_inval_range(u32 start, u32 stop, u32 line_len)
{
        u32 mva;

        if (!check_cache_range(start, stop))
                return;

        for (mva = start; mva < stop; mva = mva + line_len) {
                /* DCIMVAC - Invalidate data cache by MVA to PoC */
                asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
        }
}

int check_cache_range(unsigned long start, unsigned long stop)
{
        int ok = 1;

        if (start & (CONFIG_SYS_CACHELINE_SIZE - 1))
                ok = 0;

        if (stop & (CONFIG_SYS_CACHELINE_SIZE - 1))
                ok = 0;

        if (!ok) {
                // warn_non_spl("CACHE: Misaligned operation at range [%08lx, %08lx]\n", start, stop);
                printf("CACHE: Misaligned operation at range [%X, %X]\n", start, stop);
        }

        return ok;
}

#define ARMV7_DCACHE_INVAL_RANGE        1
#define ARMV7_DCACHE_CLEAN_INVAL_RANGE  2

static void v7_dcache_maint_range(u32 start, u32 stop, u32 range_op)
{
        u32 line_len, ccsidr;

        ccsidr = get_ccsidr();
        line_len = ((ccsidr & CCSIDR_LINE_SIZE_MASK) >>
                        CCSIDR_LINE_SIZE_OFFSET) + 2;
        /* Converting from words to bytes */
        line_len += 2;
        /* converting from log2(linelen) to linelen */
        line_len = 1 << line_len;

        switch (range_op) {
        case ARMV7_DCACHE_CLEAN_INVAL_RANGE:
                v7_dcache_clean_inval_range(start, stop, line_len);
                break;
        case ARMV7_DCACHE_INVAL_RANGE:
                v7_dcache_inval_range(start, stop, line_len);
                break;
        }

        /* DSB to make sure the operation is complete */
        dsb();
}

// __weak
void v7_outer_cache_flush_range(u32 start, u32 end) {}

/*
 * Flush range(clean & invalidate) from all levels of D-cache/unified
 * cache used:
 * Affects the range [start, stop - 1]
 */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
        check_cache_range(start, stop);

        v7_dcache_maint_range(start, stop, ARMV7_DCACHE_CLEAN_INVAL_RANGE);

        v7_outer_cache_flush_range(start, stop);
}

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */

/*
 * Load the whole word from unaligned buffer
 * Keep in your mind that it is byte loading on little-endian system
 */
static u32 
load_word(const void *buf, u32 swap)
{
	u32 word = 0;
	u8 *bitc = (u8 *)buf;
	int p;

	if (swap == SWAP_NO) {
		for (p = 0; p < 4; p++) {
			word <<= 8;
			word |= bitc[p];
		}
	} else {
		for (p = 3; p >= 0; p--) {
			word <<= 8;
			word |= bitc[p];
		}
	}

	return word;
}

static u32
check_header(const void *buf)
{
	u32 i, pattern;
	int swap = SWAP_NO;
	u32 *test = (u32 *)buf;

	debug("%s: Let's check bitstream header\n", __func__);

	/* Checking that passing bin is not a bitstream */
	for (i = 0; i < ARRAY_SIZE(bin_format); i++) {
		pattern = load_word(&test[i], swap);

		/*
		 * Bitstreams in binary format are swapped
		 * compare to regular bistream.
		 * Do not swap dummy word but if swap is done assume
		 * that parsing buffer is binary format
		 */
		if ((__swab32(pattern) != DUMMY_WORD) &&
		    (__swab32(pattern) == bin_format[i])) {
			pattern = __swab32(pattern);
			swap = SWAP_DONE;
			debug("%s: data swapped - let's swap\n", __func__);
		}

		debug("%s: %d/%x: pattern %x/%x bin_format\n", __func__, i,
		      (u32)&test[i], pattern, bin_format[i]);
		if (pattern != bin_format[i]) {
			debug("%s: Bitstream is not recognized\n", __func__);
			return 0;
		}
	}

	debug("%s: Found bitstream header at %x %s swapinng\n", __func__,
	      (u32)buf, swap == SWAP_NO ? "without" : "with");

	return swap;
}

static void *
check_data(u8 *buf, size_t bsize, u32 *swap)
{
	u32 word, p = 0; /* possition */

	/* Because buf doesn't need to be aligned let's read it by chars */
	for (p = 0; p < bsize; p++) {
		word = load_word(&buf[p], SWAP_NO);
		debug("%s: word %x %x/%x\n", __func__, word, p, (u32)&buf[p]);

		/* Find the first bitstream dummy word */
		if (word == DUMMY_WORD) {
			debug("%s: Found dummy word at position %x/%x\n",
			      __func__, p, (u32)&buf[p]);
			*swap = check_header(&buf[p]);
			if (*swap) {
				/* FIXME add full bitstream checking here */
				return &buf[p];
			}
		}
#ifndef TJT
		/* Loop can be huge - support CTRL + C */
		if (ctrlc())
			return NULL;
#endif
	}
	return NULL;
}

static int zynq_dma_transfer(u32 srcbuf, u32 srclen, u32 dstbuf, u32 dstlen)
{
	unsigned long ts;
	u32 isr_status;

	printf ( "devcfg BASE = %X\n", devcfg_base );

	/* Set up the transfer */
	writel((u32)srcbuf, &devcfg_base->dma_src_addr);
	writel(dstbuf, &devcfg_base->dma_dst_addr);
	writel(srclen, &devcfg_base->dma_src_len);
	writel(dstlen, &devcfg_base->dma_dst_len);

	isr_status = readl(&devcfg_base->int_sts);

	/* Polling the PCAP_INIT status for Set */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_DMA_DONE)) {
		if (isr_status & DEVCFG_ISR_ERROR_FLAGS_MASK) {
			debug("%s: Error: isr = 0x%08X\n", __func__,
			      isr_status);
			debug("%s: Write count = 0x%08X\n", __func__,
			      readl(&devcfg_base->write_count));
			debug("%s: Read count = 0x%08X\n", __func__,
			      readl(&devcfg_base->read_count));

			return FPGA_FAIL;
		}
		if (get_timer(ts) > CONFIG_SYS_FPGA_PROG_TIME) {
			printf("%s: Timeout wait for DMA to complete\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: DMA transfer is done\n", __func__);

	/* Clear out the DMA status */
	writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);

	return FPGA_SUCCESS;
}

static int
zynq_dma_xfer_init ( bitstream_type bstype )
{
	u32 status, control, isr_status;
	unsigned long ts;

	/* Clear loopback bit */
	clrbits_le32(&devcfg_base->mctrl, DEVCFG_MCTRL_PCAP_LPBK);

	/* Must be BIT_FULL */
	if (bstype != BIT_PARTIAL && bstype != BIT_NONE) {
		zynq_slcr_devcfg_disable();

		/* Setting PCFG_PROG_B signal to high */
		control = readl(&devcfg_base->ctrl);
		writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/*
		 * Delay is required if AES efuse is selected as
		 * key source.
		 */
		if (control & DEVCFG_CTRL_PCFG_AES_EFUSE_MASK)
			mdelay(5);

		/* Setting PCFG_PROG_B signal to low */
		writel(control & ~DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/*
		 * Delay is required if AES efuse is selected as
		 * key source.
		 */
		if (control & DEVCFG_CTRL_PCFG_AES_EFUSE_MASK)
			mdelay(5);

		/* Polling the PCAP_INIT status for Reset */
		ts = get_timer(0);
		while (readl(&devcfg_base->status) & DEVCFG_STATUS_PCFG_INIT) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for INIT to clear\n",
				       __func__);
				return FPGA_FAIL;
			}
		}

		/* Setting PCFG_PROG_B signal to high */
		writel(control | DEVCFG_CTRL_PCFG_PROG_B, &devcfg_base->ctrl);

		/* Polling the PCAP_INIT status for Set */
		ts = get_timer(0);
		while (!(readl(&devcfg_base->status) &
			DEVCFG_STATUS_PCFG_INIT)) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for INIT to set\n",
				       __func__);
				return FPGA_FAIL;
			}
		}
	}

	isr_status = readl(&devcfg_base->int_sts);

	/* Clear it all, so if Boot ROM comes back, it can proceed */
	writel(0xFFFFFFFF, &devcfg_base->int_sts);

	if (isr_status & DEVCFG_ISR_FATAL_ERROR_MASK) {
		debug("%s: Fatal errors in PCAP 0x%X\n", __func__, isr_status);

		/* If RX FIFO overflow, need to flush RX FIFO first */
		if (isr_status & DEVCFG_ISR_RX_FIFO_OV) {
			writel(DEVCFG_MCTRL_RFIFO_FLUSH, &devcfg_base->mctrl);
			writel(0xFFFFFFFF, &devcfg_base->int_sts);
		}
		return FPGA_FAIL;
	}

	status = readl(&devcfg_base->status);

	debug("%s: Status = 0x%X\n", __func__, status);

	if (status & DEVCFG_STATUS_DMA_CMD_Q_F) {
		debug("%s: Error: device busy\n", __func__);
		return FPGA_FAIL;
	}

	debug("%s: Device ready\n", __func__);

	if (!(status & DEVCFG_STATUS_DMA_CMD_Q_E)) {
		if (!(readl(&devcfg_base->int_sts) & DEVCFG_ISR_DMA_DONE)) {
			/* Error state, transfer cannot occur */
			debug("%s: ISR indicates error\n", __func__);
			return FPGA_FAIL;
		} else {
			/* Clear out the status */
			writel(DEVCFG_ISR_DMA_DONE, &devcfg_base->int_sts);
		}
	}

	if (status & DEVCFG_STATUS_DMA_DONE_CNT_MASK) {
		/* Clear the count of completed DMA transfers */
		writel(DEVCFG_STATUS_DMA_DONE_CNT_MASK, &devcfg_base->status);
	}

	return FPGA_SUCCESS;
}

static u32 *
zynq_align_dma_buffer(u32 *buf, u32 len, u32 swap)
{
	u32 *new_buf;
	u32 i;

	if ((u32)buf != ALIGN((u32)buf, ARCH_DMA_MINALIGN)) {
		new_buf = (u32 *)ALIGN((u32)buf, ARCH_DMA_MINALIGN);

		/*
		 * This might be dangerous but permits to flash if
		 * ARCH_DMA_MINALIGN is greater than header size
		 */
		if (new_buf > buf) {
			debug("%s: Aligned buffer is after buffer start\n",
			      __func__);
			new_buf = (u32 *)((u32)new_buf - ARCH_DMA_MINALIGN);
		}
		printf("%s: Align buffer at %x to %x(swap %d)\n", __func__,
		       (u32)buf, (u32)new_buf, swap);

		for (i = 0; i < (len/4); i++)
			new_buf[i] = load_word(&buf[i], swap);

		buf = new_buf;
	} else if (swap != SWAP_DONE) {
		/* For bitstream which are aligned */
		u32 *new_buf = (u32 *)buf;

		printf("%s: Bitstream is not swapped(%d) - swap it\n", __func__, swap);

		for (i = 0; i < (len/4); i++)
			new_buf[i] = load_word(&buf[i], swap);
	}

	return buf;
}

// static int zynq_validate_bitstream(xilinx_desc *desc, const void *buf,
// 				   size_t bsize, u32 blocksize, u32 *swap,
// 				   bitstream_type *bstype)
static int
zynq_validate_bitstream (const void *buf,
			       size_t bsize, u32 blocksize, u32 *swap,
			       bitstream_type *bstype)
{
	u32 *buf_start;
	u32 diff;

	buf_start = check_data((u8 *)buf, blocksize, swap);

	if (!buf_start)
		return FPGA_FAIL;

	/* Check if data is postpone from start */
	diff = (u32)buf_start - (u32)buf;
	if (diff) {
		printf("%s: Bitstream is not validated yet (diff %x)\n",
		       __func__, diff);
		return FPGA_FAIL;
	}

	if ((u32)buf < SZ_1M) {
		printf("%s: Bitstream has to be placed up to 1MB (%x)\n",
		       __func__, (u32)buf);
		return FPGA_FAIL;
	}

	if (zynq_dma_xfer_init(*bstype))
		return FPGA_FAIL;

	return 0;
}

//static int zynq_load(xilinx_desc *desc, const void *buf, size_t bsize,
//		     bitstream_type bstype)
static int
zynq_load ( const void *buf, size_t bsize, bitstream_type bstype)
{
	unsigned long ts; /* Timestamp */
	u32 isr_status, swap;

	/*
	 * send bsize inplace of blocksize as it was not a bitstream
	 * in chunks
	 */
	if (zynq_validate_bitstream ( buf, bsize, bsize, &swap,
				    &bstype))
		return FPGA_FAIL;

	buf = zynq_align_dma_buffer((u32 *)buf, bsize, swap);

	debug("%s: Source = 0x%X\n", __func__, (u32)buf);
	debug("%s: Size = %d\n", __func__, bsize);

	/* flush(clean & invalidate) d-cache range buf */
	flush_dcache_range((u32)buf, (u32)buf +
			   roundup(bsize, ARCH_DMA_MINALIGN));

	if (zynq_dma_transfer((u32)buf | 1, bsize >> 2, 0xffffffff, 0))
		return FPGA_FAIL;

	isr_status = readl(&devcfg_base->int_sts);

	/* Check FPGA configuration completion.
	 * (typically we give it 100 ms)
	 */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			printf("%s: Timeout wait for FPGA to config\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: FPGA config done\n", __func__);

	if (bstype != BIT_PARTIAL)
		zynq_slcr_devcfg_enable();

	// puts("INFO:post config was not run, please run manually if needed\n");
	printf("INFO:post config was not run, please run manually if needed\n");

	return FPGA_SUCCESS;
}

/* TJT - here are the two "public" entry points I introduced */

int
zynq_load_full ( const void *buf, size_t bsize )
{
	return zynq_load ( buf, bsize, BIT_FULL );
}

int
zynq_load_partial ( const void *buf, size_t bsize )
{
	return zynq_load ( buf, bsize, BIT_PARTIAL );
}

#if defined(CONFIG_CMD_FPGA_LOADFS) && !defined(CONFIG_SPL_BUILD)
static int zynq_loadfs(xilinx_desc *desc, const void *buf, size_t bsize,
		       fpga_fs_info *fsinfo)
{
	unsigned long ts; /* Timestamp */
	u32 isr_status, swap;
	u32 partialbit = 0;
	loff_t blocksize, actread;
	loff_t pos = 0;
	int fstype;
	char *interface, *dev_part;
	const char *filename;

	blocksize = fsinfo->blocksize;
	interface = fsinfo->interface;
	dev_part = fsinfo->dev_part;
	filename = fsinfo->filename;
	fstype = fsinfo->fstype;

	if (fs_set_blk_dev(interface, dev_part, fstype))
		return FPGA_FAIL;

	if (fs_read(filename, (u32) buf, pos, blocksize, &actread) < 0)
		return FPGA_FAIL;

	if (zynq_validate_bitstream(desc, buf, bsize, blocksize, &swap,
				    &partialbit))
		return FPGA_FAIL;

	dcache_disable();

	do {
		buf = zynq_align_dma_buffer((u32 *)buf, blocksize, swap);

		if (zynq_dma_transfer((u32)buf | 1, blocksize >> 2,
				      0xffffffff, 0))
			return FPGA_FAIL;

		bsize -= blocksize;
		pos   += blocksize;

		if (fs_set_blk_dev(interface, dev_part, fstype))
			return FPGA_FAIL;

		if (bsize > blocksize) {
			if (fs_read(filename, (u32) buf, pos, blocksize, &actread) < 0)
				return FPGA_FAIL;
		} else {
			if (fs_read(filename, (u32) buf, pos, bsize, &actread) < 0)
				return FPGA_FAIL;
		}
	} while (bsize > blocksize);

	buf = zynq_align_dma_buffer((u32 *)buf, blocksize, swap);

	if (zynq_dma_transfer((u32)buf | 1, bsize >> 2, 0xffffffff, 0))
		return FPGA_FAIL;

	dcache_enable();

	isr_status = readl(&devcfg_base->int_sts);

	/* Check FPGA configuration completion */
	ts = get_timer(0);
	while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
		if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
			printf("%s: Timeout wait for FPGA to config\n",
			       __func__);
			return FPGA_FAIL;
		}
		isr_status = readl(&devcfg_base->int_sts);
	}

	debug("%s: FPGA config done\n", __func__);

	if (!partialbit)
		zynq_slcr_devcfg_enable();

	return FPGA_SUCCESS;
}
#endif

#ifndef TJT
struct xilinx_fpga_op zynq_op = {
	.load = zynq_load,
#if defined(CONFIG_CMD_FPGA_LOADFS) && !defined(CONFIG_SPL_BUILD)
	.loadfs = zynq_loadfs,
#endif
};
#endif

#ifdef CONFIG_CMD_ZYNQ_AES
/*
 * Load the encrypted image from src addr and decrypt the image and
 * place it back the decrypted image into dstaddr.
 */
int zynq_decrypt_load(u32 srcaddr, u32 srclen, u32 dstaddr, u32 dstlen,
		      u8 bstype)
{
	u32 isr_status, ts;

	if (srcaddr < SZ_1M || dstaddr < SZ_1M) {
		printf("%s: src and dst addr should be > 1M\n",
		       __func__);
		return FPGA_FAIL;
	}

	/* Check AES engine is enabled */
	if (!(readl(&devcfg_base->ctrl) &
	      DEVCFG_CTRL_PCFG_AES_EN_MASK)) {
		printf("%s: AES engine is not enabled\n", __func__);
		return FPGA_FAIL;
	}

	if (zynq_dma_xfer_init(bstype)) {
		printf("%s: zynq_dma_xfer_init FAIL\n", __func__);
		return FPGA_FAIL;
	}

	writel((readl(&devcfg_base->ctrl) | DEVCFG_CTRL_PCAP_RATE_EN_MASK),
	       &devcfg_base->ctrl);

	debug("%s: Source = 0x%08X\n", __func__, (u32)srcaddr);
	debug("%s: Size = %zu\n", __func__, srclen);

	/* flush(clean & invalidate) d-cache range buf */
	flush_dcache_range((u32)srcaddr, (u32)srcaddr +
			roundup(srclen << 2, ARCH_DMA_MINALIGN));
	/*
	 * Flush destination address range only if image is not
	 * bitstream.
	 */
	if (bstype == BIT_NONE && dstaddr != 0xFFFFFFFF)
		flush_dcache_range((u32)dstaddr, (u32)dstaddr +
				   roundup(dstlen << 2, ARCH_DMA_MINALIGN));

	if (zynq_dma_transfer(srcaddr | 1, srclen, dstaddr | 1, dstlen))
		return FPGA_FAIL;

	if (bstype == BIT_FULL) {
		isr_status = readl(&devcfg_base->int_sts);
		/* Check FPGA configuration completion */
		ts = get_timer(0);
		while (!(isr_status & DEVCFG_ISR_PCFG_DONE)) {
			if (get_timer(ts) > CONFIG_SYS_FPGA_WAIT) {
				printf("%s: Timeout wait for FPGA to config\n",
				       __func__);
				return FPGA_FAIL;
			}
			isr_status = readl(&devcfg_base->int_sts);
		}
		printf("%s: FPGA config done\n", __func__);
		zynq_slcr_devcfg_enable();
	}

	return FPGA_SUCCESS;
}
#endif
