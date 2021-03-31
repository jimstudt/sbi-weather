#include "i2c.h"

#include <stdio.h>
#include "hardware/i2c.h"

void probe_i2c( uint scl, uint sda) {
    i2c_init( i2c0, 100*1000);
    gpio_set_function( sda, GPIO_FUNC_I2C);
    gpio_set_function( scl, GPIO_FUNC_I2C);
    gpio_pull_up( sda);
    gpio_pull_up( scl);

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
	uint8_t temperatureAddr[1] = { 0x11 };
	uint8_t temperatureData[2];

	i2c_write_blocking( i2c0, 0x68, temperatureAddr, 1, true);
	if ( i2c_read_blocking( i2c0, 0x68, temperatureData, 2, false) == 2) {
	    int temperature = (temperatureData[0]&0x7f)*4 + (temperatureData[1]>>6);
	    // broken for negative numebrs
	    
	    printf("DS3231 clock temperature is %d and %d/4 degrees.\n", temperature/4, temperature % 4);
	}

	uint8_t timeAddr[1] = { 0x00 };
	uint8_t timeData[7];

	i2c_write_blocking( i2c0, 0x68, timeAddr, 1, true);
	if ( i2c_read_blocking( i2c0, 0x68, timeData, 7, false) == 7) {
	    for (int i = 0; i < 7; i++) {
		printf("  %d: %02x\n", i, timeData[i]);
	    }
	}	
    }

    // Checkout the AT24C32
    if (1) {
	uint8_t address[2] = {0,0};
	uint8_t data[4] = {0};

	i2c_write_blocking( i2c0, 0x57, address, 2, true);
	if ( i2c_read_blocking( i2c0, 0x57, data, 4, false) == 4) {
	    printf("EEPROM begins %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
	} else {
	    printf("Failed to read EEPROM\n");
	}

	uint8_t change[6] = { 0,0, data[0]+1,data[1]+2,data[2]+3,data[3]+4};
	if ( i2c_write_blocking( i2c0, 0x57, change, 6, false) != 6) {
	    printf("EEPROM write failed\n");
	} else {
	    uint8_t ok = 1;

	    //
	    // This mess is repeatedly doing a read, which fails until
	    // the write has completed and the device is ready again.
	    //
	    uint64_t startTime = to_us_since_boot( get_absolute_time());
	    while( 1) {
		int ret = i2c_read_blocking(i2c0, 0x57, &ok, 1, false);
		if ( ret != 1) {
		    uint64_t now = to_us_since_boot( get_absolute_time());

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

	i2c_write_blocking( i2c0, 0x57, address, 2, true);
        if ( i2c_read_blocking( i2c0, 0x57, data, 4, false) == 4) {
            printf("EEPROM begins %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3]);
        } else {
            printf("Failed to read EEPROM\n");
        }
	
    }
}


