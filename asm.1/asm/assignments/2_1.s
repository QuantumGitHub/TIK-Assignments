# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be in [0, 48]
#   Compute the a0-th term of the Fibonacci sequence, then store it into a0.

# Authorized:
# beq, bne, blt, bge, bltu, bgeu
# add, addi, sub
# jal

.text
.align	1
.globl	assignment_2_1
.type	assignment_2_1, @function
assignment_2_1:

    # Assignment code.
    addi t1, zero, 1
    add t0, zero, zero
    #special cases
    beq a0, zero, L3
    beq a0, t1, L3

    addi a0, a0, -1
fi:
    add t2, t1, zero
    add t1, t1, t0
    add t0, t2, zero
    
    addi a0, a0, -1
    bne a0, zero, fi #if not, jump to fi:

    add a0, t1, zero
L3:
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
