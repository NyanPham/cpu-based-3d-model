#ifndef ARRAY_H
#define ARRAY_H

/**
 * @file array.h
 * @brief A generic dynamic array (stretchy buffer) implementation for C.
 *
 * This file implements a clever "stretchy buffer" macro system. 
 * The array pointer behaves exactly like a standard C array (e.g., `my_array[i]`),
 * but it dynamically grows its memory behind the scenes when you push new elements.
 */

/**
 * @def array_push
 * @brief Adds a new element to the end of the dynamic array.
 * 
 * Automatically calculates the size of the elements, reallocates memory 
 * if the array is full (via `array_hold`), and then assigns the new value to the end.
 * It is wrapped in a `do { ... } while (0)` loop to ensure it behaves safely like a 
 * single statement even when used inside `if` conditions without braces.
 */
#define array_push(array, value)                                              \
    do {                                                                      \
        (array) = array_hold((array), 1, sizeof(*(array)));                   \
        (array)[array_length(array) - 1] = (value);                           \
    } while (0);

/**
 * Ensures the array has enough memory to hold `count` more elements.
 * If the array is NULL, it allocates initial memory.
 * If the array is full, it doubles the capacity via `realloc`.
 * 
 * @warning This function is primarily meant to be called by the `array_push` macro,
 * but can be used manually to pre-allocate memory for performance.
 */
void* array_hold(void* array, int count, int item_size);

/**
 * Returns the current number of elements (occupied slots) in the array.
 * If the array is NULL, it safely returns 0.
 */
int array_length(void* array);

/**
 * Frees the memory allocated for the dynamic array.
 * Safely handles NULL pointers.
 */
void array_free(void* array);

#endif
