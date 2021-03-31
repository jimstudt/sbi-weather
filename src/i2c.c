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
}


