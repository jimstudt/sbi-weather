
#include "info.h"
#include <stdio.h>
#include "hardware/adc.h"
#include "pico/unique_id.h"

/*
** NOTE THEE WELL:
**
** You must do these before using this function...
** 
**  adc_init();
**  adc_set_temp_sensor_enabled(true);
**
** You will get ridiculous temperature values if you don't enable
** the temperature sensor. It also might take a little bit to converge
** on something close to right, so don't be tricky and snap it on and
** off to save power.
**
*/

#define REFERENCE_VOLTAGE 3.3

void print_info(const char *line) {
    {
	adc_select_input(4);
	uint16_t bits = adc_read();
	float voltage = bits * REFERENCE_VOLTAGE / 4095;
	float temperature = 27.0 - ( voltage - 0.706) / 0.001721;

	printf("Internal temperature: %4.1fC\n", temperature);
    }

    {
	pico_unique_board_id_t board_id;
	pico_get_unique_board_id(&board_id);

	    printf("Board identifier:");
	    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i += 2) {
		printf("%c%02x%02x", (i==0 ? ' ' : ':'), board_id.id[i], board_id.id[i+1]);
	    }
	    printf("\n");
    }

}
