static int32_t
example_generic(const int32_t *arr, size_t count) {
	size_t i;
	int32_t sum = 0;
	for (i = 0; i < count; i++)
		sum += arr[i];
	return sum;
}
