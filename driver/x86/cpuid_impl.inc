extern uint32_t cpuid_x86(void);

static uint32_t
cpuid_impl(void) {
	return cpuid_x86();
}

