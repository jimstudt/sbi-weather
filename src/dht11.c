
#include <stdio.h>
#include "pico/stdlib.h"

#include "dht11.h"

#define MAX_STATES 100

void probe_dht11( const unsigned char pin) {
    gpio_init( pin);
    gpio_pull_up( pin);   // pull-up
    gpio_set_dir( pin, GPIO_IN);

    // let sit high for a while
    sleep_ms( 1);

    #if 0
    printf("Strobing pin\n");
    for (int i = 0; i < 20; i++) {
	gpio_set_dir( pin, GPIO_OUT);
	gpio_put( pin, 0);
	sleep_ms( 500);
	gpio_set_dir( pin, GPIO_IN);
	sleep_ms( 500);
    }
    #endif
    
    bool state = 0;
    uint16_t stateTime[MAX_STATES] = { 0 };
    int states = 0;
    
    gpio_set_dir( pin, GPIO_OUT);

    gpio_put( pin, 0);
    sleep_ms( 20);      // issue the start pulse

    gpio_put( pin, 1);  // don't wait for the pullup, jam it up there.
    sleep_us( 1);
    
    gpio_set_dir( pin, GPIO_IN);
    
    uint64_t startTime = to_us_since_boot( get_absolute_time());
    uint64_t lastChange = startTime;
    
    while( true) {
	uint64_t now = to_us_since_boot( get_absolute_time());
	if ( now - startTime > 5000) break;

	if ( gpio_get(pin) != state) {
	    stateTime[states++] = now - lastChange;
	    lastChange = now;
	    state = !state;

	    if ( states >= MAX_STATES) break;
	}
    }

    printf("DHT11 %d bits sampled\n", states);
    for ( int i = 0; i < states; i += 2) {
	printf( "  %3d %3d\n", stateTime[i], stateTime[i+1]);
    }
}
