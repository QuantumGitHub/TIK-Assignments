# Input register:
#   a0,a1
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte signed integer (tip: we work with 64-bit wide registers).
#   a1 indicates whether this integer has been stored in little-endian (a1 = 0) or big-endian (a1 = 1) mode.
#   Your task is to retrieve the integer from memory and to return it.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# add, addi, sub, or
# sll, slli, srl, srli, sra, srai
# beq, bne
# jal, jalr

.text
.align	1
.globl	assignment_3_3
.type	assignment_3_3, @function
assignment_3_3:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
