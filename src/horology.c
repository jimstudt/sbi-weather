#include "horology.h"

#include <stdio.h>
#include "hardware/rtc.h"


void time_command(const char *cmd) {
    int year,month,day,hour,min,sec;
    char fluff;
    int parts = sscanf( cmd, " time %d-%d-%d %d:%d:%d %c",
			&year, &month, &day, &hour, &min, &sec, &fluff);
    if ( parts == 6) {
	printf("Setting clock\n");
	datetime_t dt = { .year = year, .month = month, .day = day, .hour = hour, .min = min, .sec = sec };

	#if 0
	if (ds3231_set( &dt)) {
	    printf("DS3231 set\n");
	} else {
	    printf("DS3231 set FAILED\n");
	}
	#endif
	
	if (rtc_set_datetime(&dt)) {
	    printf("RTC set\n");
	} else {
	    printf("RTC set FAILED\n");
	}
    }
    
    if ( !rtc_running()) {
	datetime_t n = { .year = 2021, .month = 1, .day = 1, .hour = 12, .min = 34, .sec = 56 };
	rtc_set_datetime(&n);
    }

    if ( rtc_running()) {
	datetime_t t;

	rtc_get_datetime(&t);
	printf("RP2040 RTC says %04d-%02d-%02d %02d:%02d:%02d\n",
	       t.year, t.month, t.day, t.hour, t.min, t.sec);
    } else {
	printf("RP2040 RTC not valid\n");
    }
}
