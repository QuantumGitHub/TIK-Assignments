#include "exception.h"
#include "debug.h"
#include "timer.h"

// Variable used to check the status of the timer
uint64_t timer_check = 0;

static int handle_text_demand(uintptr_t stval);

int vma_has_perm(struct vma *vma, uintptr_t scause)
{
	switch (scause) {
	case LOAD_ACCESS_FAULT:
	case LOAD_PAGE_FAULT:
		return vma->flags & VMA_READ;
	case STORE_AMO_ACCESS_FAULT:
	case STORE_AMO_PAGE_FAULT:
		return vma->flags & VMA_WRITE;
	case INSTRUCTION_PAGE_FAULT:
		return vma->flags & VMA_EXEC;
	}

	return 0;
}

/* timer_handler 
 * Controls if there is a pending timer interrupt
 * if that is the case, it handles it by context switching
*/
inline int timer_handler()
{
	uintptr_t sip = 0;
	uint64_t spp = 0;

	csrr(sip, sip);
	csrr(sstatus, spp);

	if ((sip & 0x20) == 0x20) {
		if ((spp & (1 << 8)) >> 8) {
			printerr("TIMER ERROR: this should not happen.\n");
			for (;;)
				;
		}
		ecall_disable_tint();

		if (processes_total > 1) {
			timer_check = 1;
			proc_copy_frame(&process_list[proc_running].tf,
					tf_user);
			// In case of timer interrupt, the instruction
			// that took the trap has to be repeated
			process_list[proc_running].tf.sepc -= 4;
			process_list[proc_running].status = PROCESS_PAUSED;
			proc_scheduler();
		} else if (processes_total == 0) {
			printerr("TIMER ERROR: this should not happen.\n");
			proc_turnoff();
		}
		return 1; // No process to switch to
	}
	return 0; // No timer pending
}

/* TODO:
* Implement scall_handler_print by using put().
* Prints the character contained in a1 (of the trap_frame).
*/
inline void scall_handler_print()
{
	// TODO
}

// Handler for SCALL_END
inline void scall_handler_end()
{
	proc_kill_process(proc_running);
	processes_total--;

	if (processes_total == 0) {
		printstr("No more processes to run.\n");
		proc_turnoff();
	}

	proc_scheduler();
}

/* scall_handler_execv
 * Handler for SCALL_EXECV. It executes a new app, identified
 * by a1, with the main() argument in a2. Returns -1 to the 
 * calling process (a3) in case of errors, otherwise 0.
*/
void scall_handler_execv()
{
	process_list[proc_running].status = PROCESS_PAUSED;
	tf_user->a3 = 0;
	proc_copy_frame(&process_list[proc_running].tf, tf_user);

	void *ptr__ = NULL;

	switch (tf_user->a1) {
	case 0:
		ptr__ = &_testing;
		break;
	case 1:
		ptr__ = &_hello_jr;
		break;
	default:
		printerr("ERROR: Non-existing application.\n");
		ecall_poweroff();
		break;
	}
	if (proc_run_process(ptr__, tf_user->a2) < 0) {
		tf_user->a3 = -1;
		csrw(satp, process_list[proc_running].satp);
		pt_flush_tlb();
		process_list[proc_running].status = PROCESS_RUNNING;
	}
}

/* TODO
* Implement scall_handler_yield
* If there are processes to switch to:
* 	1) Set the running process status to paused
* 	2) Copy the working trap frame to the process trap frame
* 	3) Call proc_scheduler.
* ~ 5 lines of code
*/
void scall_handler_yield()
{
	// TODO
}

/* TODO
* Implement scall_handler_getpid
* Returns the current process PID to the usermode process
* The value should be put in a3 (of the trap_frame)
*/
void scall_handler_getpid()
{
	// TODO
}

/* get_free_vpn
 * Returns the first free vpn for heap allocation, for a determined process.
 * @t: index of the process, in process_list[]
 */
uintptr_t get_free_vpn(int t)
{
	uintptr_t max_vpn = 0;
	int vpn_max_pagen = 0;

	struct node *node = process_list[t].uvmas->next;

	while (node != process_list[proc_running].uvmas) {
		struct vma *uvma = member_to_struct(node, node, struct vma);

		if (uvma->vpn != USER_STACK_BASE_VPN &&
		    uvma->vpn >= USER_HEAP_BASE_VPN) {
			if (uvma->vpn > max_vpn) {
				max_vpn = uvma->vpn;
				vpn_max_pagen = uvma->pagen;
			}
		}
		node = node->next;
	}

	if (max_vpn == 0) return USER_HEAP_BASE_VPN;

	return max_vpn + vpn_max_pagen;
}

