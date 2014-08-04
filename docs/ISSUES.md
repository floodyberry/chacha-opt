* **x86 only** `gcc` is unable to track modified assembler files included by the `as` macros due to the acrobatics required to gather files included by `gcc` (with the C preprocessor) and `as` (with the `as macros`). 

* `Yasm`, using the `GAS` parser, does not currently support `.hidden` (ELF) or `.private_extern` (Mach-O). That, or I cannot figure out how to properly invoke them. If you are only using `Yasm` for `MSVC`, this is a non-issue.
