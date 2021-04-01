#ifndef BME280_IS_IN
#define BME280_IS_IN

struct bme280_sample_t {
    int32_t pressure;    // pascals
    int32_t humidity;    // 1024 * percentage
    int32_t temperature; // Celcius * 100
};

bool bme280_init( uint8_t deviceAddress);
bool bme280_sample( struct bme280_sample_t *sample);

#endif
