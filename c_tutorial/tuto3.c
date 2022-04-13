// Author: Flavien Solt

#include <stdio.h>
#include <stdlib.h>

/**
 * In this small tutorial, you will learn about custom data types, strings, structures and about the `->` operator.
 * Please feel free to modify it and to make your own experiments.
 */

/**
 * I'll start by defining a new data type `student_age_t`.
 * The `_t` suffix is a convention that says that mysuper_pointer_type_t is a type (and not a variable, a function or a macro for example).
 */
typedef unsigned char student_age_t;

/**
 * Why do we define new types? Here are two reasons (there may be other ones):
 *  - Because some types are complex (long and error prone to write).
 *  - They show the intention of the programmer. For example, if someone uses a variable of type `mysuper_pointer_type_t`, then this variable is typically intended to contain a pointer.
 * 
 * Here, for example, I chose this type to represent your age because your age is usually not negative and not above 255.
 * For your information, a `char` variable is a variable encoded as a single byte.
 */

/**
 * Let me define a structure that represents a student.
 */
struct student {
	student_age_t age; // Equivalently, I could have written `unsigned char age;`
    long long int bank_balance; // I chose to store the bank balance in a 64-bit integer (because some students are Swiss after all), and signed (they are still students :) ).
	char *name; // This is a pointer to the student's name.
};

int main() {
    ////////////////////////////
    // Instantiating a student
    ////////////////////////////

    /**
     * Let's instantiate a student.
     */

    struct student my_student_instance;
    /**
     * For the moment, we have not yet initialized the fields (i.e., the internal variables) of the student instance.
     * Possibly, each field can have any initial value.
     * I did not print the name because I would get a segmentation fault (because the pointer can be anything).
     */
    printf("Initially, my_student_instance.age:          %d.\n", (int)my_student_instance.age);
    printf("Initially, my_student_instance.bank_balance: %d.\n", (int)my_student_instance.bank_balance);
    // printf("Initially, my_student_instance.name:          %s.\n", my_student_instance.name);
    
    /**
     * Let's now give some values to the student.
     */
    my_student_instance.age = 21;
    my_student_instance.bank_balance = -100;

    printf("Now, my_student_instance.age:          %d.\n", (int)my_student_instance.age);
    printf("Now, my_student_instance.bank_balance: %d.\n", (int)my_student_instance.bank_balance);

    /**
     * We need to create a string for the student's name.
     * One typical strategy would be to put the name into the heap.
     * Of course, there are shorter syntaxes than the one below, but 
     */

    char *new_name_pointer = (char *)malloc(7); // We reserve some space for the student's name.
    new_name_pointer[0] = 'A';
    new_name_pointer[1] = 'l';
    new_name_pointer[2] = 'b';
    new_name_pointer[3] = 'e';
    new_name_pointer[4] = 'r';
    new_name_pointer[5] = 't';
    new_name_pointer[6] = '\0'; // The character `\0` represents the end of a string.

    my_student_instance.name = new_name_pointer;
    printf("Now, my_student_instance.name:         %s.\n", my_student_instance.name);

    /**
     * Could you guess what is the size of a variable of type `struct student`?
     * Importantly, this does **not** depend on the name, because a variable of type `struct student` only contains a pointer to the name (and not the name itself).
     */
    printf("The size of a struct student is: %ld.\n", sizeof(struct student));

    /**
     * Free the name that we do not use anymore. 
     */
    free(new_name_pointer);

    ////////////////////////////
    // The `->` notation
    ////////////////////////////

    /**
     * The `x->y` is just a shortcut notation that means `(*x).y`, where x is a pointer to a struct, and y is one field of the struct.
     * Concretely, with a struct student, we get the following (it is just an useless example so that you can understand the syntax).
     */

    struct student *pointer_to_my_student = &my_student_instance;
    int student_age = pointer_to_my_student->age; // Or equivalently, `int student_age = (*pointer_to_my_student).age`.
    // Equivalently, we could have written, instead of the 2 lines above, `int student_age = my_student_instance.age;`.

    /////////////////////////////////
    // Putting students on the heap
    /////////////////////////////////

    /**
     * We may want to store students on the heap instead of in the stack, for example, if we don't know how many students there will be.
     * Here is for example how one may add one student to the stack. 
     */
    struct student *student_on_the_heap = (struct student*)malloc(sizeof(struct student));

    student_on_the_heap->age = 22;
    student_on_the_heap->bank_balance = 1400;

    /**
     * Create a name for the student: we need to allocate it separately.
     */
    student_on_the_heap->name = (char*) malloc(9);
    /**
     * Indexing in an array or doing pointer arithmetics is exactly the same, as we have seen in tuto1.
     * This looks a bit more complex, and is just here to let you see equivalent things. Please don't do pointer arithmetic in practice when filling such a simple array :)
     */
    *(student_on_the_heap->name + 0) = 'E';
    *(student_on_the_heap->name + 1) = 'i';
    *(student_on_the_heap->name + 2) = 'n';
    *(student_on_the_heap->name + 3) = 's';
    *(student_on_the_heap->name + 4) = 't';
    *(student_on_the_heap->name + 5) = 'e';
    *(student_on_the_heap->name + 6) = 'i';
    *(student_on_the_heap->name + 7) = 'n';
    *(student_on_the_heap->name + 8) = '\0'; // The character `\0` represents the end of a string.

    printf("For the student on the heap, age:          %d.\n", (int)student_on_the_heap->age);
    printf("For the student on the heap, bank balance: %d.\n", (int)student_on_the_heap->bank_balance);
    printf("For the student on the heap, name:         %s.\n", student_on_the_heap->name);

    /**
     * Free the student on the heap when we don't use it anymore, and don't forget to also free its name that was allocated separately!
     */
    free(student_on_the_heap->name);
    free(student_on_the_heap);
}
