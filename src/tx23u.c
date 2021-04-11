
#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

#include "tx23u.h"

#define MAX_STATES 100

/*
** This is just to make sure everyting works according to my
** understanding and is wired.
**
** Ultimately I will use a PIO pulse width reader for this.
*/
void probe_tx23u( const unsigned char pin) {
    tx23u_reset();
    
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
    
    bool state = 1;
    uint16_t stateTime[MAX_STATES] = { 0 };
    int states = 0;
    
    gpio_set_dir( pin, GPIO_OUT);

    gpio_put( pin, 0);
    sleep_ms( 500);      // issue the start pulse

    gpio_put( pin, 1);  // don't wait for the pullup, jam it up there.
    sleep_us( 1);

    gpio_set_dir( pin, GPIO_IN);
    
    uint64_t startTime = time_us_64();
    uint64_t lastChange = startTime;
    
    while( true) {
	uint64_t now = time_us_64();
	if ( now - startTime > 500000) break;

	if ( gpio_get(pin) != state) {
	    stateTime[states++] = now - lastChange;
	    lastChange = now;
	    state = !state;

	    if ( states >= MAX_STATES) break;
	}
    }

    printf("TX23U %d bits sampled\n", states);
    tx23u_pulse( 1000*1000, 500*1000);   // pretend we saw the computer's start trigger pulse
    for ( int i = 0; i < states; i += 2) {
	tx23u_pulse( stateTime[i], stateTime[i+1]);
	printf( "  %3d %3d\n", stateTime[i], stateTime[i+1]);
    }
}

enum state {
    TX23_IDLE=0,
    TX23_COLLECTING,
    TX23_START,
    TX23_DATA
};

static enum state state = TX23_IDLE;

void tx23u_reset(void) {
    state = TX23_IDLE;
}

static bool within( uint32_t value, uint32_t target, uint32_t tolerance) {
    return value >= ((int)target - tolerance) && value <= ((int)target + tolerance); 
}

static bool decode( uint8_t packet[], uint8_t *direction, uint16_t *speed) {
    printf("Got a wind packet! %02x %02x %02x %02x %02x\n",
	   packet[0], packet[1], packet[2], packet[3], packet[4]);
    
    uint8_t sum1 = (packet[0] & 0x0f) + ((packet[0]>>4)&0x0f) + (packet[1]&0x0f) + ((packet[1]>>4)&0x0f);
    uint8_t sum2 = (packet[2] & 0x0f);
    
    printf("  sum1=%02x sum2=%02x\n", sum1, sum2);

    return true;
}

void tx23u_pulse( uint32_t high, uint32_t low) {
    const uint32_t ms = 1000;
    const uint32_t initialBitTime = 1220;
    static uint32_t bitTime = initialBitTime;

    #define MAX_BITS 36
    static uint8_t packet[ (MAX_BITS +7)/8 ] = {0};
    static uint32_t bit = 0;
	
    switch( state) {
      case TX23_IDLE:
	if ( low > 100*ms) {
	    state = TX23_COLLECTING;
	}
	break;
      case TX23_COLLECTING:
	// TX23u holds line down for about 20ms while doing something.
	if ( low > 10*ms) {
	    state = TX23_START;
	    bitTime = initialBitTime;
	} else {
	    printf("tx23u false trigger\n");
	    state = TX23_IDLE;
	}
	break;
      case TX23_START:
	// Use a wide tolerance because the tx23u clock varies with temperature.
	if ( within( high/2, bitTime, bitTime/5) &&
	     within( low, bitTime, bitTime/5)) {
	    state = TX23_DATA;
	    bitTime = (high + low)/3;     // Learn our actual bit rate
	    bit = 0;
	    memset( packet, 0, sizeof packet);
	    break;
	} else {
	    printf("tx23u out of spec start\n");
	    state = TX23_IDLE;
	    break;
	}
      case TX23_DATA:
	if ( bit >= MAX_BITS) {
	    state = TX23_IDLE;
	    printf("tx23u overrun\n");
	    break;
	}
	uint8_t highBits = (high+bitTime/2)/bitTime;
	uint8_t lowBits = (low+bitTime/2)/bitTime;

	if ( highBits + bit >= MAX_BITS) {
	    // We are ending with a high, which then persists. Don't time check it.
	} else {
	    if ( !within( high, highBits*bitTime, bitTime/10*highBits)) {
		printf("tx23u weird high bits: %ldus %ldus/bit\n", high, bitTime);
		state = TX23_IDLE;
		break;
	    }
	    if ( !within( low, lowBits*bitTime, bitTime/10*lowBits)) {
		printf("tx23u weird low bits: %ldus %ldus/bit\n", low, bitTime);
		state = TX23_IDLE;
		break;
	    }
	}

	if ( bit == 0) {
	    if (highBits < 2) {
		state = TX23_IDLE;
		printf("tx23u bad start, missing last two high bits\n");
		break;
	    }
	    highBits -= 2;   // take off the last of the 5 start bits
	}

	for ( uint8_t i = 0; i < highBits; i++) {
	    uint8_t b = bit/8;
	    printf("== packet[%d] %02x -> ", b, packet[b]);
	    packet[b] = ( (packet[b] >> 1) | 0x80 );
	    printf("%02x\n", packet[b]);
	    
	    if ( ++bit >= MAX_BITS) {
		uint8_t dir;
		uint16_t speed;
		decode( packet, &dir, &speed);
		state = TX23_IDLE;
		return;
	    }
	}

	for ( uint8_t i = 0; i < lowBits; i++) {
	    uint8_t b = bit/8;
	    printf("== packet[%d] %02x -> ", b, packet[b]);
	    packet[b] = (packet[b] >> 1);
	    printf("%02x\n", packet[b]);

	    if ( ++bit >= MAX_BITS) {
		uint8_t dir;
		uint16_t speed;
		decode( packet, &dir, &speed);
		state = TX23_IDLE;
		return;
	    }
	}

	break;
      default:
	printf("tx23u bad state\n");
	state = TX23_IDLE;
    }
}
