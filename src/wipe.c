
#include "wipe.h"
#include "pico/bootrom.h"

void wipe(const char *line) {
    reset_usb_boot(0,0);
}
