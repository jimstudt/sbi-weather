#include "i2c.h"

#include <stdio.h>
#include "hardware/i2c.h"

static bool i2c_configured = false;

void setup_i2c( uint scl, uint sda) {
    if ( i2c_configured ) return;
    
    i2c_init( i2c0, 100*1000);
    gpio_set_function( sda, GPIO_FUNC_I2C);
    gpio_set_function( scl, GPIO_FUNC_I2C);
    gpio_pull_up( sda);
    gpio_pull_up( scl);

    i2c_configured = true;
}

int read_i2c_at_byte( uint8_t device, uint8_t addr, uint8_t *data, uint8_t maxBytes) {
    if ( i2c_write_blocking( i2c0, device, &addr, 1, true) != 1) {
	return PICO_ERROR_GENERIC;
    }
    return i2c_read_blocking( i2c0, device, data, maxBytes, false);
}
int read_i2c_at_short( uint8_t device, uint16_t addr, uint8_t *data, uint8_t maxBytes) {
    uint8_t a[] = { addr>>8, addr & 0xff };
    
    if ( i2c_write_blocking( i2c0, device, a, 2, true) != 2) {
	return PICO_ERROR_GENERIC;
    }
    return i2c_read_blocking( i2c0, device, data, maxBytes, false);
}

int write_i2c( uint8_t device, const uint8_t *data, uint8_t bytes) {
    return i2c_write_blocking( i2c0, device, data, bytes, false);
}

void probe_i2c( uint scl, uint sda) {
    setup_i2c( scl, sda);

    bool gotOne = false;
    
    for ( uint8_t addr = 0x08; addr < 0x78; addr++) {
        uint8_t rxdata;

	int ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);
	if ( ret >= 0) {
	    printf("i2c %02x returns count of %d\n", addr, ret);
	    gotOne = true;
	}
    }
    if ( !gotOne) {
	printf("No i2c devices found\n");
    }

    // Checkout the DS3231
    if (1) {
	uint8_t temperatureData[2];

	if ( read_i2c_at_byte( 0x68, 0x11, temperatureData, 2) == 2) {
	    int temperature = (temperatureData[0]&0x7f)*4 + (temperatureData[1]>>6);
	    // broken for negative numebrs
	    
	    printf("DS3231 clock temperature is %d and %d/4 degrees.\n", temperature/4, temperature % 4);
	} else {
	    printf("DS3231 failed to read temperature\n");
	}

	uint8_t timeData[7];

	if ( read_i2c_at_byte( 0x68, 0x00, timeData, 7) == 7) {
	    for (int i = 0; i < 7; i++) {
		printf("  %d: %02x\n", i, timeData[i]);
	    }
	}	
    }

    // Checkout the AT24C32
    if (1) {
	uint8_t data[4] = {0};

	if ( read_i2c_at_short( 0x57, 0x0000, data, 4) == 4) {
	    printf("EEPROM begins %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
	} else {
	    printf("Failed to read EEPROM\n");
	}

	uint8_t change[6] = { 0,0, data[0]+1,data[1]+2,data[2]+3,data[3]+4};
	if ( write_i2c( 0x57, change, 6) != 6) {
	    printf("EEPROM write failed\n");
	} else {
	    uint8_t ok = 1;

	    //
	    // This mess is repeatedly doing a read, which fails until
	    // the write has completed and the device is ready again.
	    //
	    uint64_t startTime = time_us_64();
	    while( 1) {
		int ret = read_i2c_at_short( 0x57, 0x0000, &ok, 1);
		if ( ret != 1) {
		    uint64_t now = time_us_64();

		    if ( now - startTime > 20000 ) {   // after 20ms without an ack, we have failed
			printf("EEPROM ack check failed\n");
			break;
		    } else {
			sleep_ms(1);
		    }
		} else {
		    printf("EEPROM ack = %02x\n", ok);
		    break;
		}
	    }
	}

        if ( read_i2c_at_short( 0x57, 0x0000, data, 4) == 4) {
            printf("EEPROM begins %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
        } else {
            printf("Failed to read EEPROM\n");
        }
	
    }
}


