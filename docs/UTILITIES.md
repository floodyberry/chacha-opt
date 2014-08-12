Extension agnostic utilities are provided for

* Fuzzing
* Benchmarking

## FUZZING ##

Abstract fuzzing is provided through [fuzz.h](framework/include/fuzz.h) and [fuzz.c](framework/fuzz.c). Extending an implementation to support fuzzing requires 1 simple function to be passed to the fuzzer, which then handles the work of selecting available implementations, producing random numbers, producing the input, collecting the output, comparing the output, detecting and displaying mismatches, and displaying the progress using a simple counter so you can tell progress is being made.

### EXAMPLE ###

First, the fuzzer must be initialized, which right now is just initializing its random number generator, a non-threadsafe, insecure, portable implementation of `Chacha/8`. There are two options for this:

* `void fuzz_init(void)`: initializes the rng randomly through `/dev/urandom` or `CryptGenRandom`.

* `void fuzz_init_deterministic(void)`: sets the rng state to all zeros, ensuring the same data is produced every run.

The fuzzer then needs 3 things passed to it:

#### 1+2. INPUTS + OUTPUTS ####

An extension must tell the fuzzer what inputs it will need, and the output it will produce based on those inputs. The inputs and outputs are passed as a list of pre-defined types and sizes:

    typedef enum {
        FUZZ_DONE,
        FUZZ_ARRAY,
        FUZZ_RANDOM_LENGTH_ARRAY0,
        FUZZ_RANDOM_LENGTH_ARRAY1,
        FUZZ_RANDOM_LENGTH_ARRAY2,
        FUZZ_RANDOM_LENGTH_ARRAY3
    } fuzz_type_t;
    
    typedef struct fuzz_variable_t {
        const char *desc;
        fuzz_type_t type;
        size_t size;
    } fuzz_variable_t;

* `FUZZ_DONE` indicates the end of the list, `size` is ignored
* `FUZZ_ARRAY` specifies an array of `size` 8 bit unsigned integers
* `FUZZ_RANDOM_LENGTH_ARRAY[0-3]` specifies an array of [0-`size`) 8 bit unsigned integers, where the size is determined randomly at run-time. This can be specified for output as well as input, and the size in the output must match the input size. How to determine the sizes in the next section.

The declaration for our example extension is:

    static const fuzz_variable_t fuzz_inputs[] = {
        {"input", FUZZ_RANDOM_LENGTH_ARRAY0, 16384},
        {0, FUZZ_DONE, 0}
    };
    
    static const fuzz_variable_t fuzz_outputs[] = {
        {"sum", FUZZ_ARRAY, 1},
        {0, FUZZ_DONE, 0}
    };

Input is a random length array up to 16384 bytes.

The output is 1 8 bit unsigned integer.

#### 3. FUZZ FUNCTION ####

Last is the function which processes the input and generates output for each implementation. 

`typedef void (*impl_fuzz)(const void *impl, const unsigned char *in, const size_t *random_sizes, unsigned char *out);`

    /* process the input with the given implementation and write it to the output */
    static void
    example_fuzz_impl(const void *impl, const unsigned char *in, const size_t *random_sizes, unsigned char *out) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        size_t int_count;
        unsigned char sum;
        
        /* sum the array */
        sum = example_impl->example(in, random_sizes[0]);
        
        /* store the result */
        memcpy(out, &sum, sizeof(sum));
        out += sizeof(sum);
    }

`in` is the input data, in the format specified by your inputs array.

`random_sizes` is an array which holds the randomly generated size for each specified `FUZZ_RANDOM_LENGTH_ARRAY[0-3]`.

`out` is the output data, in the format specified by your outputs array.

#### PUTTING IT ALL TOGETHER ####

    /* run the fuzzer on example */
    void
    example_fuzz(void) {
        fuzz_init();
        fuzz(example_list, sizeof(example_impl_t), fuzz_inputs, fuzz_outputs, example_fuzz_impl);
    }

That's it! Now build and run with:

    make util
    ./example-util [example,secure_compare,secure_zero] fuzz

and let it run for as long as you like (except not with any of the examples, that would be wasting electricity). If a mismatch occurs between any implementation, the fuzzer will stop and print

* The input generated for the run
* The output of the general/reference implementation (which is the last implementation in the list)
* The diff of the output of each following implementation against the general/reference implementation

## BENCHING ##

Basic benchmarking is now in! Optimizing implementations is pointless if you cannot tell if they are actually optimized or not, e.g. I had to _actually_ optimize the assembler examples here because they turned out to be slower than the "generic" version!

    typedef void (*impl_bench)(const void *impl);

    void bench(const void *impls, size_t impl_size, impl_test test_fn, impl_bench bench_fn, size_t units_count, const char *units_desc);

`bench` calls `test_fn` for each available implementation, and if that succeeds, it proceeds with the benchmark.

** NEW ** It is no longer necessary to specify how many trials to use. Internally, the benchmark code computes the smallest measurable duration between two consecutive calls to the timing function, and approximately how many `cycles_t` are accumulated during 1 second.. It then determines how many calls to `bench_fn` will last at least 25 times the smallest measurable duration, and stores it at the batch size. Finally, it computes how many times to call the batch size to last approximately 1 second. No more going crazy trying to figure out how to time on significantly slower machines or trying to batch things up to cover the variability of timers!

The basic strategy for benchmarking is to call bench once per specific combination of parameters and methods, e.g. call bench once for encrypting 16 bytes, once for 256, or once for signing a message, once for verifying a message, etc. `units_count` is the number of raw units being operated on, e.g. `32` for `secure_compare32`. `units_desc` is the type of raw unit being operated on, e.g. "byte", "signature", "keypair generation".

    unsigned char *bench_get_buffer(void);

`bench_get_buffer` returns a 64 byte aligned 32768 byte scratch buffer the benchmarked function can do whatever it likes with. 

### EXAMPLE ###

    static int32_t *bench_arr = NULL;
    static size_t bench_len = 0;

    static void
    example_bench_impl(const void *impl) {
        const example_impl_t *example_impl = (const example_impl_t *)impl;
        example_impl->example(bench_arr, bench_len);
    }

    void
    example_bench(void) {
        static const size_t lengths[] = {16, 256, 4096, 0};
        size_t i;
        bench_arr = (int32_t *)bench_get_buffer();
        memset(bench_arr, 0xf5, 32768);
        for (i = 0; lengths[i]; i++) {
            bench_len = lengths[i];
            bench(example_list, sizeof(example_impl_t), example_bench_impl, bench_len, "byte");
        }
    }

Static variables are used to keep track of what the current settings are. The bench scratch buffer is used for the input data, although its contents are not important. One call is made to `bench` for each length to benchmark.

#### BUILDING ####

    make util
    ./example-util [example,secure_compare,secure_zero] bench
