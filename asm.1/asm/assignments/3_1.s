# Input registers:
#   a0, a1
# Output register: None.

# Task:
#   a0 is the base address of a 4-byte integer table in memory.
#   Your task is to increment the a1-th element of the table.

# Authorized:
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# sll, slli, srl, srli, sra, srai

.text
.align	1
.globl	assignment_3_1
.type	assignment_3_1, @function
assignment_3_1:

    # Assignment code.
    add t1, a1, a1 #a1 is multiplied by 2
    add t1, t1, t1 #a1 is multiplied by 4
    
    add t1, a0, t1 #add 4*a1 to the address, this will lead to the a1-th 4-byte memory slot
    lw t0, 0(t1) #load value from address a0 into t0
    addi t0, t0, 1 #increment by one
    sw t0, 0(t1) #save the value at adress a0
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
