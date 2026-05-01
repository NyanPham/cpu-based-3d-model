#include <stdio.h>
#include <stdlib.h>
#include "array.h"

// ============================================================================
// Dynamic Array Implementation (Stretchy Buffers)
// ============================================================================

/**
 * MEMORY LAYOUT TRICK:
 * To make the dynamic array behave like a normal C array (`array[i]`), the pointer 
 * returned to the user points directly to the data. But we need a place to store 
 * the hidden metadata: `capacity` (total allocated slots) and `occupied` (current length).
 * 
 * We hide this metadata just *before* the data pointer!
 * 
 * Memory block:
 * [ capacity (int) ] [ occupied (int) ] [ item 0 ] [ item 1 ] [ item 2 ] ...
 *                                       ^
 *                                       |-- User's pointer points here
 * 
 * To read the metadata, we cast the user's pointer to an `int*` and step backwards by 2!
 */

// Step back 2 integer slots to find the true beginning of the malloc block
#define ARRAY_RAW_DATA(array) ((int*)(array) - 2)

// Read the Capacity (slot 0)
#define ARRAY_CAPACITY(array) (ARRAY_RAW_DATA(array)[0])

// Read the Occupied length (slot 1)
#define ARRAY_OCCUPIED(array) (ARRAY_RAW_DATA(array)[1])

/**
 * Ensures the array has enough capacity to add 'count' new elements.
 * This function handles the actual memory allocation and growth logic.
 */
void* array_hold(void* array, int count, int item_size) {
    // CASE 1: Array is entirely empty. Allocate it for the first time.
    if (array == NULL) {
        // Size = 2 ints for metadata + space for the new items
        int raw_size = (sizeof(int) * 2) + (item_size * count);
        int* base = (int*)malloc(raw_size);
        base[0] = count;  // Initial capacity
        base[1] = count;  // Initial occupied length
        
        // Return a pointer to the start of the actual data (skip the 2 metadata ints)
        return base + 2;
    } 
    // CASE 2: Array has enough space. Just increase the 'occupied' counter.
    else if (ARRAY_OCCUPIED(array) + count <= ARRAY_CAPACITY(array)) {
        ARRAY_OCCUPIED(array) += count;
        return array;
    } 
    // CASE 3: Array is full. We need to grow (realloc) the memory.
    else {
        int needed_size = ARRAY_OCCUPIED(array) + count;
        int double_curr = ARRAY_CAPACITY(array) * 2;
        
        // Standard dynamic array growth: Double the capacity, or use the exact needed size
        // if doubling isn't enough (e.g., pushing many elements at once).
        int capacity = needed_size > double_curr ? needed_size : double_curr;
        int occupied = needed_size;
        
        int raw_size = sizeof(int) * 2 + item_size * capacity;
        
        // realloc automatically copies the old data to the new memory block
        int* base = (int*)realloc(ARRAY_RAW_DATA(array), raw_size);
        base[0] = capacity;
        base[1] = occupied;
        
        // Return the new data pointer
        return base + 2;
    }
}

/**
 * Returns the number of elements in the dynamic array.
 */
int array_length(void* array) {
    // Safely check for NULL before trying to read the hidden metadata
    return (array != NULL) ? ARRAY_OCCUPIED(array) : 0;
}

/**
 * Frees the dynamic array memory block.
 */
void array_free(void* array) {
    if (array != NULL) {
        // We must free the TRUE base pointer, not the user's data pointer!
        free(ARRAY_RAW_DATA(array));
    }
}
