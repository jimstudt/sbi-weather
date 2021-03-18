#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "cli.h"
#include "wipe.h"

#define MAX_LINE 127

static void dispatch_command( const char *line, const struct command commands[] );

void poll_cli( const struct command commands[] ) {
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
		dispatch_command(line, commands);
		lineLen = 0;
	    }
	    break;
	  default:
	    putchar(ch);
	    if ( lineLen < MAX_LINE) line[lineLen++] = ch;
	}
    }
}

static void dispatch_command( const char *line, const struct command commands[] ) {
    for ( int i = 0; commands[i].name != 0; i++) {
	int len = strlen( commands[i].name);
	if ( memcmp( line, commands[i].name, len) == 0 &&
	     ( line[len] == 0 || line[len] == ' ' || line[len] == '\t')) {
	    (commands[i].func)(line);
	    return;
	}
    }
    
    printf("Unknown command: '%s'\n", line);
}
