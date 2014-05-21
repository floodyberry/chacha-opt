static cycles_t
cpucycles_impl(void) {
	static cycles_t cludge = 0;
	return cludge++;
}
