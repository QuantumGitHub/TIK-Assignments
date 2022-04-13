# Input registers:
#   a0, a1, a2
# Output register:
#   a0

# Task:
#   a0 is guaranteed to be in {0, 1}.
#   If a0 == 0, then set a0 to the value of a1. Else, set a0 to the value of a2.
#   Arithmetic & logic operations are limited in this assignment, but branches are allowed.

# Authorized:
# add, addi
# beq, bne, blt, bge, bltu, bgeu

.text
.align	1
.globl	assignment_2_0
.type	assignment_2_0, @function
assignment_2_0:

    # Assignment code.

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
