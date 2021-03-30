
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include "cli.h"
#include "info.h"
#include "wipe.h"
#include "store.h"
#include "dht11.h"
#include "i2c.h"

const uint LED_PIN = 25;
const uint DHT11_PIN = 22;

const uint SDA_PIN = 20;
const uint SCL_PIN = 21;

static void probe(const char *cmd) {
    probe_dht11( DHT11_PIN);
    probe_i2c( SCL_PIN, SDA_PIN);
}

const struct command commands[] = { { "wipe", wipe },
				    { "info", print_info },
				    { "store", print_store },
				    { "probe", probe },
				    { 0,0} };
				   
int main() {
    bi_decl(bi_program_description("SBI Weather"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
    bi_decl(bi_2pins_with_func( SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));

    adc_init();
    adc_set_temp_sensor_enabled(true);
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stdio_init_all();

    while (true) {
	uint64_t now = to_us_since_boot( get_absolute_time());
	bool ledState = ( now/1000000)&1;
	
        gpio_put( LED_PIN, ledState);

	poll_cli( commands);
    }
    return 0;
}
