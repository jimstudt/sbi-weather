
#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

#include "tx23u.h"

#define MAX_STATES 100

void tx23u_init( struct tx23u_t *device, uint8_t pin, void (*handler)(struct tx23u_t *device)) {
    *device = (struct tx23u_t){ .pin = pin,
	.handler = handler,
	.sampleValid = false,
	.sample = { {0,0}, {0,0} },
	.packet = { 0,0,0,0,0},
	.bit = 0,
	.gust = false,
	.state = TX23_IDLE,
	.seenPreamble = false,
	.bitTime = 1220,
    };

    gpio_init( pin);
    gpio_pull_up( pin);   // pull-up
    gpio_set_dir( pin, GPIO_IN);
}


void tx23u_print( struct tx23u_sample_t *sample) {
    printf("TX23U: %.1fm/s from %d, gust %.1fm/s from %d\n",
	   sample->average.speed/10.0, sample->average.direction,
	   sample->gust.speed/10.0, sample->gust.direction);
}


/*
** This is just to make sure everyting works according to my
** understanding and is wired.
**
** Ultimately I will use a PIO pulse width reader for this.
*/
void probe_tx23u( const unsigned char pin) {
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
	    printf("  %d for %lldus\n", state, now - lastChange);
	    lastChange = now;
	    state = !state;

	    if ( states >= MAX_STATES) break;
	}
    }

}


union packet {
    uint8_t byte[5];
    struct {
	uint64_t n0:4;
	uint64_t n1:4;
	uint64_t n2:4;
	uint64_t n3:4;
	uint64_t n4:4;
	uint64_t n5:4;
	uint64_t n6:4;
	uint64_t n7:4;
	uint64_t n8:4;
    };
    struct {
	uint64_t dir:4;
	uint64_t speed:12;
	uint64_t csum:4;
	uint64_t invertedDir:4;
	uint64_t invertedSpeedLow:8;
	uint64_t unused:4;
	uint64_t invertedSpeedHigh:4;
   };
};


void tx23u_reset( struct tx23u_t *device) {
    device->state = TX23_IDLE;
}

static bool within( uint32_t value, uint32_t target, uint32_t tolerance) {
    return value >= ((int)target - tolerance) && value <= ((int)target + tolerance); 
}

static bool decode( uint8_t packetIn[], struct tx23u_vector_t *vector) {
    printf("Got a wind packet! %02x %02x %02x %02x %02x\n",
	   packetIn[0], packetIn[1], packetIn[2], packetIn[3], packetIn[4]);

    union packet packet;
    memcpy( packet.byte, packetIn, 5);
    
    uint8_t sum1 = (packet.n0 + packet.n1 + packet.n2 + packet.n3) & 0x0f;
    uint8_t sum2 = packet.n4;

    if ( sum1 != sum2) {
	printf("tx23u sum mismatch: sum1=%02x sum2=%02x\n", sum1, sum2);
	return false;
    }

    uint16_t iSpeed = packet.invertedSpeedLow + (packet.invertedSpeedHigh << 8);
    if ( packet.dir != ( ~packet.invertedDir & 0x0f) || packet.speed !=  ( ~iSpeed & 0x0fff) ) {
	printf("tx23u inverted data mismatch: %02x-%02x  %04x-%04x\n",
	       packet.dir, packet.invertedDir, packet.speed, iSpeed);
	return false;
    }

    vector->direction = packet.dir;
    vector->speed = packet.speed;

    return true;
}

