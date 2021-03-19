
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include "cli.h"
#include "info.h"
#include "wipe.h"
#include "store.h"

const uint LED_PIN = 25;

const struct command commands[] = { { "wipe", wipe },
				    { "info", print_info },
				    { "store", print_store },
				    { 0,0} };
				   
int main() {
    bi_decl(bi_program_description("SBI Weather"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

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
