static void
secure_zero_generic(uint8_t *p, size_t len) {
	volatile uint8_t *vp = (volatile uint8_t *)p;
	size_t i;
	for (i = 0; i < len; i++)
		vp[i] = 0;
}

