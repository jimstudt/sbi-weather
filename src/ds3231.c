
#include "ds3231.h"

#include "i2c.h"

#include <stdio.h>

static const uint8_t DS3231_ADDR = 0x68;

bool ds3231_set( const datetime_t *dt) {
    uint8_t data[8];

    data[0] = 0;      // register address of write
    data[1] = dt->sec % 10 + ( ( dt->sec / 10) << 4);
    data[2] = dt->min % 10 + ( ( dt->min / 10) << 4);
    data[3] = dt->hour % 10 + ( ( dt->hour / 10) << 4);  // will be 24 hour format
    data[4] = dt->dotw;
    data[5] = dt->day % 10 + ( ( dt->day / 10) << 4);
    data[6] = dt->month + ( dt->year >= 2000 ? 0x80 : 0);
    data[7] = dt->year % 10 + ( ( (dt->year / 10) % 10) << 4);
    
    return write_i2c( DS3231_ADDR, data, 8) == 8;
}

bool ds3231_get( datetime_t *dt) {
    uint8_t data[7];

    if ( read_i2c_at_byte( DS3231_ADDR, 0x00, data, 7) == 7) {
	dt->sec = (data[0] & 0x0f) + 10 * (data[0]>>4);
	dt->min = (data[1] & 0x0f) + 10 * (data[1]>>4);
	if ( data[2] & 0x40 ) {  // 12 hour
	    dt->hour = (data[2] & 0x0f) + ( (data[2] & 0x10) ? 10 : 0) + ( (data[2] & 0x20) ? 12 : 0);
	} else {                 // 24 hour
	    dt->hour = (data[2] & 0x0f) + 10 * ( (data[2] & 0x30) >> 4);
	}
	dt->dotw = data[3];
	dt->day = (data[4] & 0x0f) + 10 * ( data[4]>>4);
	dt->month = ( data[5] & 0x1f);
	dt->year = (( data[5] & 0x80) ? 2000 : 1900) + 10 * ( (data[6]>>4) & 0x0f) + (data[6] & 0x0f);

	return true;
    }	
    return false;
}
