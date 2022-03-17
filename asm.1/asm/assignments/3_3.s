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
    add t1, zero, zero

    bne a1, zero, j1   # if it is small endian, we have to load the integer into the reg normally, big endian jump

    lw a0, 0(a0)
    beq zero, zero, exit
j1: #if it is small endian, we have to load the integer into the reg "backwards" starting with the last 
    addi t0, a0, 3 ## one too much because the loop decrements it, the address we want is 3 when a0 points to 0
    addi a0, a0, -1
j2:
    addi a0, a0, 1
    lbu t2, 0(a0)
    slli t1, t1, 8
    or t1, t1, t2

    bne t0, a0, j2
    add a0, t1, zero #save the result in a0
exit:
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
