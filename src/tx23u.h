#ifndef TX23U_IS_IN
#define TX23U_IS_IN

void probe_tx23u( const unsigned char pin);

void tx23u_pulse( uint32_t high, uint32_t low);
void tx23u_reset(void);


#endif