void tx23u_pulse( struct tx23u_t *device, bool isHigh, uint32_t usecs) {
    const uint32_t ms = 1000;
    const uint32_t initialBitTime = 1220;

    #define MAX_BITS 36

    
    switch( device->state) {
      case TX23_IDLE:
	if ( isHigh) break;      // high while idle is no change
	if ( usecs > 100*ms) {
	    device->state = TX23_COLLECTING;
	    device->gust = false;
	}
	break;
      case TX23_COLLECTING:
	if ( isHigh ) {
	    if ( usecs > 10*ms) {
		printf("tx23u aborted COLLECTING, high too long\n");
		device->state = TX23_IDLE;  // Too long high, not responding to a sample request
		break; 
	    } else {
		break;     // we don't care about the high part
	    }
	}

	// TX23u holds line down for about 20ms while doing something.
	if ( usecs > 10*ms) {
	    device->state = TX23_START1;
	    device->bitTime = initialBitTime;
	} else {
	    printf("tx23u false trigger\n");
	    device->state = TX23_IDLE;
	}
	break;
      case TX23_START1:
	// We are looking for the first two high bits of the preamble
	
	if ( !isHigh) {         // no lows
	    device->state = TX23_IDLE;
	    break;
	}
	// Use a wide tolerance because the tx23u clock varies with temperature.
	if ( within( usecs/2, device->bitTime, device->bitTime/5) ) {
	    device->state = TX23_START2;
	    device->bitTime = usecs/2;     // Learn our actual bit rate
	    break;
	} else {
	    printf("tx23u first start bits out of spec start\n");
	    device->state = TX23_IDLE;
	    break;
	}
      case TX23_START2:
	// We are looking for the third bit, low, of the preamble
	
	if ( isHigh) {         // no highs
	    device->state = TX23_IDLE;
	    break;
	}
	if ( within( usecs, device->bitTime, device->bitTime/10) ) {
	    device->state = TX23_DATA;
	    device->bit = 0;
	    device->seenPreamble = false;
	    memset( device->packet, 0, sizeof device->packet);
	    break;
	} else {
	    printf("tx23u 3rd start bit out of spec start\n");
	    device->state = TX23_IDLE;
	    break;
	}
      case TX23_DATA:
	if ( device->bit >= MAX_BITS) {
	    device->state = TX23_IDLE;
	    printf("tx23u overrun\n");
	    break;
	}
	uint8_t bits = (usecs+device->bitTime/2)/device->bitTime;

	// Check if we are an integer multiple of bit times, more or less.
	if ( bits + device->bit >= MAX_BITS) {
	    // We are ending with a high, which then persists. Don't time check it.
	} else {
	    if ( !within( usecs, bits*device->bitTime, device->bitTime/10*bits)) {
		printf("tx23u weird bit length: %ldus %ldus/bit\n", usecs, device->bitTime);
		device->state = TX23_IDLE;
		break;
	    }
	}

	// If we haven't seen the last two high bits of the preamble, then
	// match them and remove them.
	if ( !device->seenPreamble ) {
	    if ( !isHigh || bits < 2) {
		device->state = TX23_IDLE;
		printf("tx23u bad start, missing last two high bits of preamble\n");
		break;
	    }
	    device->seenPreamble = true;
	    bits -= 2;   // take off the last of the 5 start bits
	}

	// Stick the bits into our packet
	for ( uint8_t i = 0; i < bits; i++) {
	    uint8_t b = device->bit/8;
	    device->packet[b] = ( (device->packet[b] >> 1) | ( isHigh ? 0x80 : 0) );

	    // If we have filled the packet, then do something with it
	    if ( ++(device->bit) >= MAX_BITS) {
		if ( device->gust) {
		    if ( decode( device->packet, &device->sample.gust) ) {
			device->state = TX23_IDLE;
			device->sampleValid = true;
			if ( device->handler) device->handler( device);
			device->gust = false;
		    } else {
			device->state = TX23_IDLE;
		    }
		} else {
		    if ( decode( device->packet, &device->sample.average)) {
			device->state = TX23_COLLECTING;
			device->gust = true;
		    } else {
			device->state = TX23_IDLE;
		    }
		}
		return;
	    }
	}

	break;
      default:
	printf("tx23u bad state\n");
	device->state = TX23_IDLE;
    }

    //printf("  tx23u %d->%d\n", initialState, state);
}


bool tx23u_sample( uint8_t pin, struct tx23u_sample_t *sample) {
    struct tx23u_sample_t collected = { {0,0}, {0,0} };
    bool gotOne = false;
    
    void record( struct tx23u_t *dev) {
	if ( dev->sampleValid) {
	    collected = dev->sample;
	    gotOne = true;
	}
    }
    
    struct tx23u_t device;
    tx23u_init( &device, pin, record );

    // let sit high for a while
    sleep_ms( 1);

    gpio_set_dir( pin, GPIO_OUT);
    gpio_put( pin, 0);
    sleep_ms( 500);      // issue the start pulse

    gpio_put( pin, 1);  // don't wait for the pullup, jam it up there.
    sleep_us( 1);

    gpio_set_dir( pin, GPIO_IN);  // start listening


    
    uint64_t startTime = time_us_64();
    uint64_t lastChange = startTime;
    bool state = true;
    int states = 0;

    tx23u_pulse( &device, true, 1000*1000);   // pretend we saw the computer's start trigger pulse
    tx23u_pulse( &device, false, 500*1000);   

    while( true) {
	uint64_t now = time_us_64();
	if ( now - startTime > 500000) break;

	if ( gpio_get(pin) != state) {
	    uint64_t duration = now - lastChange;
	    
	    tx23u_pulse( &device, state, duration);
	    lastChange = now;
	    state = !state;

	    if ( states >= MAX_STATES || duration > 40000 ) break;
	}
    }

    if ( gotOne) {
	if ( sample) *sample = collected;
	return true;
    } else {
	return false;
    }
}