/* TODO:
* Implement scall_handler_mmap 
* Sets in a3 the virtual address of a newly
* created VMA. In case of error it should return NULL.
* The number of pages requested is contained in a1. The VMA flags should be: VMA_READ, VMA_WRITE, VMA_EXEC, VMA_USER.
* NOTE: The new VMA should NOT overlap with any existing VMAs. 
* Its virtual address should be based on USER_HEAP_BASE_VPN and the amount of extra pages allocated: use get_free_vpn() to find the next free vpn.
*/
void scall_handler_mmap()
{
	// TODO
}

// Handler for SCALL_TIMERSTATUS
void scall_handler_timerstatus()
{
	tf_user->a3 = timer_check;
}

/* scall_handler() 
 * Redirects the syscalls to their handlers.
 * Syscalls are coming from user mode to supervisor mode.
 * TODO: implement the handlers for: 
 * SCALL_PRINT, SCALL_YIELD, SCALL_GETPID and SCALL_MMAP 
 */
void scall_handler()
{
	tf_user->a3 = -1;
	switch (tf_user->a0) {
	case SCALL_PRINT:
		scall_handler_print(); // TODO: complete this function
		break;
	case SCALL_END:
		scall_handler_end();
		break;
	case SCALL_EXECV:
		scall_handler_execv();
		break;
	case SCALL_YIELD:
		scall_handler_yield(); // TODO: complete this function
		break;
	case SCALL_GETPID:
		scall_handler_getpid(); // TODO: complete this function
		break;
	case SCALL_MMAP:
		scall_handler_mmap(); // TODO: complete this function
		break;
	case SCALL_TIMERSTATUS:
		scall_handler_timerstatus();
		break;
	default:
		printerr("Unknown syscall. Stalling.\n");
		printdbg("syscall: ", tf_user->a0);
		print_call_info();
		ecall_poweroff();
	};
}

/* scall
 * Trap/exception handler for supervisor mode.
 * The trampoline for scall is in trap.S.
 *
 * Syscall can be called from user mode (functions in api.h)
 * Parameters are passed using registers:
 * a0: Type of syscall
 * a1: (Possible) input parameter
 * a2: (Possible) second input parameter
 * a3: Return parameter
 */
void scall()
{
	uint64_t spp = 0;
	uintptr_t scause = 0;
	uintptr_t stval = 0;
	uintptr_t satp = 0;

	if (timer_handler()) return;

	csrr(scause, scause);
	scause &= SCAUSE_EXCEPTION_CODE_MASK;

	if ((scause >> SCAUSE_INTERRUPT_SHIFT) & 0x1) {
		printerr("ERROR: Interrupt not implemented.\n");
		goto hlt;
	}

	switch (scause) {
	case ILLEGAL_INSTRUCTION:
		printerr("illegal instruction\n");
		print_all_registers();
		goto hlt;
	case ENV_CALL_U:
	case ENV_CALL_S:
		// Syscalls from usermode are sent to the dispatcher
		scall_handler();
		fix_sepc;
		return;
	case LOAD_ACCESS_FAULT:
	case LOAD_PAGE_FAULT:
	case STORE_AMO_ACCESS_FAULT:
	case STORE_AMO_PAGE_FAULT:
	case INSTRUCTION_PAGE_FAULT:
		csrr(sstatus, spp);
		csrr(satp, satp);
		csrr(stval, stval);

		uintptr_t vpn = virt2vpn(stval);
		struct vma *vma;
		if ((spp & (1 << 8)) >> 8)
			vma = vpn2vma(kvmas_head, vpn);
		else
			vma = vpn2vma(process_list[proc_running].uvmas, vpn);

		/*
		 * 1. No VMA? Hard (or major) page fault
		 * 2. VMA but invalid permissions? Hard (or major) page fault
		 * 3. VMA and valid permissions? Soft (or soft) page fault: handle it
		 */
		if (!vma) {
			goto err_majorpf;
		} else if (!vma_has_perm(vma, scause)) {
			goto err_majorpf;
		} else {
			if (pt_vma_map_page_at_vpn(&satp, vma, vpn, 0) <= 0) {
				goto err_noresources;
			} else {
				// In case the page fault is for code (.text), after the
				// allocation we need to copy the content.
				// NOTE: Understand the following condition is very helpful for the task.
				if (!((spp & (1 << 8)) >> 8) &&
				    (stval <=
				     process_list[proc_running]
						     .elf.elf.virtual_load +
					     process_list[proc_running]
						     .elf.elf.size_load) &&
				    stval >= process_list[proc_running]
						     .elf.elf.virtual_load) {
					// TODO: implement handle_text_demand()
					if (handle_text_demand(stval) != 0)
						goto hlt;
				}
				goto ret;
			}
		}

	default:
		printerr("uncaught exception\n");
		printdbg("scause: ", (void *)scause);
		goto hlt;
	}

err_noresources:
	printerr("no resources\n");
	goto hlt;
err_majorpf:
	printerr("major pf\n");
	goto hlt;
hlt:
	printerr("*** *** ***\n");
	printstr(RED);
	print_call_info();
	printerr("frame that died:\n");
	proc_print_frame(tf_user);
	printdbg(RED "Process that was running: ", proc_running);
	printdbg(RED "Out of: ", processes_total);
	printdbg(RED "PID: ", process_list[proc_running].pid);
	printstr(RESET);

	printerr("*** hlt ***\n");

	ecall_poweroff();
ret:
	return;
}

