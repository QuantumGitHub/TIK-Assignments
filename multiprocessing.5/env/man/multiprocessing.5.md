% MULTIPROCESSING(5) COMSEC | Computer Engineering '22: Homework Guide
% Michele Marazzi

# NAME
multiprocessing - Run and schedule processes

# DEADLINE
Friday, June 3, 2022 @ 11:59 (noon) (see Moodle)

# SOURCES
./src/kernel/exception.c

./src/kernel/process.c

./src/kernel/main.c

# HANDING IN
See `general(2)`

# COMMANDS
See `general(2)`

# DESCRIPTION
With assignment paging (4), you have learnt to manage virtual memory. However, up to now the only "process" that was running was the kernel itself. In **RISC-V**, there are 3 different CPU levels: machine mode, supervisor mode and _user mode_. So far, you have only worked on the supervisor mode.

This assignment will allow you to run programs in _user mode_ (called processes, or applications). In modern OSes, every process is typically independent from each other and is managed by the kernel. _User mode_ is an environment which has less _privileges_ than the kernel: the memory accesses are limited to its own mappings, and certain CPU registers might not be accessible. 

Programs that run in user mode will sometimes want to perform actions that only the supervisor or machine mode can do. For instance, evoking a new process, requesting memory or writing to the console. These requests are handled with _syscalls_, which generate exceptions handled by the kernel.

Implementing multiprocessing means that we need some sort _scheduling system_ to switch from one process to another and to keep track of which process is being executed, paused or ended.

Computers usually run multiple programs, where in turns each one gets a small amount of time to perform operations. In Jake, switching between processes can be done in two ways: via the **yield** syscall, or via the **timer interrupt**. The first option is a signal started from the process itself, while the timer is instead a periodic event. These two mechanisms can co-exist. Your task is to implement a scheduler that permits multiple programs to share _runtime_. This requires a few lines of code to handle and decide which process should be the one to run. Note that in both cases, the management is done in kernel code which runs in _supervisor_ mode. In this assignment, you will also prepare the virtual memory mappings necessary to run an application.

# Jake implementation details

A syscall can be thought of as a function. It needs arguments and can give outputs. In Jake, the arguments are put in the registers as follows:

-  **a0**: Type of syscalls, as described in scalls.h
-  **a1**: (Optional) input parameter, such as a character, or an amount of pages
-  **a2**: (Optional) second input parameter
-  **a3**: Return parameter. Could be an address, -1 in case of errors or 0.


Because these are usermode registers, once the syscall is being handled in the kernel (supervisor mode) they can be accessed through the process trap_frame (**tf_user->x**). The key functions for handling a syscall are **scall()**, **scall_handler()** and the various **scall_handler_xxx()**. First, the exception is taken by going to scall(). Then, if it is a syscall:

```
	switch (scause) {
	...
	case ENV_CALL_U:
	case ENV_CALL_S:
		// Syscalls from usermode are sent to the dispatcher
		scall_handler();
		fix_sepc;
		return;
```

the control flow is redirecteed to the syscall handler dispatcher (**scall_handler()**). In scall_handler(), depending on the usermode request, the appropriate function is called:

```
inline void scall_handler()
{
	tf_user->a3 = -1;
	switch (tf_user->a0) {
	case SCALL_PRINT:
		scall_handler_print();		// TODO: complete this function
		break;
	case SCALL_END:
		scall_handler_end();
		break;
	case SCALL_EXECV:
		scall_handler_execv();
		break;
...
```

**Please, notice the use of tf_user to access usermode registers (in this case, a0 and a3).** A useful example is **scall_handler_end()**. This is called when the process running wishes to terminate:

```
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
```

Note how **proc_running** is used as input to **proc_kill_process**, and that process_total is decreased. After this, the scheduler is called because the CPU control is passed to some other paused process. In **proc_kill_process**, the status of the process is set to: 
```
	process_list[proc_running].status = PROCESS_FREE_SLOT;
```

## Support for processes in JAKE
A process is described with the struct called __process__ (__process.h__). The fields are:

- **pid** : Process ID. *It is unique*, should start from 0, and should be incremented by 1 for each new process
- **status** : Process status as described by the **PROCESS_STATUS** enum.
- **uvmas** : a pointer to the process' VMAs linked list
- **tf** : the process' trap_frame, used for context switching
- **satp** : the process' *satp*
- **elf** : the information parsed from the application elf and a pointer to the application binary (in kernel memory). See below.

The maximum amount of concurrent processes is __MAX\_PROCESSES__ (__process.h__). Each process slot has its struct allocated in the global variable (__process.h__):

```
process process_list[MAX_PROCESSES];
```

