// GLIBC uses feature-test macros to expose functionality - define one so that
// we can see POSIX functions.
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

// Macro for swapping two values.
#define SWAP(x,y) do {\
    __typeof__(x) tmp = x;\
    x = y;\
    y = tmp;\
} while(0)

/**
 * Partition the array.  Takes the index of the pivot point as the pivot
 * argument.  Puts all of the values lower than this point into the first part
 * of the array and returns the new location of the pivot point.
 */
int partition(int *array, int left, int right, int pivot)
{
    int pivotValue = array[pivot];
    SWAP(array[pivot], array[right]);
    int storeIndex = left;
    for (int i=left ; i<right ; i++)
    {
        if (array[i] <= pivotValue)
        {
            SWAP(array[i], array[storeIndex]);
            storeIndex++;
        }
    }
    SWAP(array[storeIndex], array[right]);
    return storeIndex;
}
/**
 * Dumps the array.  Use this if you want to see intermediate steps
 */
void dump(int *array, int size)
{
    for (int i=0 ; i<size ; i++)
    {
        printf("[%d] %d\n", i, array[i]);
    }
}

/**
 * Serial Quicksort implementation.
 */
void quicksort(int *array, int left, int right)
{
     if (right > left)
     {
        int pivotIndex = left + (right - left)/2;
        pivotIndex = partition(array, left, right, pivotIndex);
        quicksort(array, left, pivotIndex-1);
        quicksort(array, pivotIndex+1, right);
     }
}
/**
 * Structure containing the arguments to the parallel_quicksort function.  Used
 * when starting it in a new thread, because pthread_create() can only pass one
 * (pointer) argument.
 */
struct qsort_starter
{
    int *array;
    int left;
    int right;
    int depth;
};

void parallel_quicksort(int *array, int left, int right, int depth);

/**
 * Thread trampoline that extracts the arguments from a qsort_starter structure
 * and calls parallel_quicksort.
 */
void* quicksort_thread(void *init)
{
    struct qsort_starter *start = init;
    parallel_quicksort(start->array, start->left, start->right, start->depth);
    return NULL;
}
/**
 * Parallel version of the quicksort function.  Takes an extra parameter:
 * depth.  This indicates the number of recursive calls that should be run in
 * parallel.  The total number of threads will be 2^depth.  If this is 0, this
 * function is equivalent to the serial quicksort.
 */
void parallel_quicksort(int *array, int left, int right, int depth)
{
    if (right > left)
    {
        int pivotIndex = left + (right - left)/2;
        pivotIndex = partition(array, left, right, pivotIndex);
        // Either do the parallel or serial quicksort, depending on the depth
        // specified.
        if (depth-- > 0)
        {
            // Create the thread for the first recursive call
            struct qsort_starter arg = {array, left, pivotIndex-1, depth};
            pthread_t thread;
            int ret = pthread_create(&thread, NULL, quicksort_thread, &arg);
            assert((ret == 0) && "Thread creation failed");
            // Perform the second recursive call in this thread
            parallel_quicksort(array, pivotIndex+1, right, depth);
            // Wait for the first call to finish.
            pthread_join(thread, NULL);
        }
        else
        {
            quicksort(array, left, pivotIndex-1);
            quicksort(array, pivotIndex+1, right);
        }
    }
}

int main(int argc, char **argv)
{
    // Default to serial mode, unless an argument is provided specifying a
    // depth
    int depth = 0;
    if (argc > 1)
    {
        depth = strtol(argv[1], NULL, 10);
    }
    // Size of the array to sort.  Optionally specified as the second argument
    // to the program.
    int size = 100000000;
    if (argc > 2)
    {
        size = strtol(argv[2], NULL, 10);
    }
    // Allocate the array and initialise it with pseudorandom numbers.  The
    // random number generator is always seeded with the same value, so this
    // should give the same sequence of numbers.
    srandom(42);
    int *values = calloc(size, sizeof(int));
    assert(values && "Allocation failed");
    for (int i=0 ; i<size ; i++)
    {
        values[i] = random();
    }
    // Sort the array
    parallel_quicksort(values, 0, size-1, depth);
    // Validate that the quicksort worked.  If you change the algorithm and
    // want to make sure that the changes worked, compile with -DDEBUG
#ifdef DEBUG
    for (int i=0 ; i<size ; i++)
    {
        assert(values[i-1] <= values[i]);
    }
#endif
    return 0;
}