/* handle_text_demand
 * Handles the soft page fault for a .text section, by copying the data from the elf file.
 * @stval virtual address that caused the page fault
 */
static int handle_text_demand(uintptr_t stval)
{
	// TODO: Copy the correct application page, to the correct
	// "kernel" virtual address. Use get_kvirt() to obtain the kernel virtual address, starting from a user virtual address. After that, use proc_copy_binary().

	// TODO
}

/* mcall
 * Trap/exception handler for machine mode.
 * The trampoline for mcall is in trap.S.
 */
void mcall(uint64_t tpointer)
{
	tf_generic = tpointer;

	uint64_t mstatus, spp, mpp, mip, mcause;
	csrr(mip, mip);

	// Timer interrupt
	if ((mip & 0x80) == 0x80) {
		csrr(mstatus, mstatus);
		spp = ((mstatus & (1 << 8)) >> 8);
		mpp = ((mstatus & (3 << 11)) >> 11);

		increase_timer();

		if (mpp == 0 && spp == 0) {
			// We forward the timer-int to S-mode
			// riscv-priv:p28
			trigger_timer_s;
		} else if (mpp == 0x3) {
			printerr("ERROR: Timer interrupt is too fast.\n");
			turn_off;
		}

		// For Qemu4.2, the timer is NMI, so mepc contains
		// the NEXT instruction
		// fix_mepc;
		return;
	}

	csrr(mcause, mcause);

	// Syscalls from supervisor to machine mode
	if ((mcause & ENV_CALL) == ENV_CALL) {
		switch (tf_generic->a0) {
		case ECALL_TIMER_SETUP:
			timer_setup();
			break;
		case ECALL_CLEAR_TIMER_S:
			clear_timer_s;
			break;
		case ECALL_TURNOFF:
			turn_off;
			break;
		default:
			printerr("ERROR: unrecognized ECALL. Stalling.\n");
			mprint_call_info();
			print_all_registers();
			turn_off;
			break;
		};

		fix_mepc;
		return;
	}
	printerr("ERROR: Unknown event to machine handler. Stalling.\n");
	mprint_call_info();
	print_all_registers();
	turn_off;
}

inline uint64_t ecall(const char ID, const char arg1, const char arg2)
{
	uint64_t retval;

	asm volatile("mv a0, %[ID]\n\t"
		     "mv a1, %[arg1]\n\t"
		     "mv a2, %[arg2]\n\t"
		     "ecall\n\t"
		     "mv %[retval], a3\n\t"
		     : [retval] "=r"(retval)
		     : [ID] "r"(ID), [arg1] "r"(arg1), [arg2] "r"(arg2)
		     : "a0", "a1", "a2", "a3", "memory");

	return retval;
}

inline void ecall_disable_tint()
{
	ecall(ECALL_CLEAR_TIMER_S, 0, 0);
}

inline void ecall_poweroff()
{
	ecall(ECALL_TURNOFF, 0, 0);
}

inline void ecall_timer_setup()
{
	ecall(ECALL_TIMER_SETUP, 0, 0);
}
