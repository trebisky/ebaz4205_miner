/*
 * Copyright (C) 2024  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

/* flood.S - send endless characters.  A first experiment to see if we could
 * get anything to run loaded via the uart bootrom secret loader.
 */

	.globl startup

# This spews out endless 'N' characters
loop:
	mov r0, #0x1000
	movt r0, #0xe000
	mov ip, #0x4e
	str ip, [r0, #0x30]
	dsb sy
	isb sy
	b loop

# An experiment to see if we can load the SP
# We use JTAG to see if this works.

startup:
	ldr	sp, =0x38000
xyz:	b	xyz

# Another experiment to see if we can manipulate
# the SWAP bit in the OCM_CFG register and get the
# bootrom back at address 0 (it didn't work).
# Once again, we use JTAG to see what this does.

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
	# this is the grand experiment of clearing the SWAP bit,
	# I see no effect.  So the experiment is a failure.
	str	r1, [r0]

# bootrom has spin loop at 0x3c
spin:	b	spin

# Idiotic junk to avoid complaints from linker
.section        .note.GNU-stack,"",%progbits

/* THE END */
