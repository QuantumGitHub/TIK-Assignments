# stack.S: First thing that runs---on all CPUs at the same time! No need to
# pick one for initialization purposes

# Why does this work without setting MIE or SIE?
# 	1. Once we're in S-mode, privilege mode is less than M ->
# 	2. M-mode interrupts are globally enabled (p. 31) ->
# 	3. medeleg to scall (p. 30)
#
# Please note that the ISA specifies that we should always support handling
# exceptions in M-mode, but we currently don't

.extern main # .extern is ignored by the assembler, but not by humans

# FIXME: there's also a TOP_STACK in buddy.c, make sure there's only one .globl?
# .globl TOP_STACK
.equ TOP_STACK, 0x2000 # 32 MB because Upper Imm. (0x2000 << 12)

.section .text
	# Initialize stack pointer
	auipc sp, TOP_STACK
	# Disable paging
	csrw satp, 0
	# Return to S-mode
	xor t0, t0, t0
	xor t0, t0, 0x01
	sll t0, t0, 0xb
	csrw mstatus, t0
	# Set return address for mret to main
	la t0, main
	csrw mepc, t0
	mret
