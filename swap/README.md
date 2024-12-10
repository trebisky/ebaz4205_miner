Swap - experiment with the SWAP bit in the OCM_CFG register

5-2024

An experiment after learning about some things in the bootrom.
I wanted to see if flipping the SWAP bit would expose the bootrom.
It turns out that there is more involved.  OCM is entirely hidden
by the SCU (snoop control unit),  We could try to turn that off,
which would involve flushing and invalidating caches and a lot
more than I care to do.

So this is a "failed experiment".  It did prove somewhat educational.
The way to investigate this is to perform an experiment where
I let the bootrom load some experimental code in lieu of the FSBL.
The SCU won't be enabled yet, so it won't interfere.

I did this, and the SWAP bit still didn't do what I hoped.
JTAG was a help in seeing what was going on, but that is well
beyond what we have here.
