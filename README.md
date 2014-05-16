# ABOUT #

This is a solution to the problem of easy to use write-once, run everywhere x86 assembler. I have experimented with write-once inline assembler (in the style of [Crypto++](http://www.cryptopp.com/)), but it turned out to be too clunky and limited, e.g. source code must be transformed to Intel Syntax and then C macros which expand the instructions properly, referencing C constants in assembler which are not referenced by C is sketchy because the constants can get optimized away and/or are linked under a different name, Win64 versions have to jump through a lot of hoops to merely generate code which then has to be compiled by MASM, clang's integrated assembler (pre 3.2ish) does not understand `.intel_syntax`, etc.

I decided the only way was to switch to an external assembler. [Yasm](http://yasm.tortall.net/) appears to be the most well supported and up-to-date, and, unlike NASM, supports GAS syntax. This means, with limiting the assembler to AT&T syntax and a few careful macros, that it is possible to have x86 assembler that is compilable by either Yasm _or_ gcc compatible compilers!

Note that this is _not_ for coding entire programs in assembler, general purpose assembler with macros, interacting with external C/assembler, etc. It is for self-contained, instruction set specific kernels which can be CPU-dispatched at runtime, e.g. crypto routines.

Room has been made for other architectures to fit in to the sample framework, but until I uh, run in to other architectures, nothing is actually done for them yet!

# HOW IT WORKS #

I really wanted to avoid this, but before anything is done, a configuration script must be run to determine compiler capabilities, instruction sets supported, and so on. This is both for the assembler to know what it can assemble, and for the C code which will use the assembler to know which versions it can use.

gcc and Yasm each have their own bootstrap file ([gcc\_driver.inc](driver/gcc_driver.inc) and [yasm\_driver.inc](config/yasm_driver.inc)) which handles determining platform, compiler, and setting up the macros needed. The initial file that includes the bootstrap code must have an `.S` extension to allow the C preprocessor to run for gcc compatible implementations. The C preprocessor macros are however only used to set up the GNU assembler macros which will be used by the assembler files; this is because there is no way to include a file from a macro with the C preprocessor.

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

* `INCLUDE` "file"

   Include `file`

* `INCLUDE_VAR_FILE` "file", variable_name

   Include `file` if `variable_name` has not been defined. When multiple implementations require the same constants, they can use `INCLUDE_VAR_FILE` to
only use a single copy instead of pulling in redundant copies.

#### INCLUDE BASED ON AVAILABLE INSTRUCTION SETS ####

Extension based includes are available for all combinations of [X86, MMX, SSE, SSE2, SSE3, SSSE3, SSE4\_1, SSE4\_2, AVX, XOP, AVX2, AVX512] and [32BIT, 64BIT]

* `INCLUDE_IF_EXT_XXBIT` "file"

   Include `file` if the assembler supports EXT instructions and is in XXBIT mode. e.g. `INCLUDE_IF_AVX2_32BIT`, `INCLUDE_IF_X86_32BIT`, `INCLUDE_IF_SSE4_1_64BIT`, etc.


### FUNCTION SUPPORT ###

* `GLOBAL` name

   Declares `name` as a global symbol
* `FN` name
 
   Declares a function named `name`
* `FN_EXT` name, args, xmmused

   Declares a function named `name`, which takes `args` args and uses `xmmused` xmm registers. This is only intended for 64 bit functions because arguments need to be translated for Win64 and xmm6..15 have to be preserved if they are used. args can be 0 to 6, more than 6 arguments are currently not handled.
* `FN_END` name

   Declares the end of function `name`. Currently only used when compiling to ELF object format to tag the type and size of the function.
* `LOAD_VAR_PIC` var, reg

   Loads the address of `var` in to `reg` in a position-independent manner. This is an `lea` for 64 bits, but 32 bits costs an extra `call` and `pop`. Any address that is needed frequently should be cached locally.

## CPUID ##

CPUID implementations are in `driver/arch/` and exposed through [cpuid.c](driver/cpuid.c) with `uint32_t cpuid(void)`.

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
        uint32_t cpu_flags;
        const char *desc;
        /* additional information, pointers to methods, etc... */
    } cpu_specific_impl_t;
    
    const void *cpu_select(const void *impls, size_t impl_size, impl_test test_fn)

`cpu_select` returns a pointer to the first implementation that will run on the current CPU and passes the provided test. If no implementations passes, NULL is returned.

`impls` is a pointer to an array of structs where each struct represents an optimized implementation with the first field being an `uint32_t` that holds the required cpu flags (see `cpu_specific_impl_t`), and the second being a pointer to an arbitrary string describing the implementation, e.g. "x86", "avx2-popcnt", etc. The structs must be ordered from most the optimized implementation to the least.

`impl_size` is the size of each struct.

`test_fn` is a pointer to a function taking a `const void *` which points to an optimized implementation, and returns an `int` which is `0` if the implementation passes all tests.

## EXAMPLE ##

An [example](src/example/) is provided demonstrating how to implement, select, and call an optimized function. The example algorithm being implemented is summing up the little-endian 32-bit signed integers in a given array and returning the result. 

### PUTTING IT TOGETHER ###

#### ASSEMBLER ####

[example.S](src/example/example.S) includes the bootstrap header and aggregates all the implementations in to a single file:

    #if defined(__GNUC__)
    #include "gcc_driver.inc"
    #else
    ;.if 0
    %include "yasm_driver.inc"
    ;.endif
    #endif
    
    INCLUDE_IF_AVX_64BIT "example/example_avx-64.inc"
    INCLUDE_IF_SSE2_64BIT "example/example_sse2-64.inc"
    
    INCLUDE_IF_SSE2_32BIT "example/example_sse2-32.inc"
    INCLUDE_IF_X86_32BIT "example/example_x86-32.inc"

with an assembler implementation, [example_x86-32.inc](src/example/example_x86-32.inc), looking like:

    SECTION_TEXT
    
    GLOBAL example_x86
    FN example_x86
        movl 4(%esp), %edx
        movl 8(%esp), %ecx
        xorl %eax, %eax
        andl %ecx, %ecx
        jz Lexample_x86_done
    
    Lexample_x86_loop:
        addl 0(%edx), %eax
        addl $4, %edx
        decl %ecx
        jnz Lexample_x86_loop
    
    Lexample_x86_done:
        ret
    FN_END example_x86

Suggested function naming is name_extension, e.g. `example_x86`, `aes_avx2`, `curve25519_sse2`, etc.

#### C ####

The C code "glue" is [impl.c](src/example/impl.c), which starts by declaring an implementation struct holding the cpu flags for the implementation and its methods. Note that an optimized implementation may have multiple methods, such as a signing function which would have separate functions for key generation, signing, and verifying:

    typedef struct example_impl_t {
        uint32_t cpu_flags;
        const char *desc;
        int32_t (*example)(const int32_t *arr, size_t count);
    } example_impl_t;

The available implementations are then detected and declared based on the defines from `config.h` (which is included by [driver.h](driver/driver.h)):

    /* declare the prototypes of the provided functions */
    #define EXAMPLE_DECLARE(ext) \
        extern int32_t example_##ext(const int32_t *arr, size_t count);

    #if defined(ARCH_X86)
        /* 32 bit only implementations */
        #if defined(CPU_32BITS)
            EXAMPLE_DECLARE(x86)
            #define EXAMPLE_X86 {CPUID_X86, "x86", example_x86}
        #endif

        /* 64 bit only implementations */
        #if defined(CPU_64BITS)
            #if defined(HAVE_AVX)
                EXAMPLE_DECLARE(avx)
                #define EXAMPLE_AVX {CPUID_AVX, "avx", example_avx}
            #endif
        #endif

        /* both 32 & 64 bit implementations */
        #if defined(HAVE_SSE2)
            EXAMPLE_DECLARE(sse2)
            #define EXAMPLE_SSE2  {CPUID_SSE2, "sse2", example_sse2}
        #endif
    #endif

The "generic" version, which should be maximally portable C, is then declared. Its purpose is to both be a "reference" implementation, and to allow the function to run on platforms where you do not yet have optimized assembler. It is obviously up to you, but speed should most likely be second to portability and readability:

    /* the "always runs" version */
    #define EXAMPLE_GENERIC {CPUID_GENERIC, "generic", example_generic}
    #include "example_generic.h"

Finally, the full list of optimized functions is declared, going in order of most optimized to least, with the generic implementation at the bottom. The chosen optimized implementation is declared as well; this could be an integer index in to the implementation list, a pointer to an implemention, etc. I've chosen a static struct to minimize the pointer loads, but that is a minor detail.

    /* list implemenations from most optimized to least, with generic as the last entry */
    static const example_impl_t example_list[] = {
        #if defined(EXAMPLE_AVX)
            EXAMPLE_AVX,
        #endif
        #if defined(EXAMPLE_SSE2)
            EXAMPLE_SSE2,
        #endif
        #if defined(EXAMPLE_X86)
            EXAMPLE_X86,
        #endif
        EXAMPLE_GENERIC
    };
    
    static example_impl_t example_opt = {0,0,0};

To select the best implementation for the current CPU, two functions are needed: one to test an implementation, which is not called directly:

    /* test an implementation */
    static int
    example_test(const void *impl) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        int32_t arr[50], i;
        for (i = 0; i < 50; i++)
            arr[i] = i;
        return (example_impl->example(arr, 50) == 1225) ? 0 : 1;
    }
    
and one to call `cpu_select` and set up `example_opt`:

    /* choose the best implemenation for the current cpu */
    int
    example_init(void) {
        const void *opt = cpu_select(example_list, sizeof(example_impl_t), example_test);
        if (opt) {
            example_opt = *(const example_impl_t *)opt;
            return 0;
        } else {
            return 1;
        }
    }

The user facing function exposing the chosen optimized implementation is the last piece:

    /* call the optimized implementation */
    int
    example(const int32_t *arr, size_t count) {
        return example_opt.example(arr, count);
    }

## BUILDING ##

### CONFIGURING ###

    ./configure [options]
 
#### Help ####
 * `-h`, `--help`: Prints help
    
#### CONFIGURATION OPTIONS ####
 * `--debug`: Builds with no optimization and debugging symbols enbaled
 * `--disable-as`: Do not use external assembly
 * `--force-32bits`: Build for 32bits regardless of underlying system
 * `--pic`: Pass `-fPIC` to the compiler. If you are using `LOAD_VAR_PIC` properly, all assembler will be PIC safe by default
 * `--strict`: Use strict compiler flags for C
 * `--yasm`: Use Yasm to compile external asm

#### SHELL VARIABLES USED ####

 * `CC`: The C compiler to use [default: gcc]
 * `AR`: The archiver to use [default: ar]
 * `RANLIB`: The indexer to use [default: ranlib]
 * `STRIP`: The symbol stripper to use [default: strip]
 * `INSTALL`: The installer to use [default: install]
 * `CFLAGS`: Addition C flags to pass to the compiler

Well some aren't used yet, but you know, for the future.

### COMPILING ###

    make
    ./example

### VISUAL STUDIO ###

Rename `config/config.h.visualstudio` to `config/config.h`, download at least [Yasm 1.2](http://yasm.tortall.net/) and [follow the Yasm integration steps](http://yasm.tortall.net/Download.html) for your version of Visual Studio. Set the parser for each `.S` file to "Gas"; manual flags for Yasm are `-r nasm -p gas -f win[32,64]`. Additionally add `driver;src;` to the include path for C/C++ and Yasm.

## FUZZING ##

Abstract fuzzing is provided through [fuzz.h](driver/fuzz.h) and [fuzz.c](driver/fuzz.c). Extending an example to support fuzzing requires 3 simple functions to be passed to the fuzzer, which then handles the work of selecting available implementations, producing random numbers, collecting the output, comparing the output, detecting mismatches, and displaying the progress a simple counter so you can tell progress is being made.

### EXAMPLE ###

First, the fuzzer must be intialized, which right now is just initializing its random number generator, a non-threadsafe, insecure, portable implementation of `Chacha/8`. There are two options for this:

* `void fuzz_init(void)`: initializes the rng randomly through `/dev/urandom` or `CryptGenRandom`.

* `void fuzz_init_deterministic(void)`: sets the rng state to all zeros, ensuring the same data is produced every run.



The fuzzer then needs 3 functions passed to it:

#### 1. SETUP ####

`typedef void (*impl_fuzz_setup)(uint8_t *in, size_t *in_bytes, size_t *out_bytes);`

`fuzz_setup` takes a pointer for the input data to be generated in to (`in`), a pointer which stores the amount of input bytes generated (`in_bytes`), and a pointer which stores the expected output bytes based on the input (`out_bytes`). The fuzzer internally uses a buffer of `16384+1024` bytes for generated input any additional input data needed, and `16384+1024` bytes for each implementation to generate output.

    /* setup a fuzz pass, generate random data for the input, and tell the 
       fuzzer how much output to expect */
    static void
    example_fuzz_setup(uint8_t *in, size_t *in_bytes, size_t *out_bytes) {
        uint8_t *in_start = in;
        size_t arr_len;
        fuzz_get_bytes(&arr_len, sizeof(arr_len));
    
        /* use an array size of 0->16384 bytes / sizeof(int32_t) = number 
           of ints to count up */
        arr_len = (arr_len % 16384) / sizeof(int32_t);
        memcpy(in, &arr_len, sizeof(arr_len));
        in += sizeof(arr_len);
    
        /* generate the input ints! */
        fuzz_get_bytes(in, arr_len * sizeof(int32_t));
        in += arr_len * sizeof(int32_t);
    
        /* amount of input that will be used */
        *in_bytes = in - in_start;
    
        /* amount of output each implementation will produce */
        *out_bytes = sizeof(int32_t);
    }

#### 2. FUZZ ####

Next is the function which processes the input and generates output for each implementation. For example, a fuzzing function could use the input as a private key, generate a public key and store it, sign a block of data and store the signature, and finally verify the signature and store the result.

`typedef size_t (*impl_fuzz)(const void *impl, const uint8_t *in, uint8_t *out);`

    /* process the input with the given implementation and write it to the 
       output */
    static size_t
    example_fuzz(const void *impl, const uint8_t *in, uint8_t *out) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        uint8_t *out_start = out;
        size_t int_count;
        int32_t sum;
    
        /* read count */
        memcpy(&int_count, in, sizeof(int_count));
        in += sizeof(int_count);
    
        /* sum the array */
        sum = example_impl->example((const int32_t *)in, int_count);
    
        /* store the result */
        memcpy(out, &sum, sizeof(sum));
        out += sizeof(sum);
    
        /* return bytes written */
        return (out - out_start);
    }

#### 3. PRINT ####

The fuzzer provides a function to print raw data in hexadecimal format, optionally diff'd against a base buffer:

`void fuzz_print_bytes(const char *desc, const uint8_t *bytes, const uint8_t *base, size_t len)`

`desc` is a simple description that will be printed before the data

`bytes` is the raw data to print.

`base` is pointer to the 'known good' data. If `base == bytes` (the pointer value, not the data), the raw data will be printed. If `base != bytes` then `bytes[i] ^ base[i]` will be printed. If the values are equal, `____` is printed to aid in ignoring equal values.

`len` is the number of bytes to print.


When a mismatch occurs between implementations, the fuzzer stops and calls the user defined print function for each implementation with a pointer to the input data, the output data for that implemenattion, and the output data for the "generic" implementation.

`typedef void (*impl_fuzz_print)(const void *impl, const uint8_t *in, const uint8_t *out, const uint8_t *generic_out);`

    /* print the output for the given implementation, and xor it against 
       generic_out if needed */
    static void
    example_fuzz_print(const void *impl, const uint8_t *in, const uint8_t *out, const uint8_t *generic_out) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        if (out == generic_out) {
            size_t int_count;
            /* this is the generic data, print the input first */
            printf("INPUT\n\n");
    
            /* input length */
            memcpy(&int_count, in, sizeof(int_count));
            in += sizeof(int_count);
            printf("length: %u\n", (uint32_t)int_count);

            /* dump data */
            example_fuzz_print_bytes("data", in, in, int_count * sizeof(int32_t));

            /* switch to output! */
            printf("OUTPUT\n\n");
        }
        printf("IMPLEMENTATION:%s\n", example_impl->desc);
        example_fuzz_print_bytes("sum", out, generic_out, sizeof(int32_t));
        out += sizeof(int32_t);
        generic_out += sizeof(int32_t);
    }

#### PUTTING IT ALL TOGETHER ####

    /* run the fuzzer on example */
    void
    example_fuzzer(void) {
        fuzz_init();
        fuzz(example_list, sizeof(example_impl_t), example_fuzz_setup, example_fuzz, example_fuzz_print);
    }

That's it! Now build and run with:

    make fuzz
    ./example-fuzz

and let it run for as long as you like (except not with the example fuzzer, that would be wasting electricity).

# LICENSE #

Public Domain, or MIT
