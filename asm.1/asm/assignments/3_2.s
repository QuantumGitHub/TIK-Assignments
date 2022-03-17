# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is the base address of a 4-byte integer table in memory.
#   Your task is to find the number of nonzero elements before the first zero element in the table.
#   It is guaranteed that the table contains at least one zero element.

# Authorized:
# jal, jalr
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi, sub
# beq, bne, blt, bge, bltu, bgeu

.text
.align	1
.globl	assignment_3_2
.type	assignment_3_2, @function
assignment_3_2:

    # Assignment code.
    addi t0, zero, -1 #counter variable, it is -1 because the first time j1 is entered it is incremented

j1:
    addi t0, t0, 1 #increment t0
    lw t1, 0(a0) #load the value from address a0 to t1
    addi a0, a0, 4 #the address has to be pointing to the next table slot, which is 4 enumeration further in the memory
    bne t1, zero, j1 #branch if the value of the current address is not equal to zero

    add a0, t0, zero #save the value of the counter to a0
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
