# Input register:
#   a0
# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer. It is guaranteed to be included in [1, 32].
#   Your task is to create a binary number with a0 ones on the most significant bit side, and the potential other bits are zero.
#   Example: if a0 = 9, then a0 must be set to 0b11111111100000000000000000000000 

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_5
.type	assignment_1_5, @function
assignment_1_5:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
