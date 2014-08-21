# ABOUT #

These are specially optimized block functions for [ChaCha](http://cr.yp.to/chacha.html), 
a stream cipher with a 256 bit key and a 64 bit nonce.

HChaCha is also implemented, which is used to build XChaCha, a variant which extends the 
nonce from 64 bits to 192 bits. See [Extending the Salsa20 nonce](http://cr.yp.to/snuffle/xsalsa-20110204.pdf). 

All assembler is PIC safe.

When calling the one-shot versions `chacha_impl`, `xchacha_impl`, `chacha`, and `xchacha`, input/output are 
assumed to be word aligned. Incremental support has no alignment requirements, but will obviously slow down if
non word-aligned pointers are passed.

If you encrypt anything without using a MAC (HMAC, Poly1305, etc), you will be found, and made fun of.

# CALLING #

## ChaCha ##

`void chacha_blocks_impl(chacha_blocks_state *state, const uint8_t *in, uint8_t *out, size_t bytes);`

**state** is a word aligned pointer to a struct in the form of

    chacha_blocks_state {
        uint8_t s[48];
        size_t rounds;
    }

where **s** is words 4-15 of the ChaCha state in little endian, and **rounds** is an even number >= 2. 
Little endian systems are free to treat **s** as `uint32_t s[12]`.

**in** is a pointer to the input data which is XOR'd against the resulting ChaCha stream. If **in** is 
`NULL`, the ChaCha stream will be written directly to **out**.

**out** is a pointer where the data will be written to. It may be the same as **in** if you want 
to process data in place.

**bytes** is the number of bytes to process. It is not required to be a multiple of the ChaCha block 
size (64 bytes), but the internal block counter will be incremented by ((bytes + 63) / 64), so it should only 
be a non-multiple of 64 if there is no more data to process.

`void chacha_impl(const chacha_key *key, const chacha_iv *iv, const uint8_t *in, uint8_t *out, size_t inlen, size_t rounds);`

Encrypts `inlen` bytes from `in` to `out, using `key`, `iv`, and `rounds`.

## HChaCha ##

`void hchacha_impl(const uint8_t key[32], const uint8_t iv[16], uint8_t out[32], size_t rounds);`

**key** is a pointer to 32 unsigned bytes. 

**iv** is a pointer to 16 unsigned bytes. 

**out** is a pointer where the 32 byte output of HChaCha(key, iv, rounds) will be stored.

**rounds** is the number of rounds to use and must be >= 2.

`void xchacha_impl(const chacha_key *key, const chacha_iv24 *iv, const uint8_t *in, uint8_t *out, size_t inlen, size_t rounds);`

Encrypts `inlen` bytes from `in` to `out, using `key`, `iv`, and `rounds` using XChacha/rounds.

## chacha.c ##

[chacha.c](chacha.c) and [chacha.h](chacha.h) is a ChaCha implementation which handles all the high level functions 
(incremental state, input alignment, validity testing) and calls out to `chacha_blocks_impl` and `hchacha_impl`. 
Compiling with `-DCHACHA_IMPL=ver` replaces `chacha_blocks_impl` with `chacha_blocks_ver`, `hchacha_impl` with `hchacha_ver`, etc.,
which allows testing of specific functions. This will change when I get cpu-dispatching included.

### Functions ###

`void chacha_init(chacha_state *S, const chacha_key *key, const chacha_iv *iv, size_t rounds);`

Initialize the chacha_state with `key` and `iv`, and `rounds`, and sets the internal block counter to 0.

`size_t chacha_update(chacha_state *S, const uint8_t *in, uint8_t *out, size_t inlen);`

Generates/crypts up to `inlen + 63` bytes depending on how many bytes are in the internal buffer, and returns the number
of encrypted bytes written to `out`.

`uint64_t chacha_get_counter(chacha_state *S);`

Returns the value of the internal block counter, which is the number of bytes processed / 64.

`void chacha_set_counter(chacha_state *S, uint64_t counter);`

Sets the value of the internal block counter.

`size_t chacha_final(chacha_state *S, uint8_t *out);`

Generates/crypts any leftover data in the state to `out`, returns the number of bytes written.

`void chacha(const chacha_key *key, const chacha_iv *iv, const uint8_t *in, uint8_t *out, size_t inlen, size_t rounds);`

Encrypts `inlen` bytes from `in` to `out, using `key`, `iv`, and `rounds`.

`void xchacha(const chacha_key *key, const chacha_iv24 *iv, const uint8_t *in, uint8_t *out, size_t inlen, size_t rounds);`

Encrypts `inlen` bytes from `in` to `out, using `key`, `iv`, and `rounds` using XChacha/rounds.

### Examples ###

#### Encrypting a buffer, single call: 

    const size_t rounds = 20;
    chacha_key key = {..};
    chacha_iv iv = {..};
    uint8_t in[100] = {..}, out[100];
    
    chacha(&key, &iv, in, out, 100, rounds);

#### Encrypting Incrementally

Encrypting incrementally, i.e. with multiple calls to collect/write data. Note 
that passing in data to be encrypted will not always result in data being written out. The 
implementation collects data until there is at least 1 block (64 bytes) of data available.

    const size_t rounds = 20;
    chacha_state S;
    chacha_key key = {{..}};
    chacha_iv iv = {{..}};
    uint8_t in[100] = {..}, out[100], *out_pointer = out;
    size_t i, bytes_written;
    
    chacha_init(&S, &key, &iv, rounds);

    /* add one byte at a time, extremely inefficient */
    for (i = 0; i < 100; i++) {
        bytes_written = chacha_update(&S, in + i, out_pointer, 1);
        out_pointer += bytes_written;
    }
    bytes_written = chacha_final(&S, out_pointer);

#### XChaCha

XChaCha is called exactly like ChaCha, except `xchacha` and `xchacha_init` are called instead,
and the iv is `chacha_iv24` (192 bits) instead of `chacha_iv` (64 bits). 

`chacha_key`, `chacha_iv`, and `chacha_iv24` may be accessed directly through `.b[i]`, e.g. `key.b[0] = 0xff;`.

# VERSIONS #

~~XOP~~, and any other architecture, is not included yet because I do not have access to any of those machines 
to test with.

x86-64, SSE2-32, and SSE3-32 versions are minorly modified from DJB's public domain implementations.

## Reference ##

* Generic: [chacha\_blocks\_ref](chacha_blocks_ref.c)

## x86 (32 bit) ##

* 386 compatible: [chacha\_blocks\_x86](chacha_blocks_x86-32.S)
* SSE2: [chacha\_blocks\_sse2](chacha_blocks_sse2-32.S)
* SSSE3: [chacha\_blocks\_ssse3](chacha_blocks_ssse3-32.S)
* AVX: [chacha\_blocks\_avx](chacha_blocks_avx-32.S)
* XOP: [chacha\_blocks\_xop](chacha_blocks_xop-32.S)
* AVX2: [chacha\_blocks\_avx2](chacha_blocks_avx2-32.S)

## x86-64 ##

* x86-64 compatible: [chacha\_blocks\_x86](chacha_blocks_x86-64.S)
* SSE2: [chacha\_blocks\_sse2](chacha_blocks_sse2-64.S)
* SSSE3: [chacha\_blocks\_ssse3](chacha_blocks_ssse3-64.S)
* AVX: [chacha\_blocks\_avx](chacha_blocks_avx-64.S)
* XOP: [chacha\_blocks\_xop](chacha_blocks_xop-64.S)
* AVX2: [chacha\_blocks\_avx2](chacha_blocks_avx2-64.S)

x86-64 will almost always be slower than SSE2, but on some older AMDs it may be faster

# TESTING #

Run `./test.sh [ref,x86,sse2,ssse3,avx,xop,avx2] [32,64]` to test that the specified version
is producing the correct results. Features tested:

## Implementation specific

* Partial block generation
* Single block generation
* Multi block generation
* Counter handling when the 32-bit low half overflows to the upper half
* Streaming and XOR modes

## chacha.c API

* Incremental encryption
* Input/Output alignment

# BENCHMARKS #

Run `./bench-x86.sh [ref,x86,sse2,ssse3,avx,xop,avx2] [32,64]` to bench the specified version. 
The benchmark will not run if the version does not pass the validity tests.

When storing the ChaCha stream directly in to the output buffer instead of XOR'ing it with the input 
(i.e. setting **in** to NULL), performance will be 0.01-0.05 cycles/byte faster.

XChaCha/r has the same performance as ChaCha/r plus the cost of one HChaCha/r call.

## [E5200](http://ark.intel.com/products/37212/) ##

### ChaCha ###

<table>
<thead><tr><th>Impl.</th><th>1 byte</th><th>8</th><th>12</th><th>20</th><th>576 bytes</th><th>8</th><th>12</th><th>20</th><th>8192 bytes</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>SSSE3-64  </td> <td></td><td> 237</td><td> 300</td><td> 437</td> <td></td><td>  1.71</td><td>  2.23</td><td>  3.30</td> <td></td><td>  1.46</td><td>  1.90</td><td>  2.82</td> </tr>
<tr> <td>SSE2-64   </td> <td></td><td> 262</td><td> 337</td><td> 500</td> <td></td><td>  1.98</td><td>  2.65</td><td>  3.97</td> <td></td><td>  1.68</td><td>  2.29</td><td>  3.42</td> </tr>
<tr> <td>SSSE3-32  </td> <td></td><td> 287</td><td> 350</td><td> 487</td> <td></td><td>  2.04</td><td>  2.69</td><td>  3.99</td> <td></td><td>  1.72</td><td>  2.37</td><td>  3.59</td> </tr>
<tr> <td>SSE2-32   </td> <td></td><td> 312</td><td> 400</td><td> 562</td> <td></td><td>  2.43</td><td>  3.26</td><td>  4.95</td> <td></td><td>  2.12</td><td>  2.90</td><td>  4.52</td> </tr>
</tbody>
</table>

### HChaCha ###

<table>
<thead><tr><th>Impl.</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>SSSE3-64  </td> <td> 162</td><td> 237</td><td> 362</td> </tr>
<tr> <td>SSSE3-32  </td> <td> 175</td><td> 250</td><td> 375</td> </tr>
<tr> <td>SSE2-64   </td> <td> 200</td><td> 275</td><td> 450</td> </tr>
<tr> <td>SSE2-32   </td> <td> 200</td><td> 275</td><td> 450</td> </tr>
</tbody>
</table>

## [E3-1270](http://ark.intel.com/products/52276/) ##

### ChaCha ###

<table>
<thead><tr><th>Impl.</th><th>1 byte</th><th>8</th><th>12</th><th>20</th><th>576 bytes</th><th>8</th><th>12</th><th>20</th><th>8192 bytes</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>AVX-64    </td> <td></td><td> 176</td><td> 240</td><td> 364</td> <td></td><td>  1.22</td><td>  1.68</td><td>  2.64</td> <td></td><td>  1.04</td><td>  1.46</td><td>  2.29</td> </tr>
<tr> <td>SSSE3-64  </td> <td></td><td> 180</td><td> 248</td><td> 384</td> <td></td><td>  1.35</td><td>  1.88</td><td>  2.94</td> <td></td><td>  1.18</td><td>  1.65</td><td>  2.59</td> </tr>
<tr> <td>AVX-32    </td> <td></td><td> 184</td><td> 248</td><td> 380</td> <td></td><td>  1.50</td><td>  2.03</td><td>  3.10</td> <td></td><td>  1.24</td><td>  1.72</td><td>  2.68</td> </tr>
<tr> <td>SSSE3-32  </td> <td></td><td> 228</td><td> 292</td><td> 428</td> <td></td><td>  1.84</td><td>  2.47</td><td>  3.74</td> <td></td><td>  1.65</td><td>  2.23</td><td>  3.41</td> </tr>
</tbody>
</table>

### HChaCha ###

<table>
<thead><tr><th>Impl.</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>AVX-64    </td> <td> 116</td><td> 180</td><td> 308</td> </tr>
<tr> <td>AVX-32    </td> <td> 128</td><td> 192</td><td> 320</td> </tr>
<tr> <td>SSSE3-64  </td> <td> 128</td><td> 192</td><td> 328</td> </tr>
<tr> <td>SSSE3-32  </td> <td> 136</td><td> 204</td><td> 336</td> </tr>
</tbody>
</table>

## [i7-4770K](http://ark.intel.com/products/75123) ##

Timings are with Turbo Boost and Hyperthreading, so their accuracy is not concrete. 
For reference, OpenSSL and Crypto++ give ~0.8cpb for AES-128-CTR and ~1.1cpb for AES-256-CTR, 
and ~7.4cpb for SHA-512.

### ChaCha ###

<table>
<thead><tr><th>Impl.</th><th>1 byte</th><th>8</th><th>12</th><th>20</th><th>576 bytes</th><th>8</th><th>12</th><th>20</th><th>8192 bytes</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>AVX2-64   </td> <td></td><td> 146</td><td> 194</td><td> 313</td> <td></td><td>  0.68</td><td>  0.97</td><td>  1.48</td> <td></td><td>  0.52</td><td>  0.71</td><td>  1.08</td> </tr>
<tr> <td>AVX2-32   </td> <td></td><td> 170</td><td> 218</td><td> 337</td> <td></td><td>  0.83</td><td>  1.11</td><td>  1.66</td> <td></td><td>  0.62</td><td>  0.83</td><td>  1.24</td> </tr>
<tr> <td>AVX-64    </td> <td></td><td> 146</td><td> 194</td><td> 316</td> <td></td><td>  1.06</td><td>  1.50</td><td>  2.33</td> <td></td><td>  0.94</td><td>  1.32</td><td>  2.05</td> </tr>
<tr> <td>AVX-32    </td> <td></td><td> 158</td><td> 206</td><td> 328</td> <td></td><td>  1.32</td><td>  1.82</td><td>  2.81</td> <td></td><td>  1.12</td><td>  1.57</td><td>  2.47</td> </tr>
</tbody>
</table>

### HChaCha ###

(these are all literally the same version, timing differences are noise)

<table>
<thead><tr><th>Impl.</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>AVX2-64   </td> <td>  81</td><td> 155</td><td> 251</td> </tr>
<tr> <td>AVX2-32   </td> <td>  87</td><td> 155</td><td> 254</td> </tr>
<tr> <td>AVX-64    </td> <td>  87</td><td> 155</td><td> 274</td> </tr>
<tr> <td>AVX-32    </td> <td>  87</td><td> 152</td><td> 251</td> </tr>
</tbody>
</table>

## AMD FX-8120 ##

Timings are with Turbo on, so accuracy is not concrete. I'm not sure how to adjust for it either, 
and depending on clock speed (3.1ghz vs 4.0ghz), OpenSSL gives between 0.73cpb - 0.94cpb for AES-128-CTR, 
1.03cpb - 1.33cpb for AES-256-CTR, and 10.96cpb - 14.1cpb for SHA-512.

### ChaCha ###

<table>
<thead><tr><th>Impl.</th><th>1 byte</th><th>8</th><th>12</th><th>20</th><th>576 bytes</th><th>8</th><th>12</th><th>20</th><th>8192 bytes</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>XOP-64    </td> <td></td><td> 194</td><td> 269</td><td> 418</td> <td></td><td>  1.09</td><td>  1.47</td><td>  2.25</td> <td></td><td>  0.93</td><td>  1.22</td><td>  1.80</td> </tr>
<tr> <td>AVX-64    </td> <td></td><td> 245</td><td> 344</td><td> 544</td> <td></td><td>  1.41</td><td>  1.97</td><td>  3.14</td> <td></td><td>  1.20</td><td>  1.63</td><td>  2.51</td> </tr>
<tr> <td>XOP-32    </td> <td></td><td> 247</td><td> 322</td><td> 471</td> <td></td><td>  1.44</td><td>  1.96</td><td>  3.01</td> <td></td><td>  1.26</td><td>  1.70</td><td>  2.59</td> </tr>
<tr> <td>AVX-32    </td> <td></td><td> 276</td><td> 375</td><td> 573</td> <td></td><td>  1.88</td><td>  2.53</td><td>  3.78</td> <td></td><td>  1.62</td><td>  2.16</td><td>  3.23</td> </tr>
</tbody>
</table>

### HChaCha ###

<table>
<thead><tr><th>Impl.</th><th>8</th><th>12</th><th>20</th></tr></thead>
<tbody>
<tr> <td>XOP-64    </td> <td>  84</td><td> 160</td><td> 309</td> </tr>
<tr> <td>XOP-32    </td> <td>  91</td><td> 165</td><td> 318</td> </tr>
<tr> <td>AVX-64    </td> <td> 144</td><td> 243</td><td> 441</td> </tr>
<tr> <td>AVX-32    </td> <td> 144</td><td> 237</td><td> 441</td> </tr>
</tbody>
</table>


# LICENSE #

Public Domain, or MIT