EBAZ4205 Bitcoin miner board -- setup

This directory contains a collection of mostly Python scripts that I
have used while reverse engineering and configuring my EBAZ4205

What is the different between uboot_env and env_orig you ask?

uboot_env is what was dumped by printenv by a running uboot
env_orig was dumped from the uboot image on flash
So the running copy may have automatically added or augmented
the variables that are stored "on disk" in the Uboot executable.
