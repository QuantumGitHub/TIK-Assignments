# Input registers:
#   a0, a1, a2, a3, a4, a4, a6
# Output register: None

# Task:
#   Your task is to multiply two matrices M0 and M1 of 2-byte signed integers and to store the resulting matrix M2 in memory.
#   a0 contains the base address of M0.
#   a1 contains the width of M0  (number of columns). Included in [1, 30]. It is the same as the height of M1.
#   a2 contains the height of M0 (number of rows). Included in [1, 30]. It is the same as the height of M2.
#   a3 contains the base address of M1.
#   a4 contains the width of M1  (number of columns). Included in [1, 30]. It is the same as the width of M2.
#   a5 contains the (pre-allocated) address where to store M2.

# Authorized:
# sll, slli, srl, srli, sra, srai
# lb, lh, lw, ld, lbu, lhu, lwu, ldu
# sb, sh, sw, sd
# add, addi
# beq, bne, blt, bge, bltu, bgeu
# j, jal, jalr
# sll, slli, srl, srli, sra, srai
# sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu
# mul

.text
.align	1
.globl	assignment_5_0
.type	assignment_5_0, @function
assignment_5_0:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
