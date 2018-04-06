#include <time.h>

void    send_debug(unsigned char *s, ...);
clock_t start_timer(void);
void    set_timer(clock_t t);
clock_t read_timer(void);
clock_t get_timer_diff(void);
