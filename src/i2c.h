#ifndef I2C_IS_IN
#define I2C_IS_IN

#include "pico/stdlib.h"

void setup_i2c( uint scl, uint sda);
void probe_i2c( uint scl, uint sda);

int read_i2c_at_byte( uint8_t device, uint8_t addr, uint8_t *data, uint8_t maxBytes);
int read_i2c_at_short( uint8_t device, uint16_t addr, uint8_t *data, uint8_t maxBytes);

// sadly, you need to have your register address in the data, I'm not going to copy
// to make adjacent in memory.
int write_i2c( uint8_t device, const uint8_t *data, uint8_t bytes);

#endif
