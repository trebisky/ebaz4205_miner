Get timer and uart interrupts on the EBAZ4205

----

Originally written in January, 2021 using arm-linux-gnu-gcc
as the cross compiler.

In May of 2024, I tried to build it again.  I no longer have
arm-linux-gnu-gcc.  I tried switching to arm-none-eabi, but that
will take some work to sort out compiler options in the Makefile.

The quick answer is: "dnf install gcc-arm-linux-gnu"

This "just works", but now we get annoying warnings from "ld".

1. /usr/bin/arm-linux-gnu-ld: warning: start.o: missing .note.GNU-stack section implies executable stack
1. /usr/bin/arm-linux-gnu-ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
1. /usr/bin/arm-linux-gnu-ld: warning: interrupts.elf has a LOAD segment with RWX permissions

Append this to xxx.S

.section        .note.GNU-stack,"",%progbits

Add this to the linker options:

-Wl,--no-warn-rwx-segments

----

Copied from /u1/Projects/OrangePi/Archive/inter_kyu

This was first made to work with a serial rcv interrupt,
then a simple timer driver as added.

This gets timer interrupts at 1000 Hz, reporting on them
continuously at 1 Hz on the console.
Keypresses generate interrupts which are reported.

The Orange Pi version this was derived from was a
quick hack pulling all kinds of stuff from Kyu.
My comments on that from the Orange Pi archive follow:

----

After pulling my hair out with some simple code
to demonstrate timer interrupts, I decided I could
help troubleshoot things by pulling in a few modules
from Kyu that would tell me about data aborts and give
traceback on exceptions, and such things.

So after a no holds barred hack-job editing session,
lo and behold it all just works!

So, we have here code that demonstrates using timer
interrupts on the Allwinner H3 using the ARM GIC.

This will be a useful stepping stone towards getting
Kyu running on the Orange Pi.

This code has no trouble with the timer running at
    10,000 Hz
