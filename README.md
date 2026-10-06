# Ghidra refresher: minimal viable exercises

About two hours to go from "I did a couple of reverse-engineering labs once" to "I can open
an unknown stripped binary and explain what it does". The exercises start with a small
program that still has its symbols, and end on a target close to real work: a stripped,
optimized binary with real bugs, then the same code on ARM.

Answer each question out loud **before** you open the answer key at the bottom.

> [!WARNING]
> `vuln_shell.c` and the `shell-O2*` binaries are **deliberately vulnerable** (buffer
> overflow, format string, memory leak, uninitialized read). They are here for study only.
> Do not install them anywhere or expose them to untrusted input.

## Contents

| File | What it is |
|---|---|
| `ex1-64` (`exercise-1.c`) | Recursive Fibonacci, `-O0`, with symbols and debug info |
| `ex2-64` (`exercise-2.c`) | A function that returns a struct, `-O3`, stripped |
| `shell-O2-stripped` (`vuln_shell.c`) | A small shell with four bugs, `-O2`, stripped |
| `shell-O2` | The same shell, not stripped (to check your work, not to start from) |
| `arm_demo-aarch64.o`, `arm_demo-x86_64.o` (`arm_demo.c`) | The same two functions, built for AArch64 and for x86-64 |
| `scripts/DumpDecomp.java` | Headless script that prints the decompiled C of every function |
| `Makefile` | Rebuilds every binary |

The binaries are committed on purpose: the addresses quoted below (`FUN_00400520`,
`DAT_00403100`...) come from these exact builds. They were built with GCC 16.2.1 on
Fedora 44 (x86-64 and the `aarch64-linux-gnu` cross-compiler). If you rebuild with
another compiler, the questions still apply, but the addresses will move.

## 0. Setup

