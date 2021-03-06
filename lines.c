/*
  Text line manipulation routines.

  This basically boils down to being able to read, modify, create and delete lines
  in text buffers.

  Of course this means that we need to work out which line termination character(s)
  we will use.  Ideally we should support them all, and use whatever a buffer already
  uses.  For simplicity for now, we will accept either CR or FL, and insert only CR
  for ease of cross-development and interoperability with UNIX-like systems.

*/

/*
  Find a given line in a buffer, returning its offset in the buffer
  Because memory is the main limiting factor, buffers are stored as linear slabs,
  necessitating a linear search.  We will in time cache the last found line, so that
  the search can continue in either direction from that point, instead of having to
  start from scratch.

  We could also automatically pre-calculate line addresses near to where the cursor is
  currently in a buffer, so that when a user does move around we already know where to,
  and the latency of the line search can be effectively hidden.

*/

#include "memory.h"
#include "buffers.h"
#include "screen.h"
#include "debug.h"

// Buffer for currently retrieved line
unsigned char line_buffer_buffer_id=0xff;
unsigned int line_buffer_line_number=0xffff;
unsigned char line_buffer[255];
unsigned char line_buffer_length=0;
unsigned char line_buffer_original_length=0;
unsigned char line_buffer_dirty=0;

// Information for current search for a line
unsigned char line_search_buffer[255];
unsigned char line_search_buffer_bytes;
unsigned char line_search_buffer_offset;
unsigned int line_offset_in_buffer,space_remaining;
unsigned char c;

// Information on the last line retrieved, to speed up search for
// successive lines
unsigned char last_buffer_id;
unsigned int last_line_number;
unsigned int last_line_offset_in_buffer;
unsigned int line_number_in;

unsigned int line_find_offset_backwards(unsigned char buffer_id,
					unsigned int line_number)
{
  /* If we are called, we are searching backward from last_line_number.
   */

  line_number=last_line_number-line_number;
  // we need to search back one line, to find the end of the previous line
  ++line_number;

  screen_hex(SCREEN_ADDRESS+80*2+60,line_offset_in_buffer);
  screen_decimal(SCREEN_ADDRESS+80*2+68,line_number);
  screen_decimal(SCREEN_ADDRESS+80*2+75,line_number_in);

  // Mark search buffer empty to begin with
  line_search_buffer_offset=255;
  
  while(line_number) {

    screen_decimal(SCREEN_ADDRESS,line_number);
    screen_hex(SCREEN_ADDRESS+6,line_search_buffer_offset);
    POKE(SCREEN_ADDRESS+0*80+16,PEEK(SCREEN_ADDRESS+0*80+16)+1);
    
    while(line_search_buffer_offset!=0) {
      // Keep loop as simple as possible
      POKE(SCREEN_ADDRESS+0*80+20,PEEK(SCREEN_ADDRESS+0*80+20)+1);
      --line_search_buffer_offset;
      if (line_search_buffer[line_search_buffer_offset]<14) {
	if ((line_search_buffer[line_search_buffer_offset]=='\r')
	    ||(line_search_buffer[line_search_buffer_offset]=='\n')) {
	  line_number--;
	  if (!line_number) {
	    // Here is where we want to be.
	    line_offset_in_buffer+=line_search_buffer_offset+1;
	    break;
	  }
	}
      }
    }
    if (!line_number) break;

    if (!line_offset_in_buffer) {      
      return 1;
    }

    // Make sure we have some bytes to work with
    {
      // We need to read some more bytes from the buffer to search
      space_remaining=line_offset_in_buffer;
      if (space_remaining<255) c=space_remaining; else c=255;
      buffer_get_bytes(buffer_id,line_offset_in_buffer,c,line_search_buffer);
      line_search_buffer_offset=c;
      line_search_buffer_bytes=c;
      
      line_offset_in_buffer-=c;
      
    }
    
  }

  if (!line_number) {
    // Remember line for next time
    last_buffer_id=buffer_id;
    last_line_number=line_number_in;
    last_line_offset_in_buffer=line_offset_in_buffer;
    // return address of line
    return line_offset_in_buffer;
  }

}


