static unsigned char
example_generic(const unsigned char *arr, size_t count) {
	size_t i;
	unsigned char sum = 0;
	for (i = 0; i < count; i++)
		sum += arr[i];
	return (sum % 256);
}
