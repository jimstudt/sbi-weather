#ifndef DS3231_IS_IN
#define DS3231_IS_IN

#include "hardware/rtc.h"

bool ds3231_set( const datetime_t *dt);
bool ds3231_get( datetime_t *dt);

#endif
