#include <stdio.h>
#include <string.h>
#include "chacha.h"

/* define get_ticks */
static uint64_t
get_ticks(void) {
	uint32_t lo, hi;
	__asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)lo | ((uint64_t)hi << 32));
}

#include "bench-base.h"

int main() {
	chacha_bench();
	return 0;
}
