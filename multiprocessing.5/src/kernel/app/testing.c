#include "api.h"
#include "../util.h"

//FIXME: test/test.h contains the cursed kernel/util.h
#include "../../test/test.h"

enum tests {
	flags_txt_vma,
	flags_stack_vma,
	copy_binary_implementation,
	print_syscall,
	getpid_syscall,
	check_mmap,
	launch_mult_processes,
	check_mult_app_req_mem,
	timer_schedulable,
	check_app_return,
	__success
};

static const struct test ts[] = {
	[flags_txt_vma] = { "flags_txt_vma", 1 },
	[flags_stack_vma] = { "flags_stack_vma", 1 },
	[copy_binary_implementation] = { "copy_binary_implementation", 1 },
	[print_syscall] = { "print_syscall", 0 },
	[getpid_syscall] = { "getpid_syscall", 1 },
	[check_mmap] = { "check_mmap", 1 },
	[launch_mult_processes] = { "launch_mult_processes", 1 },
	[check_mult_app_req_mem] = { "check_mult_app_req_mem", 1 },
	[timer_schedulable] = { "timer_schedulable", 1 },
	[check_app_return] = { "check_app_return", 0 }
};

int proc_test(void)
{
	struct test_machine tm = { 0 };

	for (tm.t = 0; tm.t < __success; tm.t++) {
		switch (tm.t) {
		case flags_txt_vma:
		case flags_stack_vma:
		case copy_binary_implementation:
			break;
		case print_syscall:
			printstr(
				"~~ Hello world! ~~ \n");
			break;
		case getpid_syscall:
			if (getpid() != 0) goto err_pid_syscall;
			break;

		case check_mmap:
			char *ptr;
			ptr = mmap(1);

			if (!ptr) goto err_mmap_allocation;

			ptr[0] = 'H';
			ptr[1] = 'e';
			ptr[2] = 'l';
			ptr[3] = 'l';
			ptr[4] = 'o';
			ptr[5] = '\0';
			break;

		case launch_mult_processes:
			for (int i = 0; i < 10; i++) {
				while (syscall(SCALL_EXECV, HELLO_JR, i) ==
				       -1) {
					syscall(SCALL_YIELD, 0, 0);
				}
			}

			break;
		case check_mult_app_req_mem:
			for (int i = 10; i < 20; i++) {
				while (syscall(SCALL_EXECV, HELLO_JR, i) ==
				       -1) {
					syscall(SCALL_YIELD, 0, 0);
				}
			}
			break;
		case timer_schedulable:
			for (int i = 20; i < 30; i++) {
				while (syscall(SCALL_EXECV, HELLO_JR, i) ==
				       -1) {
					syscall(SCALL_YIELD, 0, 0);
				}
			}
			if (syscall(SCALL_TIMERSTATUS, 0, 0) != 1)
				goto err_timer;
			break;
		case check_app_return:
			break;
		}

		test_score_update(&tm, ts, tm.t);
		test_print_success(ts[tm.t].name, tm.scores[tm.t],
				   tm.total_score);
	}

	return 0;

err_mmap_allocation:
	test_print_failure(ts[tm.t].name, tm.total_score);
	printstr("error: could not allocate using mmap.. \n");
	return -1;

err_pid_syscall:
	test_print_failure(ts[tm.t].name, tm.total_score);
	printstr("\terror: invalid return from GETPID syscall\n");
	return -1;
err_timer:
	test_print_failure(ts[tm.t].name, tm.total_score);
	printstr("\terror: timer not working properly\n");
	return -1;
}

void start()
{
	proc_test();

	for (int runs = 0; runs < 10; runs++) {
		for (volatile int a = 0; a < 2000000; a++) {
			if (a % 1000000 == 0) {
			}
		}
	}

	syscall(SCALL_END, 0, 0);
}
