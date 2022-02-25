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

    # -- End of assignment code.

    jr ra # Return to the testing framework. Don't modify.
