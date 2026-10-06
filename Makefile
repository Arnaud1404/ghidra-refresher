# Rebuilds the exercise binaries. The committed binaries were built with
# GCC 16.2.1 (Fedora 44); another compiler version may shift the addresses
# quoted in README.md.

CC      = gcc
ARM_CC  = aarch64-linux-gnu-gcc
# Keeps the build directory out of the debug info.
NOPATH  := -ffile-prefix-map=$(CURDIR)=.

BINS := ex1-64 ex2-64 shell-O2 shell-O2-stripped
OBJS := arm_demo-aarch64.o arm_demo-x86_64.o

.PHONY: all x86 arm clean
all: x86 arm
x86: $(BINS) arm_demo-x86_64.o
arm: arm_demo-aarch64.o

ex1-64: exercise-1.c
	$(CC) -Wall -Wextra -std=c11 -g -O0 $(NOPATH) -o $@ $<

ex2-64: exercise-2.c
	$(CC) -Wall -Wextra -std=c11 -g -O3 $(NOPATH) -o $@ $<
	strip $@

shell-O2: vuln_shell.c
	$(CC) -std=c17 -O2 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L $(NOPATH) -o $@ $<

shell-O2-stripped: shell-O2
	strip -o $@ $<

arm_demo-aarch64.o: arm_demo.c
	$(ARM_CC) -O2 -ffreestanding $(NOPATH) -c $< -o $@

arm_demo-x86_64.o: arm_demo.c
	$(CC) -O2 -ffreestanding $(NOPATH) -c $< -o $@

clean:
	rm -f $(BINS) $(OBJS)
