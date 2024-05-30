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

#	movw	sp, #:lower16:cur_thread
#	movt	sp, #:upper16:cur_thread
#	ldr	sp, [sp, #0]
	ldr	sp, =0x50000

	bl	main

spin:
        b      spin

# Idiotic junk to avoid complaints from linker
.section        .note.GNU-stack,"",%progbits

/* THE END */
