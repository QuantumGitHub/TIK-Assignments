# Input registers:
#   a0, a1
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   a1 is guaranteed to be included in [1, 32].
#   Your task is to zero out the a1 top bits of a0.
#   Example: if a0 = 0b11010001101000100111001110001101 and a1 = 7, then a0 must be set to 0b00000001101000100111001110001101 

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_6
.type	assignment_1_6, @function
assignment_1_6:

    # Assignment code.
    lui t0, 0x80000
    addi a1, a1, -1
    sra t0, t0, a1

    lui t1, 0xf0000
    srai t1, t1, 31
    xor t0, t0, t1

    and a0, a0, t0
    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
