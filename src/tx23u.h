#ifndef TX23U_IS_IN
#define TX23U_IS_IN

struct tx23u_vector_t {
	uint16_t speed;   // cm/Sec
	uint8_t direction;
};

struct tx23u_sample_t {
    struct tx23u_vector_t average;
    struct tx23u_vector_t gust;
};    

enum tx23u_state {
    TX23_IDLE=0,
    TX23_COLLECTING,
    TX23_START1,
    TX23_START2,
    TX23_DATA
};


struct tx23u_t {
    uint8_t pin;
    bool sampleValid;
    struct tx23u_sample_t sample;
    void (*handler)(struct tx23u_t *device);

    // Internal stuff below, hands off
    uint8_t packet[5];
    uint8_t bit;
    bool gust;
    bool seenPreamble;
    enum tx23u_state state;
    uint32_t bitTime;
};

void probe_tx23u( const unsigned char pin);

void tx23u_init( struct tx23u_t *device, uint8_t pin, void (*handler)(struct tx23u_t *device));

bool tx23u_sample( uint8_t pin, struct tx23u_sample_t *sample);

void tx23u_print( struct tx23u_sample_t *sample);

void tx23u_pulse( struct tx23u_t *device, bool isHigh, uint32_t usecs);
void tx23u_reset( struct tx23u_t *device);


#endif
