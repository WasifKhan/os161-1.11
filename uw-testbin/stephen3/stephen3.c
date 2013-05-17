/*
 * stephen.c
 *
 * Developed by Stephen Pisani in F12
 *
 * Test program that accesses an array in the data segment over 9 MB large.
 * Only actually modifies 3 of the data pages and the user stack, so if the page
 * replacement algorithm is efficient then there only needs to be 4 writes to
 * the swap file at most. An inefficient algorithm that writes all pages to the
 * swap file will run out of swap space.
 *
 * This works on our kernel, but may cause panics with other page replacement
 * algorithms.
 *
 * The expected running time for an efficient algorithm is fairly short, less
 * than 15 seconds. If every page is being written to the swap file it will
 * take much longer depending on the swap file size.
 */

#include <unistd.h>
#include <stdio.h>

// 10 MB array
#define NUM_BYTES (10 * 1024 * 1024)
#define PAGE_SIZE 4096

volatile char array[NUM_BYTES];

int main()
{
    char temp;

    int i;
    for( i  = 0; i * PAGE_SIZE < NUM_BYTES; ++i ) {
        // Read in all the pages
        temp = array[i * PAGE_SIZE];

        // Only write in 3 of them
        if ( i < 3 ) {
            array[i * PAGE_SIZE] = 42;
        }
    }

    printf( "SUCCESS\n" );
    return 0;
}
