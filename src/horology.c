#include "horology.h"

#include <stdio.h>
#include "hardware/rtc.h"
#include "ds3231.h"

// Should be const by rtc_set_datetime doesn't like that.
static datetime_t epoch = { .year = 1970, .month = 1, .day = 1, .hour = 0, .min = 0, .sec = 0, .dotw = 4 };

static bool horology_initialized = false;

void horology_init(void) {
    if ( horology_initialized ) return;
    
    rtc_init();

    datetime_t n;
    if ( !ds3231_get(&n)) {
	printf("Failed to load time from DS3231\n");
	rtc_set_datetime(&epoch);
    } else {
	rtc_set_datetime(&n);
    }

    horology_initialized = true;
}

void time_command(const char *cmd) {
    if ( !horology_initialized) horology_init();
    
    int year,month,day,hour,min,sec;
    char fluff;
    int parts = sscanf( cmd, " time %d-%d-%d %d:%d:%d %c",
			&year, &month, &day, &hour, &min, &sec, &fluff);
    if ( parts == 6) {
	printf("Setting clock\n");
	datetime_t dt = { .year = year, .month = month, .day = day, .hour = hour, .min = min, .sec = sec };

	if (ds3231_set( &dt)) {
	    printf("DS3231 set\n");
	} else {
	    printf("DS3231 set FAILED\n");
	}
	
	if (rtc_set_datetime(&dt)) {
	    printf("RTC set\n");
	} else {
	    printf("RTC set FAILED\n");
	}
    }
    
    if ( rtc_running()) {
	datetime_t t;

	if ( rtc_get_datetime(&t)) {
	    printf("RP2040 RTC says %04d-%02d-%02d %02d:%02d:%02d\n",
		   t.year, t.month, t.day, t.hour, t.min, t.sec);
	} else {
	    printf("RP2040 RTC gettime failed\n");
	}
    } else {
	printf("RP2040 RTC not valid\n");
    }
}
