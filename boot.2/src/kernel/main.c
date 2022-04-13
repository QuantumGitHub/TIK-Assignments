#include "util.h"
#include "buddy.h"
#include "pt.h"
#include "test.h"

void main()
{
	int ret;

	buddy_init();

#ifdef TEST
	buddy_test();
#endif

	printstr("done!\n");

	/* FIXME: should shutdown */
	for (;;) {
	}
	/* Trick to make it stop the infinite main-loop */
};
