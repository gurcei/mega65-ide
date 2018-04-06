#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "debug.h"

unsigned char mybyte;
unsigned char buff[256];
clock_t t0, t1;

clock_t start_timer(void)
{
  t0 = clock();
  return t0;
}

void set_timer(clock_t t)
{
  t0 = t;
}

clock_t read_timer(void)
{
  t1 = clock();
  return t1;
}

clock_t get_timer_diff(void)
{
  clock_t diff;
  read_timer();
  diff = t1 - t0;
  t0 = t1;
  return diff;
}

#pragma optimize(off)
void send_debug(unsigned char *s, ...)
{
  int k;
	va_list args;
	va_start(args, s);
	vsprintf(buff, s, args);
	va_end(args);

  // init M65 IO
  POKE(0xd02f,0x47);
  POKE(0xd02f,0x53);

  for (k = 0; buff[k] != '\0'; k++)
  {
    mybyte = buff[k];
    __asm__ ( "lda #$7C" );
    __asm__ ( "ldy %v", mybyte); // (unsigned char)s[k]);
    __asm__ ( "sta $D640" );
    __asm__ ( "nop" );
  }
  // add a carriage return at the end
    __asm__ ( "lda #$7C" );
    __asm__ ( "ldy #$0a" );
    __asm__ ( "sta $D640" );
    __asm__ ( "nop" );

  // reset it back to normal?
  //POKE(0xd031,0);
  // VIC-II IO mode
  //POKE(0xd02f,0);
}
#pragma optimize(on)

