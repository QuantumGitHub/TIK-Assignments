// Author: Flavien Solt

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * In this small tutorial, you will learn how pointers, malloc and free work in C.
 * Please feel free to modify it and to make your own experiments.
 */

int main() {
    ////////////////////////////
    // Variables on the stack
    ////////////////////////////

    /**
     * Let's create a variable my_var.
     * This variable may have two representations in assembly when we compile this code.
     * (a) Either it will be stored somewhere on the stack.
     * (b) Or it will only reside in the registers if it has a short lifetime.
     * The cool thing is, that as C programmers, we do not care, all this is completely transparent, the compiler does it for us!
     * 
     * Such a variable will **never** be stored on the heap.
     */
    int my_var = 0;

    /**
     * Every variable in C has an address, corresponding to where it is stored (if we are in case (b) above, then the compiler will allocate some stack space to make it fit to the (a) case).
     */
    int *addr_of_my_var = &my_var;

    /**
     * Now, my_var can be changed directly for example...
     */
    my_var = 7;

    // Warning: printf may not work in Jake.
    // To print in hex, I would write %x (for a 32-bit variable) or %llx (for a 64-bit variable).
    // To print a string, I would write %s.
    printf("The value of my_var is currently: %d\n", my_var);

    /**
     * or through a pointer to it.
     */
    *addr_of_my_var = 9;

    ////////////////////////////
    // Variables on the heap
    ////////////////////////////

    /**
     * To allocate an integer on the heap instead, we would write the line below.
     * However, typically we don't allocate small data in memory if we know their size in advance.
     */
    int *addr_of_my_var_on_heap = (int *) malloc(sizeof(int));

    /**
     * The `(int *)` cast is not strictly necessary but the compiler may give warnings because malloc returns a (void *), and addr_of_my_var_on_heap expects an (int *).
     * 
     * Also, sizeof(int) is equal to 4, so we could have replaced the line above with: `int *addr_of_my_var_on_heap = (int *) malloc(4);`, but the `sizeof` macro avoids a lot of mistakes.
     */

    /**
     * You may want to check whether the malloc succeeded.
     * If not, it will return NULL.
     * For example, if there is no free space on the heap to satisfy our request, then it will fail.
     * 
     * If the malloc has failed, do NOT try to free the returned pointer (you would effectively do `free(0)`, which does not make sense for the heap).
     */
    if (addr_of_my_var_on_heap == NULL) {
        printf("My malloc failed :'( \n");
        return -1;
    }

    /**
     * Finally, do not forget to free the heap space allocated.
     * Further in the class, make sure to be clear about:
     * - Why we need to explicitly free heap space but not stack space.
     * - What happens exactly if the process stops before it has freed all its heap memory.
     */
    free(addr_of_my_var_on_heap);

    ////////////////////////////
    // Pointer arithmetics
    ////////////////////////////

    /**
     * When you add an integer to a pointer, then it does something cool and dangerous at the same time.
     * Let's take an example.
     */

    char my_char = 0; // This is 1 byte wide.
    int  my_int  = 0; // This is 4 bytes wide.

    char *incremented_char_addr = &my_char+1;
    int  *incremented_int_addr  = &my_int+1;

    printf("char address increased by %lu.\n", ((u_int64_t)incremented_char_addr) - ((u_int64_t)&my_char));
    printf("int  address increased by %lu.\n", ((u_int64_t)incremented_int_addr)  - ((u_int64_t)&my_int));

    /**
     * The char pointer increased by 1, but the int pointer advanced by 4, wow! Why?
     * Because it eases navigating in arrays! (remember the iterators that you may have seen in previous CS classes).
     */
    char my_char_arr[4] = {10, 11, 12, 13};
    int  my_int_arr[4]  = {20, 21, 22, 23};

    /**
     * Now, my_char_arr and my_int_arr are pointers to the beginning of the respective arrays.
     */

    char *second_char_elem = my_char_arr+1;
    int  *second_int_elem  = my_int_arr+1;

    printf("second char array element: %d.\n", *second_char_elem);
    printf("second int  array element: %d.\n", *second_int_elem);

    /**
     * Below, I simulate for you what we would have to do if the pointer arithmetic would not work.
     * First, I create `char *` pointers: they advance 1 byte by 1 byte (as opposed to int pointers for example, that advance 4 by 4).
     * Then, we observe that we get a wrong result for the int array (but not for the char array).
     */

    char *second_char_elem_wrong_pointer_arithmetic = (char *)(((char *)&my_char_arr)+1);
    int  *second_int_elem_wrong_pointer_arithmetic  = (int  *)(((char *)&my_int_arr)+1); // Here, I first forced C to interpret my_int_arr as a char array by writing `(char *)&my_int_arr`. 

    printf("second char array element where we did not do a proper pointer arithmetic: %d.\n", *second_char_elem_wrong_pointer_arithmetic);
    printf("second int  array element where we did not do a proper pointer arithmetic: %d.\n", *second_int_elem_wrong_pointer_arithmetic);

    return 0;
}
