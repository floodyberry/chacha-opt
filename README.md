# ABOUT #

This is a solution to the problem of easy to use write-once, run everywhere x86 assembler. I have experimented with write-once inline assembler (in the style of [Crypto++](http://www.cryptopp.com/)), but it turned out to be too clunky and limited, e.g. source code must be transformed to Intel Syntax and then C macros which expand the instructions properly, referencing C constants in assembler which are not referenced by C is sketchy because the constants can get optimized away and/or are linked under a different name, Win64 versions have to jump through a lot of hoops to merely generate code which then has to be compiled by MASM, clang's integrated assembler (pre 3.2ish) does not understand `.intel_syntax`, etc.

I decided the only way was to switch to an external assembler. [Yasm](http://yasm.tortall.net/) appears to be the most well supported and up-to-date, and, unlike NASM, supports GAS syntax. This means, with limiting the assembler to AT&T syntax and a few careful macros, that it is possible to have x86 assembler that is compilable by either Yasm _or_ gcc compatible compilers!

Note that this is _not_ for coding entire programs in assembler, general purpose assembler with macros, interacting with external C/assembler, etc. It is for self-contained, instruction set specific kernels which can be CPU-dispatched at runtime, e.g. crypto routines.

x86 is fully supported, and the first pass at ARM support is now in!

# QUICK OVERVIEW #

* Write once, run everywhere assembler, using GCC and Yasm.
* Project name is set in [project.def](project.def) and version is set in [project.ver](project.ver)
* Platform specific code (cpu feature detection, cpu cycles, assembler macros) is in `src/driver/platform`
* Optimized implementations go in `extensions/name` and are exposed through `include/name.h`
* Sample `main.c` and fuzzing / benchmarking support is in [src](src).
* Platforms supported are x86, ARM.

# HOW IT WORKS #

I really wanted to avoid this, but before anything is done, a configuration script must be run to determine compiler capabilities, instruction sets supported, and so on. This is both for the assembler to know what it can assemble, and for the C code which will use the assembler to know which versions it can use.

gcc and Yasm each have their own bootstrap file ([gcc\_driver.inc](driver/gcc_driver.inc) and [yasm\_driver.inc](driver/yasm_driver.inc)) which handles determining platform, compiler, and setting up the macros needed. The initial file that includes the bootstrap code must have an `.S` extension to allow the C preprocessor to run for gcc compatible implementations. The C preprocessor macros are however only used to set up the GNU assembler macros which will be used by the assembler files; this is because there is no way to include a file from a macro with the C preprocessor.

## BOOTSTRAPPING ##

The standard header for each file is

    #if defined(__GNUC__)
    #include "gcc_driver.inc"
    #else
    ;.if 0
    %include "yasm_driver.inc"
    ;.endif
    #endif

gcc will include `gcc_driver.inc` and ignore the Yasm section. Yasm interprets # and ; as comments and will include `yasm_driver.inc`. Finally, a file included by a gnu as macro will interpret `#` as a comment, `;` as a statement separator, evaluate the `.if 0`, and skip `yasm_driver.inc` and wind up doing nothing.

## MACROS AVAILABLE ##

Once bootstrapped, the following macros are available to every assembler file.

### SECTIONS ###

* `SECTION_TEXT`

   Switch to the code section
* `SECTION_RODATA`

   Switch to the read-only data section. Right now this is `.text` to simplify position-independent variables in 32-bit code.

### INCLUDING FILES ###

Note: If you are compiling on OS X with gcc, any file that is included _must_ have Unix line endings! The `as` that ships with OS X appears to puke on Windows line endings and spits out a confusing mess of errors:

    ).3????7?~?:0:Junk character 13 (
    ??3????7?~?:0:invalid character (0xd) in operand 2
    ??3????7?~?:0:invalid character (0xd) in operand 1

I _believe_ I have this automatically done through .gitattributes now, but, you know, for posterity and search engines.

* `INCLUDE` "file"

   Include `file`

* `INCLUDE_VAR_FILE` "file", variable_name

   Include `file` if `variable_name` has not been defined. When multiple implementations require the same constants, they can use `INCLUDE_VAR_FILE` to
only use a single copy instead of pulling in redundant copies.

#### INCLUDE BASED ON AVAILABLE INSTRUCTION SETS ####

Extension based includes are available for all combinations of [X86, MMX, SSE, SSE2, SSE3, SSSE3, SSE4\_1, SSE4\_2, AVX, XOP, AVX2, AVX512] and [32BIT, 64BIT]

* `INCLUDE_IF_EXT_XXBIT` "file"

   Include `file` if the assembler supports EXT instructions and is in XXBIT mode. e.g. `INCLUDE_IF_AVX2_32BIT`, `INCLUDE_IF_X86_32BIT`, `INCLUDE_IF_SSE4_1_64BIT`, etc.

#### INCLUDING ON NON-X86 PLATFORMS ####

For the moment, non-x86 platforms are gcc only, so you may use standard #if defined / #include / #endif in your .S file. This has the added bonus of allowing included files to be tracked by gcc's makefile dependency generation.

### FUNCTION SUPPORT ###

* `GLOBAL` name

   Declares `name` as a global symbol
* `HIDDEN` name

   Declares `name` as a hidden global symbol, if supported.
* `FN` name
 
   Declares a function named `name`
* `FN_EXT` name, args, xmmused

   Declares a function named `name`, which takes `args` args and uses `xmmused` xmm registers. This is only available for x86-64 because arguments need to be translated for Win64 and xmm6..15 have to be preserved if they are used. args can be 0 to 6, more than 6 arguments are currently not handled.
* `FN_END` name

   Declares the end of function `name`. Currently only used when compiling to ELF object format to tag the type and size of the function.
* `LOAD_VAR_PIC` var, reg

   Loads the address of `var` in to `reg` in a position-independent manner. This is an `lea` for 64 bits, but 32 bits costs an extra `call` and `pop`. Any address that is needed frequently should be cached locally.

* Local Names:
    * `FN_LOCAL` name
    * `FN_EXT_LOCAL` name, args, xmmused
    * `FN_END_LOCAL` name

    Like their above versions, except prefixed with the project name. This is done so systems with no support for hidden symbols will not have symbol clashes for common names. To use the resulting symbols in C, use `LOCAL_PREFIX(name)`.


## CPUID ##

CPUID implementations are in `driver/arch/` and exposed through [cpuid.c](driver/cpuid.c) with `unsigned long cpuid(void)`.

The [x86 cpuid](driver/x86/cpuid_x86.S) detects everything from MMX up to (theoretically, based on Intel's programming reference) AVX-512. The implementation "cheats" by having the bootstrap provide `CPUID_PROLOGUE` and `CPUID_EPILOGUE` so a single implementation can be used for both x86 and x86-64.

Also provided are example runtime dispatching functions to test and select the optimal version based on the current CPU.

### CPUID FLAGS ###

A value of `CPUID_GENERIC` (or 0) indicates the underlying CPU is unknown and is common to all platforms.

    CPUID_GENERIC   = 0

#### X86 ####

Major architecture flags start from the bottom, while individual features go from the top. They'll meet in the middle some day.

    CPUID_X86       = (1 <<  0)
    CPUID_MMX       = (1 <<  1)
    CPUID_SSE       = (1 <<  2)
    CPUID_SSE2      = (1 <<  3)
    CPUID_SSE3      = (1 <<  4)
    CPUID_SSSE3     = (1 <<  5)
    CPUID_SSE4_1    = (1 <<  6)
    CPUID_SSE4_2    = (1 <<  7)
    CPUID_AVX       = (1 <<  8)
    CPUID_XOP       = (1 <<  9)
    CPUID_AVX2      = (1 << 10)
    CPUID_AVX512    = (1 << 11)
    
    CPUID_RDRAND    = (1 << 26)
    CPUID_POPCNT    = (1 << 27)
    CPUID_FMA4      = (1 << 28)
    CPUID_FMA3      = (1 << 29)
    CPUID_PCLMULQDQ = (1 << 30)
    CPUID_AES       = (1 << 31)

### IMPLEMENTATION SELECTION BY CPUID ###

    typedef struct cpu_specific_impl_t {
        unsigned long cpu_flags;
        const char *desc;
        /* additional information, pointers to methods, etc... */
    } cpu_specific_impl_t;
    
    const void *cpu_select(const void *impls, size_t impl_size, impl_test test_fn)

`cpu_select` returns a pointer to the first implementation that will run on the current CPU and passes the provided test. If no implementations passes, NULL is returned.

`impls` is a pointer to an array of structs where each struct represents an optimized implementation with the first field being an `unsigned long` that holds the required cpu flags (see `cpu_specific_impl_t`), and the second being a pointer to an arbitrary string describing the implementation, e.g. "x86", "avx2-popcnt", etc. The structs must be ordered from most the optimized implementation to the least.

`impl_size` is the size of each struct.

`test_fn` is a pointer to a function taking a `const void *` which points to an optimized implementation, and returns an `int` which is `0` if the implementation passes all tests.

## LIBRARY SUPPORT ##

Static and shared library support is now mostly done! 

When available, every function/variable is treated as hidden/private by default. Mark a function/variable for export by using `LIB_PUBLIC` for prototypes and the actual instance, e.g.:

    LIB_PUBLIC int
    some_public_function(void);

    LIB_PUBLIC int
    some_public_function(void) {
        return 42;
    }

In each of your public headers, you must add a simple stub to define `LIB_PUBLIC` to blank if it has not already been defined. This is the minor cost of having no external headers required (other than `<stddef.h>` for size_t):

    #if !defined(LIB_PUBLIC)
        #define LIB_PUBLIC
    #endif

If you are using a common name for a function that may clash with another library if hidden/private is not supported, e.g. `cpuid`, wrap any reference to it with `LOCAL_PREFIX` to have the name of the project added as a prefix:

    unsigned long
    LOCAL_PREFIX(cpuid)(void) {
        return CPU_GENERIC;
    }
    
    static void
    some_static_function(void) {
        unsigned long cpuflags = LOCAL_PREFIX(cpuid)();
        /* does stuff with cpuflags here */
    }

### CANNOT FIND -LEXAMPLE ###

If you are getting `/usr/bin/ld: error: cannot find -lexample` when trying to link against your new library, and you have the library in `/usr/local/lib`, you may be running in to [Shared library in /usr/local/lib not found](http://stackoverflow.com/questions/5873516/shared-library-in-usr-local-lib-not-found). The problem is the system is using the gold linker, which for no discernable reason does not check `/usr/local/lib` (what the hell). You will need to uninstall it (`apt-get remove binutils-gold`, etc.), or add `/usr/local/lib` to `LIBRARY_PATH`.

## BUILDING ##

### NAME ###

The name of the project is set in [project.def](project.def). This is used to create project specific function names using `LOCAL_PREFIX`.

The project version is in [project.ver](project.ver). Unused except for shared library names on some *nix's.

### CONFIGURING ###

    ./configure [options]
 
#### HELP ####
 * `-h`, `--help`: Prints help

#### INSTALLATION OPTIONS ####

  * `--prefix=PREFIX`: Install architecture-independent files in PREFIX [default: `/usr/local`]
  * `--exec-prefix=EPREFIX`: Install architecture-dependent files in EPREFIX [default: `PREFIX`]
  * `--bindir=DIR`: Install binaries in DIR [default: `EPREFIX/bin`]
  * `--libdir=DIR`: Install libs in DIR [default: `EPREFIX/lib`]
  * `--includedir=DIR`: Install includes in DIR [default: `PREFIX/include`]

#### CONFIGURATION OPTIONS ####
 * `--debug`: Builds with no optimization and debugging symbols enbaled
 * `--disable-as`: Do not use external assembly
 * `--force-32bits`: Build for 32bits regardless of underlying system
 * `--force-64bits`: Build for 64bits regardless of underlying system
 * `--generic`: Alias for --disable-as, forces a generic build
 * `--pic`: Pass `-fPIC` to the compiler. If you are using `LOAD_VAR_PIC` properly, all assembler will be PIC safe by default. This is required for shared builds
 * `--strict`: Use strict compiler flags for C
 * `--yasm`: Use Yasm to compile external asm

#### SHELL VARIABLES USED ####

 * `CC`: The C compiler to use [default: gcc]
 * `AR`: The archiver to use [default: ar]
 * `LD`: The linker to use [default: gcc -o]
 * `RANLIB`: The indexer to use [default: ranlib]
 * `STRIP`: The symbol stripper to use [default: strip]
 * `INSTALL`: The installer to use [default: install]
 * `CFLAGS`: Additional C flags to pass to the compiler
 * `SOFLAGS`: Additional flags to pass to `LD` when compiling a shared library

Well some may not be used yet, but you know, for the future.

### COMPILING ###

* `make` or `make lib` compile as a static library
* `make shared` compile as a shared library (requires --pic except on windows)
* `make exe` creates a sample executable
* `make util` creates a fuzzing / benchmarking executable


### INSTALLING ###

* `make install-lib` installs as a static library
* `make install-shared` installs as a shared library

### VISUAL STUDIO ###

I've got the Visual Studio project generator working! It generates a Visual Studio [2010,2012,2013] solution with projects for a static library, dynamic library, and utility project for both 32 and 64 bits. Generated files (exe, lib, dll) are currently placed in `asm-opt/bin/[Release|Debug]/[amd64|x86-32bit]/`.

It only requires that [yasm.exe](http://yasm.tortall.net/Download.html) be somewhere in the system path for Visual Studio to execute. You can place `yasm.exe` in the `vs2010/` directory if you're especially lazy.

    php genvs.php [options]

#### OPTIONS ####

##### REQUIRED #####

* `--version=VERSION`: Select the Visual Studio version to generate a solution for. [`vs2010`, `vs2012`, `vs2013`]

##### OPTIONAL #####

* `--disable-yasm`: Compile without Yasm support, i.e. with only reference versions

## EXAMPLE ##

See [EXAMPLE](docs/EXAMPLE.md)

## UTILITIES ##

See [UTILITIES](docs/UTILITIES.md)

## ISSUES ##

Issues keeping things from being 'perfect'. See [ISSUES](docs/ISSUES.md)

# LICENSE #

Public Domain, or MIT
