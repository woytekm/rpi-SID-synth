//
// Controlling SID chip from Raspberry Pi via TCA6416A I2C I/O expander
//
// TCA6416A has 16 GPIO lines, and can convert logic levels between I2C and I/O pins - second feature makes
// it suitable for any project which requires Pi to interact with TTL logic (for example, SID).
//
// I'm using first 8 lines of TCA6416A for the SID data bus, and second 8 lines for address bus and control lines of SID
//

#include <stdio.h>
#include "rpi-sid.h"
#include "i2c_lib.h"
#include "tca6416a.h"


int SID_via_tca6416_reset(int i2c_dev, uint8_t ic_addr)
 {

    if(SID_DEBUG)
      printf("SID_via_tca6416_reset: SID reset...\n");
    
    //write reset bit pattern to TCA...

    set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_1, 0b10100000);
    set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_1, 0b00000000);
    usleep(10); // eight cycles should be enough, but...
    set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_1, 0b10100000);

    return 0;

 }


//
// Controlling SID through I2C 16-bit I/O expander requires to write data to address lines 
// _and_ control lines in one byte -  it's done by merging 5 bits of address, and
// three additional bits for control lines - CS, RST and R/W (always low for writes), 
// and then writing result to I2C.
//

int SID_via_tca6416_write(int i2c_dev, uint8_t ic_addr, uint8_t addr, uint8_t data)
 {
 
  uint8_t cshi_rsthi_mask=160; // CS = 1, RST = 1, [101xxxxx] where x = data bits
  uint8_t cslo_rsthi_mask=32;  // CS = 0, RST = 1, [001xxxxx] where x = data bits
 
  uint8_t out_addr = 0; 

  out_addr = cslo_rsthi_mask | addr; // merge five bits of address with control lines pattern (bitwise OR)

  if(SID_DEBUG)
   printf("SID_via_tca6416_write: writing address lines [%d]...\n",out_addr);

  set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_1, out_addr);

  if(SID_DEBUG)
   printf("SID_via_tca6416_write: writing data lines [%d]...\n",data);

  set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_0, data);

  usleep(2); // sleep two cycles of the clock - this should be enough to get CS=0, CLOCK=1 pattern

  out_addr = cshi_rsthi_mask | out_addr;  // clear write pattern (CS=1)

  if(SID_DEBUG)
   printf("SID_via_tca6416_write: setting CS to HI...\n",data);
  
  set_i2c_register(i2c_dev, ic_addr, TCA6416A_OUTPUT_BANK_1, out_addr);

 }


int SID_write_msg(int i2c_dev, uint8_t ic_addr, SID_msg_t SID_msg)
 {
   uint8_t msg_byte_lo, msg_byte_hi;
   uint16_t bit_mask = 255;

   msg_byte_lo = bit_mask & SID_msg.data;
   msg_byte_hi = SID_msg.data >> 8;

   if(SID_msg.data > 255)
    {
     SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg.addr, msg_byte_lo);
     SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg.addr+1, msg_byte_hi);
    }
   else
    SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg.addr, msg_byte_lo);

   return 0;
 }


int SID_play_note(int i2c_dev, uint8_t ic_addr, uint16_t note, uint8_t len, uint8_t bpm)
 {
  SID_msg_t SID_msg;
  uint32_t note_duration;
  int note_len_divider[4] = {120,60,30,15};

  note_duration = (note_len_divider[len] * 1000000) / bpm;

  SID_msg.addr = 0; 
  SID_msg.data = note;
  
  SID_write_msg(i2c_dev,TCA6416A_I2C_ADDR_LO, SID_msg );

  SID_msg.addr = 4;
  SID_msg.data = 17;

  SID_write_msg(i2c_dev,TCA6416A_I2C_ADDR_LO, SID_msg );

  usleep(note_duration);

  SID_msg.data = 16;
  SID_write_msg(i2c_dev,TCA6416A_I2C_ADDR_LO, SID_msg );
  return 0;
 }


