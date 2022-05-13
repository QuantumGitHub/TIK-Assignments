#include "api.h"

/*mmap
 *Performs a SCALL_MMAP
 *FIXME: Maybe add parameters for page size?
 *@pagen: number of pages requested
 */
uint64_t mmap(const char pagen)
{
	return syscall(SCALL_MMAP, pagen, 0);
}

/*put
 *Performs a SCALL_PRINT
 *@a: character to print 
 */
void put(const char a)
{
	syscall(SCALL_PRINT, a, 0);
}

/*getpid
 *return: current process PID
 */
int getpid(void)
{
	return (int)syscall(SCALL_GETPID, 0, 0);
}

/*evoke
 *Performs a SCALL_EXECV
 *@arg1: a program to execute (picked from PROGRAMS enum)
 *@arg2: arguments for the main function (fetched from the application by get_arg())
 */
void evoke(const char arg1, const char arg2)
{
	syscall(SCALL_EXECV, arg1, arg2);
}

/*syscall
 * FIXME: Write documentation
 */
uint64_t syscall(const char ID, const char arg1, const char arg2)
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

/*get_arg
 *Get argument of process call passed by evoke(.,.,arg2)
 */
uint64_t get_arg(void)
{
	uint64_t arg;

	asm volatile("mv %[arg], a3\n\t" : [arg] "=r"(arg)::"memory");

	return arg;
}