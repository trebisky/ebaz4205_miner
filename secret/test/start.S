/*
 * Copyright (C) 2024  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */
/* start.S
 */

	.globl startup
startup:
	ldr	sp, =0x38000
xyz:	b	xyz

	# The OCM_CFG register
	mov	r0,#0x0910
	movt	r0,#0xf800
	mov	r1, #8

	# Now pad to put the spin loop
	# at the same address as the one in the bootrom
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	# flip memory
	str	r1, [r0]

# bootrom has spin loop at 0x3c
spin:	b	spin

# This spews out endless 'N' characters
loop:
	mov r0, #0x1000
	movt r0, #0xe000
	mov ip, #0x4e
	str ip, [r0, #0x30]
	dsb sy
	isb sy
	b loop

	.globl Xstartup
Xstartup:

#	movw	sp, #:lower16:cur_thread
#	movt	sp, #:upper16:cur_thread
#	ldr	sp, [sp, #0]
#	ldr	sp, =0x40000
	ldr	sp, =0x38000

	bl	main

spin2:
        b      spin2

# Idiotic junk to avoid complaints from linker
.section        .note.GNU-stack,"",%progbits

/* THE END */
