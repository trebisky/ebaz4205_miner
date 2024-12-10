Get timer and uart interrupts on the EBAZ4205

I am not sure I ever got this to work .....

----

Copied from /u1/Projects/OrangePi/Archive/inter_kyu

This was first made to work with a serial rcv interrupt,
then a simple timer driver was added.

This gets timer interrupts at 1000 Hz, reporting on them
continuously at 1 Hz on the console.
Keypresses generate interrupts which are reported.

----

The Orange Pi version of this was derived from was a
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
