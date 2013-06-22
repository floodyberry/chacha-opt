#include <stdio.h>
#include "chacha.h"

int main() {
	if (!chacha_check_validity()) {
		printf("self check FAILED\n");
		return 1;
	} else {
		printf("self check passed\n\n");
		return 0;
	}
}

