Notes on compiler and Makefile issues for my EBAZ4205 Bitcoin miner board Zynq demos

I have used my Fedora x86 disktop (currently running Fedora 40) to generate these demos.

I originally used the "arm-linux-gnu" compiler, and have decided to stick with it.
It works.  I might make more sense to use the "arm-none-eabi" compiler packages,
but they require me to sort out a different set of options, and I am lazy.

The new compilers (as of Fedora 40) give several annoying warnings:

<pre>
/usr/bin/arm-linux-gnu-ld: warning: start.o: missing .note.GNU-stack section implies executable stack
/usr/bin/arm-linux-gnu-ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
/usr/bin/arm-linux-gnu-ld: warning: interrupts.elf has a LOAD segment with RWX permissions
</pre>

The build still works, but I would rather not be afflicted by irrelevant warnings.
Here are some explanations on my website:

[2023 - linker warnings] (http://cholla.mmto.org/orange_pi/pc_h3/notes/update_2023.html)
[2024 - linker warnings] (http://cholla.mmto.org/fpga/ebaz4205/bare/compiler.html)

In short, append this line to every assembler source (*.S) file:

1. .section        .note.GNU-stack,"",%progbits

Also add these options to the link line in the Makefile:

1. -Wl,--no-warn-rwx-segments
