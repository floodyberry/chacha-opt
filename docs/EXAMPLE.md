An [example](extensions/example/) is provided demonstrating how to implement, select, and call an optimized function. The example algorithm being implemented is summing up the ~~~little-endian~~~ 32-bit signed integers in a given array and returning the result. 

### PUTTING IT TOGETHER ###

#### ASSEMBLER ####

[example.S](extensions/example/example.S) includes the bootstrap header and aggregates all the implementations in to a single file:

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

with an assembler implementation, [example_x86-32.inc](extensions/example/example_x86-32.inc), looking (something like) like:

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

The C code "glue" is [impl.c](extensions/example/impl.c), which starts by declaring an implementation struct holding the cpu flags for the implementation and its methods. Note that an optimized implementation may have multiple methods, such as a signing function which would have separate functions for key generation, signing, and verifying:

    typedef struct example_impl_t {
        uint32_t cpu_flags;
        const char *desc;
        int32_t (*example)(const int32_t *arr, size_t count);
    } example_impl_t;

The available implementations are then detected and declared based on the defines from `asmopt.h`:

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
    example_test_impl(const void *impl) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        int32_t arr[50], i, sum;
        int ret = 0;
        
        for (i = 0, sum = 0; i < 50; i++) {
            arr[i] = i;
            sum += i;
        }
        for (i = 0; i <= 50; i++) {
            ret |= (example_impl->example(arr, 50 - i) == sum) ? 0 : 1;
            sum -= (50 - i - 1);
        }
        return ret;
    }

and one to call `cpu_select` and set up `example_opt`:

    /* choose the best implemenation for the current cpu */
    int
    example_init(void) {
        const void *opt = cpu_select(example_list, sizeof(example_impl_t), example_test_impl);
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

## REALISTIC EXAMPLES ##

### secure_zero ###

    #include "secure_zero.h"
    
    void secure_zero(uint8_t *p, size_t len);

Securely erases an array, removing the chance that the compiler could optimize the call out


### secure_compare ###

    #include "secure_compare.h"

    int secure_compare8(const uint8_t *x, const uint8_t *y);
    int secure_compare16(const uint8_t *x, const uint8_t *y);
    int secure_compare32(const uint8_t *x, const uint8_t *y);

Compares two 8/16/32 byte arrays in constant time without leaking timing information.
