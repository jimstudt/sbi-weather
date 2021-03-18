#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "cli.h"
#include "wipe.h"

#define MAX_LINE 127

static void dispatch_command( const char *line);

void poll_cli(void) {
    static char line[MAX_LINE+1];
    static short lineLen = 0;

    while(true) {
	int ch = getchar_timeout_us(0);     // maybe use timeout to slow us down on poll loop?
	if ( ch == PICO_ERROR_TIMEOUT) break;

	switch(ch) {
	  case '\n':
	  case '\r':
	    putchar('\r');
	    putchar('\n');
	    if ( lineLen > 0 ) {
		line[lineLen] = 0;
		dispatch_command(line);
		lineLen = 0;
	    }
	    break;
	  default:
	    putchar(ch);
	    if ( lineLen < MAX_LINE) line[lineLen++] = ch;
	}
    }
}

static void dispatch_command( const char *line) {
    if ( memcmp( line, "reset", 5) == 0) {
	wipe();
    }
    printf("Dispatch: '%s'\n", line);
}
