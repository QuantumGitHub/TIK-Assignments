// Author: Flavien Solt

#include <stdio.h>

/**
 * In this small tutorial, you will learn how the preprocessor works in C.
 * Please feel free to modify it and to make your own experiments.
 */

/**
 * The preprocessor is a tool that runs automatically before the compiler.
 * You can recognize the preprocessor commands as they start with a `#` character.
 * For our purposes, the preprocessor does two tasks.
 * 1) It replaces all `#include` commands by the content of the file in `<>` for built-in headers, or in "" for headers that you create. You can create your own header files and do, for example #include "my_header".
 * 2) It replaces all macros, defined by the `#define` command, literally with the specified values.
 */

/**
 * This link is useful if you need more information: https://gcc.gnu.org/onlinedocs/cpp/Macros.html
 */

int main() {
    ////////////////////////////
    // Macro objects
    ////////////////////////////

/**
 * By convention, macro objects are often written in capital letters.
 * They can be defined anywhere and can be re-defined.
 * Usually, we define macros at the very top of the file. I am doing differently in this tutorial, but strongly advise to regroup macro definition in the top of the file.
 */
#define MY_FIRST_MACRO 7

    int my_num = 5 + MY_FIRST_MACRO; // 12.
    printf("The value of my_num is: %d\n", my_num);

/**
 * Because macros are textually replaced, be careful about parentheses!
 * More pitfalls here: https://gcc.gnu.org/onlinedocs/cpp/Macro-Pitfalls.html
 */
#define MY_SECOND_MACRO 4+3

    int my_second_num = 2 * MY_SECOND_MACRO; // 11, because it is expanded as 2 * 4+3.
    printf("The value of my_second_num is: %d\n", my_second_num);

/**
 * So usually we put parentheses around macros.
 */
#define MY_THIRD_MACRO (4+3)

    int my_third_num = 2 * MY_THIRD_MACRO; // 14, because it is expanded as 2 * (4+3). 
    printf("The value of my_third_num is: %d\n", my_third_num);

    ////////////////////////////
    // Macro function-like
    ////////////////////////////

/**
 * We can also define function-like macros that take arguments and return a value.
 * The difference with traditional C functions is that a function-like macro will be textually replaced in-line, whereas traditional C functions are typically called (using the jal and jalr RISC-V instructions for instance).
 * However, the C compiler may decide to treat traditional C functions as macros under certain conditions.
 * The cool thing is that when you type C, this decision-making is transparent to you.
 */
#define my_first_macro_with_params(first_param, another_param) (2*first_param + another_param)

    int my_result = my_first_macro_with_params(8, 3); // 19.
    printf("The value of my_result is: %d\n", my_result);
}
