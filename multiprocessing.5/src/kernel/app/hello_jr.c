#include "api.h"
#include "../util.h"

void main(uint64_t arg)
{
	if (arg < 10) {
		syscall(SCALL_YIELD, 0, 0);
	} else if (arg >= 10 && arg < 20) {
		char *ptr;
		ptr = mmap(50);

		if (ptr) {
			ptr[0] = 't';
			ptr[1 + 1024 * 4 * 1] = 'e';
			ptr[1 + 1024 * 4 * 2] = 's';
			ptr[1 + 1024 * 4 * 3] = 't';
		}

		for (int vmas = 0; vmas < 100; vmas++) {
			ptr = mmap(1);
		}
	} else {
		for (int runs = 0; runs < 10; runs++) {
			for (volatile int a = 1; a < 2000000; a++) {
				if ((a % 1000000) == 0) {
				}
			}
		}
	}

	return;
}

void start()
{
	uint64_t arg = get_arg();
	main(arg);
	syscall(SCALL_END, 0, 0);
}
