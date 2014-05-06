static int
example_generic(const int *arr, size_t count) {
	size_t i;
	int sum = 0;
	for (i = 0; i < count; i++)
		sum += arr[i];
	return sum;
}
