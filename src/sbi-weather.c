
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/rtc.h"

#include "cli.h"
#include "info.h"
#include "wipe.h"
#include "store.h"
#include "dht11.h"
#include "i2c.h"
#include "horology.h"
#include "bme280.h"

const uint LED_PIN = 25;
const uint DHT11_PIN = 22;

const uint SDA_PIN = 20;
const uint SCL_PIN = 21;

const uint8_t BME280_DEVICE_ADDRESS = 0x77;

static void probe(const char *cmd) {
    probe_dht11( DHT11_PIN);
    probe_i2c( SCL_PIN, SDA_PIN);
}

const struct command commands[] = { { "wipe", wipe },
				    { "info", print_info },
				    { "store", print_store },
				    { "probe", probe },
				    { "time", time_command },
				    { 0,0} };
				   
int main() {
    bi_decl(bi_program_description("SBI Weather"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    bi_decl(bi_2pins_with_func( SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));

    setup_i2c( SCL_PIN, SDA_PIN);
    
    horology_init();

    adc_init();
    adc_set_temp_sensor_enabled(true);
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    bme280_init( BME280_DEVICE_ADDRESS);
    
    stdio_init_all();

    uint64_t nextSample = time_us_64()/1000000 + 1;
    const uint32_t samplePeriod = 1;
    
    while (true) {
	uint64_t now = time_us_64();
	uint64_t second = now/1000000;
	
	bool ledState = ( now/1000000)&1;
	
        gpio_put( LED_PIN, ledState);

	poll_cli( commands);

	if ( second >= nextSample) {
	    nextSample = second + samplePeriod;

	    // get sensor data
	    struct bme280_sample_t b;
	    if ( bme280_sample(&b)) {
		printf("BME280: %.2fC %.2f%%rH %ldPa\n", b.temperature/100.0, b.humidity/1024.0, b.pressure);
	    } else {
		printf("Failed to sample BME280\n");
	    }
	    
	    // send sensor data
	}
    }
    return 0;
}
