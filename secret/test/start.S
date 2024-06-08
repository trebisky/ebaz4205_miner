/*
 * Copyright (C) 2024  Tom Trebisky  <tom@mmto.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. See README and COPYING for
 * more details.
 */

/* start.S
 *
 * Putting the stack at 0x3_8000 won't work.
 * That 64K chunk of memory is up at 0xffff_xxxx
 */

	.globl startup
startup:
	ldr	sp, =0x28000
	bl	main

spin:	b	spin

# Idiotic junk to avoid complaints from linker
.section        .note.GNU-stack,"",%progbits

/* THE END */
