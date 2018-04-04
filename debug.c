#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

unsigned char mybyte;
unsigned char buff[256];

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