The status of a process/process slot (process_list[].status) is described by enum **PROCESS_STATUS** (**process.h**). Its possible values are :

- 	**PROCESS_FREE_SLOT**. The slot is free to be used 
-	**PROCESS_RUNNING**. Usermode is currently running this process 
-	**PROCESS_PAUSED**. The process will be scheduled to run in usermode. It is not currently being run. 
-	**PROCESS_RESERVED**. The process slot is reserved and cannot be used.


The process that is running is **identified by its process_list index**, saved in the global variable (__process.h__):

```
int proc_running
```
**The process with index 0 is currently not used **and it is reserved**.** The total number of processes running is saved in the global variable (__process.h__):

```
int processes_total
```


Please, note that a process PID _can_ be different from its index in **process\_list**.

# TASKS

Take care of the _TODOs_ in process.c, exception.c and main.c. We suggest to use the order that is kept in this description. The programs we want to run as processes are included in the folder **kernel/app/** and are **testing.c** and **hello_jr.c** and they should not be modified.

### -- **process.c**
You need to complete the __proc_run_process()__ function. This function takes as arguments 1) a pointer to the application binary and 2) the argument to pass to the main of the application (**a3**). 

#### **proc_find_free_process_slot()**

The first thing to do is finish the **proc_find_free_process_slot()** function. It should return an index to an element of **process\_list** that is free to use (i.e., its status is **PROCESS_FREE_SLOT**). In case none of the elements satisfy this condition, it should return -1.

#### Parsing the elf

Now you need to parse the elf with __parse\_elf()__. __parse\_elf()__ takes as arguments the pointer to the application structure, and a pointer to where the retrieved information should be written to. For the latter, you should use the struct **elf_jake** (__elfpars.h__).

__elf\_jake__ contains:

- __struct elf_simple elf__ see below (filled by parse_elf())
- __char *ptr_elf__ Pointer to the elf in memory (filled by parse_elf())

__elf\_simple__ contains:

- __e_entry__ : the virtual address where the application starts running
- __size_load__ : the size (in bytes) of the application code
- __offset_load__ : an offset *w.r.t. the elf pointer* where the .text begins. the ".text" section, is where the code of the application resides
- __virtual_load__  : the virtual address where the application code should be placed

#### **proc_create_process_vmas()**

Now you need to create the page table entries for the new process. Initially there will only be 2 vmas: one for the code and one for the stack. To do so, complete the function **proc_create_process_vmas()**. The inputs of this function are an index to a free element of process_list, and a pointer to an elf_jake structure that has been parsed. It returns the virtual address of the process' stack.

You need to:

- Allocate a new page table saving it to **root_pt**. This will be where the user mapping will be saved. Use the function **pt\_alloc\_pt()** for this.

- Set **process\_list[t].uvmas**. This is a linked list that needs to be first initialized using **pt_init_vmas_head()**. After that, new vmas can be 1) allocated with **pt_alloc_vma()** and 2) "Filled" with **pt_vma_new()**. You should create __2__ vmas. One for the text, and one for the stack. More details in the comments in the code. The vpn for the stack should be at **USER_STACK_BASE_VPN**, while the vpn for the text depends on the elf.

- The variables vpn_text and vpn_stack  should be set accordingly and before their uses.


Remember that __vpn_text__ and __vpn_stack__ address a single page, i.e., you should consider that their value will be shifted by 12 bits to the left. 

### -- **exception.c**

Both the *stack* and *text* user-mode vmas are created but not populated. This means that, as soon as the process is launched (by jumping to elf.e_entry) there will be a soft page fault. A soft page fault for a *text* page needs to be treated carefully. Not only it should be allocated (we already did this for you), but the code should be copied to it. This is your first task to do in **exception.c**.

#### Handling soft page faults for applications code 
If you look in scall(), you will see that if a user-mode application causes a (.text) soft page fault, **handle_text_demand()** is called. Your task is to write this function. handle_text_demand() needs to call **proc_copy_binary()** (see below) with the correct arguments. handle_text_demand needs to specificy to proc_copy_binary which is the target virtual address the function should write to, where is the elf structure and which is the missing page (w.r.t. the .text segment). 

It shold **not** ask proc_copy_binary() to copy the code directly to the user virtual address, as the supervisor cannot access it. Instead, you should first obtain its identity mapping. This will give you a (kernel) virtual address, which is mapped to the same physical region. Use get_kvirt() for this.

The elf pointer (**ptr_elf**) points to the entire application binary. The .text segment (the application code) starts at an offset from ptr_elf given by **elf.offset_load** (you will need this for proc_copy_binary). The .text segment is loaded at the virtual address **elf.virtual_load**. Lastly, the faulting address is contained in **stval**. This information will give you the page missing.

#### **proc_copy_binary()** (in process.c)

Complete the function **proc\_copy\_binary()** (in process.c) by using __memcpy__. NOTE: the amount of data to copy is AT MOST one page at the time. The destination is the kernel virtual address of the target *_text_* page, the source is obtained by: 1) the elf pointer 2) the load offset 3) the page faulting. Some information is contained in the **elf** struct precedently filled.

This should allow you to start at least one process, however you are still not able to see any output. Before implementing the scheduler, it is suggested that you implement the **PRINT** syscall.

#### Syscalls

You need to complete the syscall handlers for **SCALL_PRINT**, **SCALL_GETPID**, **SCALL_MMAP**, and **SCALL_YIELD**. You should leave **SCALL_YIELD** for after **scheduler** is completed (see blow). **SCALL_PRINT** simply prints a character, while **SCALL_GETPID** just returns the process PID. **SCALL_MMAP** creates a vma of the requested page number and returns its virtual address.

#### How to return values to the usermode process?

The return values are saved in **a3**. However, if you would simply modify the **a3** CPU register, this would have no effect. After the syscall is handled, the original control flow is restored by loading the **trap_frame** again on the CPU. This means that you need to modify the **a3** associated with the **trap_frame** of the interrupting application. You can find the associated frame as:

```
tf_user->a3
```

**tf_user** holds all the original CPU content and will be used to restore them. 

### -- **process.c**

#### **proc_scheduler()**

As the name suggest, this function is used in the kernel to change from one running process to another. Once this is called, your code needs to:

1. Find the next process that should run, by looking into **process_list[]**. You can choose the algorithm to implement this.

2. Update **proc_running** accordingly

**It is very important that your code maintains the organization of jake.** For example, if you look at the function that is called at the end (context_switch()), this will use 

```
&process_list[proc_running].tf
```

As the trap frame to load on the CPU. This implies that **proc_running** will index the process to be restored, and that its trap frame contains the correct CPU registers.


### -- **exception.c**

The last task is to complete **SCALL\_YIELD** by completing **scall_handler_yield()**  Your code should first check if there is any other running process (**processes_total**). If that is the case, it should copy the temporary trap frame (contained in **tf_user**) to its process trap frame, contained in **process_list[].tf**: use **copy\_frame()** for this. Then, it should call **proc_scheduler()**. 

### -- **main.c**: adding the timer

In real life, processes are scheduled not only by yielding, but also by timers. We have implemented the timer for you. If you have implemented everything correctly, by activating the timer you should get a correct output (graded assigned) and no crash. When you are ready to work with the timer, uncomment "**ecall_timer_setup()**" in main.c. **If you can't obtain a bug-free output with the timer, submit your code with the line commented. Please, note that if the timer does not work, it could interefere with a working implementation, resulting in a lower grade. You can always see the result of the grading by running "make"**.

# TESTS

The following tests will determine your grade, and will be run in the following order:

## flags_txt_vma (1/8)
Test if the flags for the text vma are assigned.

## flags_stack_vma (1/8)
Test if the flags for the stack vma are assigned.

## copy_binary_implementation (1/8)
Test if proc_copy_binary() is implemented.

## print_syscall (0/8)
Test if the print syscall is implemented (mandatory but does not provide points).

## getpid_syscall (1/8)
Test if the GETPID syscall is implemented.

## check_mmap (1/8)
Test if the MMAP syscall is implemented.

## launch_mult_processes (1/8)
Test if the scheduler is implemented.

## check_mult_app_req_mem (1/8)
Test if multiple applications can request memory via MMAP.

## timer_schedulable (1/8)
Test if the timer can schedule processes without crashing.

## check_app_return (0/8)
Additional test to see if no major crashes are detected (mandatory but does not provide points).

# Appendix A ~ Extra details

## Applications in JAKE

**testing** and **hello\_jr** are compiled and included in Jake OS automatically at each _make_. Applications on many operating systems (including Jake) are described with the **elf** file format. A file format is just a standard way to give rules on how a file should be written (other examples are .jpg, .bmp, ..., .exe). Not all OSes uses the **elf** format as default, for instance, Windows uses the PE format to describe ".exe" files. A file format is necessary because the OS needs to know how to handle the application that is to be run, for example which libraries it might need, where the code should be placed in memory and so on. We already prepared for you a very easy elf parser with the function __parse\_elf()__ (in __elfpars.c__). 

## Exception/Interrupt handling

When an exception or interrupt happens, the code starts executing as supervisor at the address contained in the __stvec__, which in our case points to **trap_handler_s** in the file **trap.S**. Once the execution is over, a specific return command (__sret__) brings the control back to the address contained in __sepc__ as usermode. Whenever this handling occurs, the value of all the CPU registers (_a0, a1, ... sp, ... ra_) should be immediately saved somewhere. In particular, their values are written in what we call a _trap frame_. This allows the CPU to go back to the original workload, once the interrupt/exception is resolved. Moreover, it also allows context switching, i.e.: passing from one program (a.k.a. application) to another -- you can see this in the struct **trap_frame** (**process.h**). 

A very useful type of exception is a syscall. The aim of a syscall is for the usermode to request specific high-priviledge operations, which are run by the kernel. This is achieved using the __ecall__ assembly instruction, which generates an exception. Examples of syscalls include:

* **SCALL_END**: It indicates that the process that was running wishes to terminate. We already implemented this for you. Please take a look, and **note that a global variable (**processes_total**) is decreased**. 
* **SCALL_EXECV**:It indicates that the application running wishes to launch another program.  
* **SCALL_YIELD**: An application using this syscall, communicates to the kernel that an immediate context switching is possible. You need to complete it.

## Summary

To recap: the usermode can perform a syscall (via the __ecall__ assembly instruction) to ask the kernel to perform some operations, such as printing on the console. The CPU will see a syscall and generate an exception, by jumping to a specific location in memory. After the interrupt is resolved, the original workload is restored. A very important syscall is the one used to print something on the screen, identified as (__scalls.h__): 

```
#define SCALL_PRINT 1 
```

While handling the syscall in kernel, the process registers can be accessed (and modified) via the trap_frame (tf_user).

# Appendix C ~ Timer extra details

Examples of _machine_ mode syscalls, **only callable from supervisor mode** include:

* **ECALL_TURNOFF**: Shut down Qemu.
* **ECALL_CLEAR_TIMER_S**: Clears the timer interrupt for the supervisor. 

Why is machine mode required? This is due to how timer interrupts works in RISC-V. Please read the next paragraph for more details.

## Timer interrupts in RISC-V 

RISC-V has a timer interrupt per each mode (machine, supervisor and user). Generally, interrupts (or exceptions) can be delegated towards lower privileges. In many situations this avoids expensive context switch. For example, in the case of a page fault, the supervisor (running kernel code) will handle it: it makes no sense for the CPU to jump to machine mode only to move to supervisor immediately after. For this reason, any page fault will be immediately delegated to supervisor mode. 

However, the timer interrupts are "special". Each mode has his own timer interrupt, but there is only one _source_, which goes directly to machine mode. After this has happened, the timer interrupt _can_ be manually set to other modes via software. This is possible because interrupt registers of lower modes can be edited. In other words: the timer induces the machine to take control, no matter what. However, each mode has an _independent_ flag that indicates if a timer interrupt is present. For the supervisor, these bit should be set via software while in the machine.

Making things more complex, clearing the supervisor timer interrupt requires higher privileges. I.e., an __ecall__ to machine mode is needed. **Note that we only want to have timer interrupts in supervisor mode and not user mode**.

Let us recap the order of events:

1. A timer interrupt is received and registered  

2. The CPU sees that there is an interrupt associated with the machine mode

3. The machine mode interrupt handler is evoked (**mcall**, in **syscall.c**)
    - 3.1 The code clears the interrupt, and triggers a timer interrupt for supervisor mode
    - 3.2 The control flow goes back to the original point

4. The supervisor interrupt handler is evoked because of a timer interrupt (**scall**, in **syscall.c**)
    - 4.1 The supervisor asks the machine mode to clear the timer interrupt
        - 4.1.1 The machine mode interrupt handler is evoked 
        - 4.1.2 The timer interrupt of the supervisor is cleared
    - 4.2 The supervisor handles the process time sharing

5. The original (_or a different_) control flow is restored

## Timers in RISC-V/Qemu

How is the timer set up? How can the interrupt be cleared? The timer works as memory-mapped registers, in the physical addresses:

* 0x200BFF8 : **MTIME**
* 0x2004000 : **MTIMECMP** 

**MTIME** gets increased by 1 with the timer frequency. When **MTIMECMP == MTIME**, a timer interrupt is generated. Changing the value of **MTIMECMP** clears the interrupt. To have timer interrupts of a defined **PERIOD**, every time **MTIMECMP** is updated, its value should become  **MTIME+PERIOD**. Please, note that the update takes place in machine mode: memory accesses are direct, i.e. they do not use the MMU. 

# COLOPHON
This is the fourth homework assignment of Computer Engineering '22.
