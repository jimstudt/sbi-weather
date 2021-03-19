#include "store.h"

#include "pico/stdlib.h"
#include <stdio.h>

static uint8_t store[4096] __attribute__ (( aligned(4096) ));

void print_store( const char *line) {
    printf("Store location: %p\n", &store);
    printf("Store size: %d bytes\n", sizeof(store)); 
}
