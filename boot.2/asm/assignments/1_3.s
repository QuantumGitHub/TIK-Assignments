# Output register:
#   a0

# Task:
#   a0 is a 32-bit integer.
#   Set a0's value to 0xbadcab1e.

# Authorized:
# sll, slli, srl, srli, sra, srai
# add, addi, sub, lui, auipc
# xor, xori, or, ori, and, andi
# slt slti, sltu, sltiu

.text
.align	1
.globl	assignment_1_3
.type	assignment_1_3, @function
assignment_1_3:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