unsigned int line_find_offset(unsigned char buffer_id, unsigned int line_number)
{
  line_number_in=line_number;
  
  if (!buffers[buffer_id].loaded)
    if (buffer_load(buffer_id)) {
      display_footer(FOOTER_DISKERROR);
      return 0xffff;
    }
  
  // Start from the beginning of the buffer, and search forward.
  line_search_buffer_offset=0;
  line_search_buffer_bytes=0;
  if (!line_number) {
    // Short cut jumping to start of buffer
    return 0;
  }

#if 0
  // XXX - When we enable editing of buffers, we have to discard the last_line_number
  // stuff, or at least amend it, if lines are added, deleted or modified
  if ((last_buffer_id==buffer_id)&&(last_line_number>line_number)) {
    POKE(SCREEN_ADDRESS+24*80+1,PEEK(SCREEN_ADDRESS+24*80+1)+1);
  // But don't use if it would be better to search forwards from the beginning.
    if ((last_line_number-line_number)<line_number)
      return line_find_offset_backwards(buffer_id,line_number);
  }
#endif
    
  if ((last_buffer_id==buffer_id)&&(last_line_number<=line_number)) {
    // We have recently accessed a line before this one somewhere, so we can
    // re-use that as a starting point
    line_number-=last_line_number;
    line_offset_in_buffer=last_line_offset_in_buffer;
  } else {
    // No cached recent line to speed things up, so start at the beginning
    line_offset_in_buffer=0;
  }
  
  while(line_number) {
    
    // Return failure if we have reached the end of the buffer
    if (line_offset_in_buffer>=buffers[buffer_id].length) return 0xffff;

    // Quickly scan through the bytes we have
    while(line_search_buffer_offset!=line_search_buffer_bytes) {
      // Keep loop as simple as possible
      if (line_search_buffer[line_search_buffer_offset]<14) {
	if ((line_search_buffer[line_search_buffer_offset]=='\r')
	    ||(line_search_buffer[line_search_buffer_offset]=='\n')) {
	  line_number--;
	  if (!line_number) {
	    // Here is where we want to be.
	    line_offset_in_buffer+=line_search_buffer_offset+1;
	    break;
	  }
	}
      }
      ++line_search_buffer_offset;
    }
    if (!line_number) break;
    
    // Make sure we have some bytes to work with
    {
      // We need to read some more bytes from the buffer to search
      line_offset_in_buffer+=line_search_buffer_bytes;
      space_remaining=buffers[buffer_id].length-line_offset_in_buffer;
      if (space_remaining<255) c=space_remaining; else c=255;
      buffer_get_bytes(buffer_id,line_offset_in_buffer,c,line_search_buffer);
      line_search_buffer_offset=0;
      line_search_buffer_bytes=c;
    }
    
    if (line_search_buffer_offset>=line_search_buffer_bytes) {
      return 0xffff;
    }

  }
  if (!line_number) {
    // Remember line for next time
    last_buffer_id=buffer_id;
    last_line_number=line_number_in;
    last_line_offset_in_buffer=line_offset_in_buffer;
    // return address of line
    return line_offset_in_buffer;
  }
}

unsigned char buffer_list_header[80]=
  "filename          flags                 base    alloc   length lines            ";
// 01234567891111111111222222222233333333334444444444555555555566666666667777777777
//           0123456789012345678901234567890123456789012345678901234567890123456789

unsigned char line_fetch(unsigned char buffer_id, unsigned int line_number)  
{
  unsigned char i;
  
  if (!buffer_id) {
    /* It is the *buffer-list* buffer:
       Generate the line of information about the buffer slot.
    */
    unsigned char bid=line_number;

    // Start with spaces
    lfill((long)line_buffer,' ',255);
    line_buffer_length=80;

    if (!bid) {
      lcopy((long)buffer_list_header,(long)line_buffer,80);

      // Then first free buffer address
      screen_hex((unsigned int)&line_buffer[73],buffer_first_free_byte);

    } else if (buffers[bid].filename[0]) {      
      // File name
      for(i=0;buffers[bid].filename[i]&&(i<16);i++)
	line_buffer[i]=buffers[bid].filename[i];      
      
      // Then loaded and dirty flags
      if (buffers[bid].dirty) line_buffer[18]=0x44;
      if (buffers[bid].loaded) line_buffer[19]=0x4C;
      
      // Then buffer address (hex)
      screen_hex((unsigned int)&line_buffer[40],
		 buffers[bid].resident_address_low);
      //		 +(((long)buffers[bid].resident_address_high)<<16L));
      
      // Then allocated length (hex)
      screen_hex((unsigned int)&line_buffer[48],buffers[bid].allocated);
      
      // Then used length (decimal)
      screen_decimal((unsigned int)&line_buffer[56],buffers[bid].length);

      // Then number of lines in buffer (decimal)
      screen_decimal((unsigned int)&line_buffer[63],buffers[bid].line_count);
    }    
    // convert screen codes for hex to ASCII
    for(i=0;i<80;i++) if (line_buffer[i]<' ') line_buffer[i]|=0x60;
  } else {
    // Find the line in the buffer
    //if (last_buffer_id==buffer_id) {
      //if (last_line_number<=line_number)
        line_offset_in_buffer=line_find_offset(buffer_id,line_number);
      //else
        //line_offset_in_buffer=line_find_offset_backwards(buffer_id,line_number);
    //}
      //else
        //line_offset_in_buffer=line_find_offset(buffer_id,line_number);

    if (line_offset_in_buffer==0xffff) return 0xff;

    // Read it into the buffer
    buffer_get_bytes(buffer_id,line_offset_in_buffer,255,line_buffer);
    
    for(line_buffer_length=0;
	(line_buffer_length<0xff)
	  &&(line_buffer[line_buffer_length]!='\r')
	  &&(line_buffer[line_buffer_length]!='\n');line_buffer_length++)
      continue;
    if (line_buffer_length==0xff) {
      display_footer(FOOTER_LINETOOLONG);
      return 0xff;
    }
  }

  // We have the line, so set appropriate information
  line_buffer_line_number=line_number;
  line_buffer_buffer_id=buffer_id;
  line_buffer_dirty=0;

  // Fill past end of line in buffer with spaces for display convenience
  lfill((long)line_buffer+line_buffer_length,' ',255-line_buffer_length);  
  
  // Remember original length of line, so that it can be used for easier writing-back
  // of modified lines.
  line_buffer_original_length=line_buffer_length;
  return 0x00;
}
