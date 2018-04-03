#include <stdio.h>
#include "memory.h"
#include "buffers.h"
#include "screen.h"
#include "windows.h"
#include "input.h"

unsigned char mybyte;

#pragma optimize(off)
void send_debug(unsigned char *s)
{
  int k;

  // init M65 IO
  POKE(0xd02f,0x47);
  POKE(0xd02f,0x53);

  for (k = 0; s[k] != '\0'; k++)
  {
    mybyte = s[k];
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


void main(void)
{  
  // Set keyboard IRQ faster, so that cursor movement etc is faster
  // (approximately 2x normal speed)
  POKE(0xdc05U,0x20);

  setup_screen();
  initialise_windows();
  initialise_buffers();

  send_debug("hello, how are you today?");

  while(!poll_keyboard());

  // Exit nicely

  // 1MHz, 40 column
  c65_io_enable();
  POKE(0xd031,0);
  // VIC-II IO mode
  POKE(0xd02f,0);
  // Reset machine
  __asm__ ( "jmp $fce2" );

}
