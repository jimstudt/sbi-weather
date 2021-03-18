#ifndef CLI_IS_IN
#define CLI_IS_IN

struct command {
    const char *name;
    void (*func)(const char *line);
};
    
void poll_cli( const struct command commands[] );

#endif
