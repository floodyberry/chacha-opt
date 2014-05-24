typedef cycles_t (*cpucycles_x86_fn)(void);

extern cycles_t cpucycles_x86(void);
static cycles_t cpucycles_select(void);

static cpucycles_x86_fn cpucycles_impl = cpucycles_select;

#if defined(HAVE_GETTIMEOFDAY)
#include <sys/time.h>

static cycles_t
cpucycles_x86_cludge(void) {
	struct timeval t;
	gettimeofday(&t, NULL);
	return ((cycles_t)t.tv_usec * 1000000) + (cycles_t)t.tv_sec;
}
#else
/* what can a 386/486 use for this otherwise? */
static cycles_t
cpucycles_x86_cludge(void) {
	static cycles_t cludge = 0;
	return cludge++;
}
#endif

static cycles_t
cpucycles_select(void) {
	cpucycles_impl = (cpuid() & CPUID_RDTSC) ? cpucycles_x86 : cpucycles_x86_cludge;
	return cpucycles_impl();
}
