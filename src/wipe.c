
#include "wipe.h"
#include "hardware/watchdog.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"

void wipe(void) {
    const uint8_t blank[256] = {0};

    // Stop core1. We better be core0.
    multicore_reset_core1();                  
	
    uint32_t saved = save_and_disable_interrupts();
    // Wipe the secondary boot loader
    flash_range_program( 0, blank, 256);      
    restore_interrupts(saved);
	
    // this will reset us in 1 millisecond.
    watchdog_enable( 1, false);
    
    // await our doom
    while( true);
}
