typedef cycles_t (*cpucycles_x86_fn)(void);

extern cycles_t cpucycles_x86(void);
static cycles_t cpucycles_select(void);

static cpucycles_x86_fn cpucycles_impl = cpucycles_select;

static cycles_t
cpucycles_x86_cludge(void) {
	/* what can a 386/486 use for this? */
	static cycles_t cludge = 0;
	return cludge++;
}

static cycles_t
cpucycles_select(void) {
	cpucycles_impl = (cpuid() & CPUID_RDTSC) ? cpucycles_x86 : cpucycles_x86_cludge;
	return cpucycles_impl();
}