| What | Notes |
|---|---|
| Ghidra | Tested with 12.1.4. Download the `PUBLIC` zip from the [releases page](https://github.com/NationalSecurityAgency/ghidra/releases), check its SHA-256, unzip, run `ghidraRun` |
| JDK | Ghidra 12.1 needs JDK 21 or newer (64-bit) |
| x86-64 compiler (only to rebuild) | `gcc` |
| AArch64 cross-compiler (only to rebuild) | `gcc-aarch64-linux-gnu` on Fedora, Debian and Ubuntu. The exercise needs no libc: `arm_demo.c` is freestanding |

On WSL2, the GUI runs through WSLg. The first start takes about 20 seconds.

To rebuild the binaries:

```sh
make clean && make      # or `make x86` with no cross-compiler
```

To check what Ghidra sees without the GUI, run the headless analyzer with the script in
`scripts/` (it imports into a throwaway project and deletes it afterwards):

```sh
mkdir -p /tmp/ghidra-proj
$GHIDRA_HOME/support/analyzeHeadless /tmp/ghidra-proj tmp \
  -import shell-O2-stripped -scriptPath scripts -postScript DumpDecomp.java \
  -deleteProject
```

## The handful of actions you need

| Action | How |
|---|---|
| Rename a function, variable or global | select it, `L` |
| Retype a variable | select it, `Ctrl+L` |
| Comment | `;` in the listing |
| Go to an address or a name | `G` |
| Who uses this? | right-click, References, Show References to |
| Edit a function's prototype | right-click the function name in the decompiler, Edit Function Signature |
| All strings | Window, Defined Strings |
| Control flow graph | Window, Function Graph |
| Create a struct | Data Type Manager, right-click the program node, New, Structure |

---

## Exercise 1: warm-up with symbols (15 min)

`ex1-64`: `-O0`, not stripped. Create a new project (File, New Project, non-shared),
import `ex1-64`, and accept the default analysis.

1. Find `main`, `display_fibonacci` and `fibonacci` in the Symbol Tree. For `fibonacci`,
   put the source, the listing and the decompiler side by side.
2. In the listing of `fibonacci`: which register holds `n` on entry? Which one holds the
   return value? Where is the recursive call, and what happens to the stack around it?
3. Open the Function Graph of `display_fibonacci`. Point at the loop: which block is the
   test, which is the body, which edge goes back?
4. There is a `FUN_00400360` that the source never mentions. What is it? (Hint: which
   section is it in? Look at what it jumps through.)

## Exercise 2: stripped and `-O3` (25 min)

`ex2-64`: `-O3`, stripped. Keep `exercise-2.c` open: this exercise compares what the
compiler did with code you already know. Try each question from the binary first, then
check it against the source. (Exercise 3 is the one you start without the source.)

1. The Symbol Tree has no `main`. Start from `entry`: which argument of
   `__libc_start_main` is `main`? Rename it.
2. In `main`, name the globals and constants: `DAT_00403014`, `_DAT_004011f0`, and the
   literal `0x65`. Which source values are they?
3. `process_data` is called once in the source. Is there a call in `main`? Where did it go,
   and why does a copy of it still exist elsewhere (`FUN_00400520`)?
4. In `main`, `result.label` seems to have vanished: the `printf` prints `local_14`, which
   is `name`. Why is the compiler allowed to do that?
5. A common claim is that a function returning a struct gets a hidden pointer as its
   first argument. Look at `FUN_00400520`'s signature on x86-64: is there a hidden
   pointer? Explain the `ulong` return and the `CONCAT44`.
6. Create `result_t` by hand (int, char[4], double: check the offsets and the total size),
   then apply it where it fits.

## Exercise 3: a stripped program with real bugs (40 min)

`shell-O2-stripped`: a small shell with deliberate bugs (`vuln_shell.c`), `-O2`,
stripped. **Do not open `vuln_shell.c` until the end.** This is the realistic exercise:
treat it like a service running on a device.

1. Strategy first: with no symbols, what are your entry points into the code? Use Defined
   Strings to find `"recall"` and `"history"`, follow the references, and land in the
   function that handles commands. Rename it.
2. How many functions did you expect from a shell with history, parsing, builtins and
   `fork`? How many does Ghidra show? Explain.
3. Name the globals from their use: `DAT_00403100`, `DAT_004030e0`, `DAT_004030c8`,
   `DAT_004030c0`. What type and size is each one? How can you bound the size of
   `DAT_00403100` from the binary alone?
4. Find, without the source, **four defects**:
   - a copy into a fixed-size buffer with no length check;
   - a `printf` whose format comes from the user;
   - a heap allocation whose previous value is overwritten without being freed;
   - a heap block read before it was ever written.

   For each one: the line in the decompiler, and what input reaches it.
5. The recall builtin checks `if (3 < uVar4)`. The source checks `slot < 0 || slot >= 4`.
   Where did the `slot < 0` test go?
6. `local_220[0]` is passed to `strtok_r` as its state, and the arguments start at
   `local_220[1]`. What did the decompiler merge? Fix it (retype, or right-click the
   variable and split it out if your version offers it).
7. Now open `vuln_shell.c` and check your four findings against the source.

## Exercise 4: the same code on ARM (20 min)

Import both `arm_demo-aarch64.o` and `arm_demo-x86_64.o`. Ghidra reads the architecture
from the ELF header (`AARCH64:LE:64:v8A` for the first).

1. `checksum` on both: in which registers do `buf` and `len` arrive, and where does the
   result leave? (AArch64 vs x86-64 calling convention.)
2. In the AArch64 loop, find the load that also advances the pointer, and the `eor` that
   also shifts. What does x86-64 need instead?
3. What does `cbz` do, and what does it replace on x86-64?
4. `fibonacci` on AArch64 starts with `paciasp` then `stp x29, x30, [sp, #-144]!`. What is
   `x30`? Why is it saved, when an x86-64 `call` needs no such instruction? What does
   `paciasp` add, and how does it relate to a stack canary?
5. `checksum` starts with `bti c`. What is it for?

## Exercise 5: say it out loud (10 min)

1. What does `strip` remove, and what does it leave that you used in exercise 3?
2. Why is decompiled code never the original source? Give three causes you saw today.
3. How do you find `main` in a stripped, dynamically linked ELF?
4. Ghidra read the architecture from the ELF header. A firmware dump is often a raw
   blob: what must you tell Ghidra yourself, and how could you find it?
5. Ghidra is static. What would you do with gdb on exercise 3 that Ghidra cannot do?

If you can say something like this in your own words, you are done:

> "On a stripped binary, I start from what survives `strip`: the strings and the imports.
> I follow the references back to the function that handles the input, I rename and
> retype as I go, and I look for dangerous calls, such as a `printf` whose format comes
> from the user."

---

## Answer key (check after answering)

<details>
<summary>Exercise 1</summary>

2. `n` arrives in `edi` (first integer argument, System V AMD64); the result leaves in
   `eax`. At `-O0`, `n` is first spilled to the stack (a `[rbp-0x14]`-style slot) and
   reloaded around each `call fibonacci`; `call` pushes the return address.
3. The loop: one block tests `i <= n`, the body calls `fibonacci` then `printf`, and an
   edge returns from the end of the body to the test.
4. `FUN_00400360` is in `.plt`: it is **PLT0**, the lazy-binding stub. It pushes a pointer
   from the GOT and jumps through `GOT[2]` to the dynamic linker's resolver. Every
   `xxx@plt` entry falls back to it the first time an import is called.

</details>

<details>
<summary>Exercise 2</summary>

1. `entry` calls `__libc_start_main(FUN_00400390, ...)`: the first argument is `main`.
2. `DAT_00403014` is `g_multiplier` (global, read in the loop bound and the score).
   `_DAT_004011f0` is the constant `4.5` in `.rodata`. `0x65` is `101`, the `id`, propagated
   as a constant.
3. At `-O3`, `process_data` was **inlined** into `main`. A copy still exists because the
   function is not `static`: another file could call it, so the compiler must keep it.
4. `label` is `strncpy(.., name, 3)` and `name` is also `strncpy(.., argv[1], 3)`: same
   bytes, so the compiler reuses `name`. Variables that hold identical values get merged.
5. No hidden pointer on x86-64: a 16-byte struct of class INTEGER (`id` + `label`, 8 bytes)
   and SSE (`score`) is returned in **`rax` and `xmm0`**. Ghidra shows the `rax` half as a
   `ulong` built with `CONCAT44(label, id)`, and misses the `xmm0` half. The hidden pointer
   is the 32-bit x86 (cdecl) case, and the x86-64 case for structs larger than 16 bytes.
6. `result_t`: `id` at 0 (4 bytes), `label` at 4 (4 bytes), `score` at 8 (8 bytes,
   8-aligned), total **16 bytes**.

</details>

<details>
<summary>Exercise 3</summary>

1. Strings and imports survive `strip`: the dynamic symbols (`printf`, `strcpy`, `fork`...)
   must stay for the loader, and literals stay in `.rodata`. References from `"recall"`
   lead to `FUN_004004c0`, which is `main`.
2. About seven functions in the source; the program part of the binary is essentially
   `main`. At `-O2`, the `static` helpers called once (`history_init`, `record_history`,
   `show_history`, `recall_slot`, `parse_input`, `execute_command`) are inlined.
3. `DAT_00403100`: `last_command`, `char[32]` (destination of `strcpy`). `DAT_004030e0`:
   `history`, `char *[4]` (indexed by `slot * 8`). `DAT_004030c8`: `slot_used`, `int *`
   (from `malloc(0x10)`, indexed by `slot * 4`). `DAT_004030c0`: `history_count`, `int`
   (incremented, taken modulo 4). Size bound: the next global after `DAT_00403100`, or the
   end of `.bss`, in the listing or with `readelf -S` / Ghidra's Memory Map.
4. The four defects:
   - **C1**: `strcpy(&DAT_00403100, local_230)`, reached by any line of 32 bytes or more.
   - **C2**: `printf(pcVar6, &DAT_00403100)` where `pcVar6` is the second word of a
     `history` line, so `history %p%p` controls the format.
   - **C3**: `*(char **)(&DAT_004030e0 + slot * 8) = strdup(...)` with no `free` of the old
     pointer: from the fifth line on, the ring overwrites and leaks.
   - **C4**: `malloc(0x10)` for `slot_used`, never zeroed, then read in `recall` before
     any slot was written (`recall 1` on a fresh session).
5. `atoi` became `strtol(.., 10)` cast to `uint`. A negative slot becomes a huge unsigned
   value, so one unsigned comparison `3 < uVar4` covers both `< 0` and `>= 4`.
6. The decompiler merged two stack objects: `tokenizer_state` (8 bytes) sits right below
   `parsed_args[64]`, so it shows one `char *[65]` array whose slot 0 is the `strtok_r`
   state.

</details>

<details>
<summary>Exercise 4</summary>

1. AArch64: `buf` in `x0`, `len` in `w1` (32-bit view of `x1`), result in `w0`. x86-64:
   `rdi`, `esi`, result in `eax`.
2. `ldrb w3, [x2], #1` loads a byte and post-increments `x2`; `eor w0, w3, w0, lsl #1`
   shifts `w0` inside the `eor` (barrel shifter). x86-64 needs separate instructions:
   `movzx`, then `add rdi, 1`, and `add eax, eax` for the shift before the `xor`.
3. `cbz w1, label`: compare with zero and branch in one instruction. x86-64 uses
   `test esi, esi` then `je`.
4. `x30` is the **link register**: `bl` stores the return address there, not on the stack.
   A function that calls others must save it (with the frame pointer `x29`) or the next
   `bl` overwrites it. `paciasp` **signs** the return address with a pointer-authentication
   code, checked before `ret`: a forged return address fails the check. A canary detects an
   overwrite of the stack *next to* the return address; PAC protects the address itself.
5. `bti c` (Branch Target Identification) marks a valid target for indirect calls; with
   BTI enforced, an indirect branch landing elsewhere faults. It limits jump- and
   call-oriented programming. The cross-compiler used here emits PAC and BTI by default;
   others may need `-mbranch-protection=standard`.

</details>

<details>
<summary>Exercise 5</summary>

1. `strip` removes `.symtab` (local and static function names, globals) and debug info. It
   cannot remove the dynamic symbols needed by the loader (imports), nor strings and code.
2. Inlining, constant propagation and variable merging (exercise 2), types lost to
   `undefined`/`ulong`, struct returns split across registers, stack objects merged
   (exercise 3), loops reshaped by the optimizer.
3. `entry` (`_start`) passes `main` as the first argument of `__libc_start_main`.
4. The **architecture and endianness** (the language) and the **base address** where the
   blob is loaded. Clues: strings and the vendor's documentation for the chip, the reset
   vector or vector table at the start of the image, absolute pointers in the data that
   only make sense for one base address.
5. Break on the dangerous calls (`strcpy`, `printf`), inspect the real arguments and
   memory at run time, confirm what input reaches them, and watch the effect of an
   overflow on the neighbouring globals.

</details>

## Credits and license

Exercises 1 and 2 are adapted from a university reverse-engineering lab. `vuln_shell.c`
comes from [c-secure-build](https://github.com/Arnaud1404/c-secure-build).

Released under the [MIT License](LICENSE): you may reuse, adapt and share these
exercises, as long as you keep the copyright notice. If you use them in a course, a
workshop or a write-up, please credit:

> Arnaud Gomes, *Ghidra refresher*, https://github.com/Arnaud1404/ghidra-refresher
