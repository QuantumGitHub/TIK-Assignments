# Input registers:
#   a0, a1, a2
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be 0 or 1.
#   If a0 == 0, then set a0 to the value of a1. Else, set a0 to the value of a2.
#   No jumps or branches are allowed except for the return.

# Authorized:
# slld, slliw, sllid, srlw, srld, srliw, srlid, sraw, srad, sraiw, sraid
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_7
.type	assignment_1_7, @function
assignment_1_7:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
