/* cache.c
 *
 * Just a few routines to flush the data cache.
 * This needs to be done before we do a DMA transfer
 * to the PL, otherwise, who knows what actually
 * gets sent to the poor PL.
 *
 * Tom Trebisky  6-6-2022
 *
 * The bulk of this is from uboot
 * arch/arm/cpu/armv7/cache_v7.c
 */

/* TJT says: I try to put all my hacks on this.
 */
#define TJT

#ifdef TJT
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int __u32;

typedef unsigned int size_t;

typedef unsigned long ulong;

#define NULL	(0)

/* The `const' in roundup() prevents gcc-3.3 from calling __divdi3 */
#define roundup(x, y) (                                 \
{                                                       \
        const typeof(y) __y = y;                        \
        (((x) + (__y - 1)) / __y) * __y;                \
}                                                       \
)


// From include/linux/kernel.h
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK((x),(typeof(x))(a)-1)

// See: arch/arm/include/asm/cache.h
#define CONFIG_SYS_CACHELINE_SIZE 64
#define ARCH_DMA_MINALIGN       CONFIG_SYS_CACHELINE_SIZE

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

/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
/* ------------------------------------------------------------- */
#endif /* end TJT */


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

static u32
get_ccsidr(void)
{
        u32 ccsidr;

        /* Read current CP15 Cache Size ID Register */
        asm volatile ("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));
        return ccsidr;
}

#ifdef notdef
/* Big (!) asm functions from:
 *  arch/arm/cpu/armv7/cache_v7_asm.S
 */
void v7_flush_dcache_all(void);
void v7_invalidate_dcache_all(void);

/* Flush the whole doggone thing.
 */
void
flush_dcache_all(void)
{
        v7_flush_dcache_all();

        // v7_outer_cache_flush_all();
}
#endif

/* ------------------------------------- */

static void
v7_dcache_clean_inval_range(u32 start, u32 stop, u32 line_len)
{
        u32 mva;

        /* Align start to cache line boundary */
        start &= ~(line_len - 1);
        for (mva = start; mva < stop; mva = mva + line_len) {
                /* DCCIMVAC - Clean & Invalidate data cache by MVA to PoC */
                asm volatile ("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
        }
}

static int
check_cache_range(unsigned long start, unsigned long stop)
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

static void
v7_dcache_inval_range(u32 start, u32 stop, u32 line_len)
{
        u32 mva;

        if (!check_cache_range(start, stop))
                return;

        for (mva = start; mva < stop; mva = mva + line_len) {
                /* DCIMVAC - Invalidate data cache by MVA to PoC */
                asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
        }
}

#define ARMV7_DCACHE_INVAL_RANGE        1
#define ARMV7_DCACHE_CLEAN_INVAL_RANGE  2

static void
v7_dcache_maint_range(u32 start, u32 stop, u32 range_op)
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

/*
 * Flush range(clean & invalidate) from all levels of D-cache/unified
 * cache used:
 * Affects the range [start, stop - 1]
 */
void
flush_dcache_range(unsigned long start, unsigned long stop)
{
        check_cache_range(start, stop);

        v7_dcache_maint_range(start, stop, ARMV7_DCACHE_CLEAN_INVAL_RANGE);

        // v7_outer_cache_flush_range(start, stop);
}

void
flush_dcache_buffer ( unsigned long start, int size )
{
	unsigned long stop;

	stop = start + roundup ( size, ARCH_DMA_MINALIGN );
        flush_dcache_range ( start, stop );
}

/* THE END */
