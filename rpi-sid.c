//
// Controlling SID chip from Raspberry Pi via TCA6416A I2C I/O expander
//
// TCA6416A has 16 GPIO lines, and can convert logic levels between I2C and I/O pins - second feature makes
// it suitable for any project which requires Pi to interact with TTL logic (for example, SID).
//
// I'm using first 8 lines of TCA6416A for the SID data bus, and second 8 lines for address bus and control lines of SID
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "rpi-sid.h"
#include "i2c_lib.h"
#include "tca6416a.h"


void SID_init_synth(void)
 {
  software_LFO1_routing = software_LFO1_routing|LFO_ROUTING_CUTOFF;
  software_LFO2_routing = LFO_ROUTING_NONE;
 }


void software_LFO1(void)
 {

  uint16_t amplitude = 0;
  uint32_t step_interval = 100000;
  int step = 1;

  while(1)
   {

    usleep(step_interval);

    amplitude += step;

    if(amplitude == 255)
     step = -1;
    if(amplitude == 0)
     step = 1;

    if(software_LFO1_routing|LFO_ROUTING_CUTOFF)
     SID_apply_filter_cutoff(amplitude);

    if(software_LFO1_routing|LFO_ROUTING_RESO)
     SID_apply_filter_resonance(amplitude/16);
   }

 }

void software_LFO2(void)
 {

  uint16_t amplitude = 0;
  uint32_t step_interval = 100000;
  int step = 1;

  while(1)
   {

    usleep(step_interval);

    amplitude += step;

    if(amplitude == 255)
     step = -1;
    if(amplitude == 0)
     step = 1;
 
    if(software_LFO2_routing|LFO_ROUTING_CUTOFF)
     SID_apply_filter_cutoff(amplitude);

    if(software_LFO2_routing|LFO_ROUTING_RESO)
     SID_apply_filter_resonance(amplitude/16);

   }

 }


int SID_apply_filter_cutoff(uint16_t cutoff_value)
 {
  uint8_t cutoff_lo, cutoff_hi;
  uint16_t lo_mask = 7;
  SID_msg_t SID_FC_msg_hi;
  SID_msg_t SID_FC_msg_lo;
 
  SID_FC_msg_hi.addr = SID_FLT_CUTOFF_HI;
  SID_FC_msg_lo.addr = SID_FLT_CUTOFF_LO; 

  if(cutoff_value > 2047)  //11 bits max
    return -1;

  cutoff_lo = lo_mask & cutoff_value;
  cutoff_hi = cutoff_value >> 3;

  SID_FC_msg_lo.data = cutoff_lo;
  SID_FC_msg_hi.data = cutoff_hi;

  SID_queue_one_msg(&global_SID_msg_queue, &SID_FC_msg_lo);
  SID_queue_one_msg(&global_SID_msg_queue, &SID_FC_msg_hi);
 }


int SID_apply_filter_resonance(uint8_t resonance_value)
 {
   SID_msg_t SID_RES_msg;

   SID_RES_msg.data = (resonance_value << 4)|15;
   SID_RES_msg.addr = SID_FLT_RESO_ROUTE;
   SID_queue_one_msg(&global_SID_msg_queue, &SID_RES_msg);
 }


SID_msg_t *SID_dequeue_one_msg(SID_msg_queue_t *queue)
 {

  if(queue->rpos == queue->wpos)
   return NULL;

  if(queue->rpos == SID_MSG_QUEUE_LEN)
   queue->rpos = 0;
 
  queue->rpos++;

  if(SID_DEBUG)
   printf("SID_dequeue_one_msg: dequeued one SID msg on pos %d (%x, %x)\n", 
           queue->rpos-1, queue->SID_msg_pipe[queue->rpos-1].addr, queue->SID_msg_pipe[queue->rpos-1].data);
  return &queue->SID_msg_pipe[queue->rpos-1];

 }


int SID_queue_one_msg(SID_msg_queue_t *queue, SID_msg_t *SID_msg)
 {
  
   if((queue->wpos == SID_MSG_QUEUE_LEN) && (queue->rpos == 0))
    { 
      if(SID_DEBUG)
       printf("SID_queue_one_msg: error: cannot queue more messages! queue overflow!\n");
      return -1;
    }

   if(queue->wpos == (queue->rpos - 1))
    {
      if(SID_DEBUG)
       printf("SID_queue_one_msg: error: cannot queue more messages! we are just behind dequeuing pointer!\n");
      return -1;
    }

   if(queue->wpos == SID_MSG_QUEUE_LEN)
    queue->wpos = 0;
   
   memcpy(&queue->SID_msg_pipe[queue->wpos], SID_msg, sizeof(SID_msg_t));

   queue->wpos++;

   if(SID_DEBUG)
    printf("SID_queue_one_msg: queued msg on pos %d (%x, %x)\n",queue->wpos-1,SID_msg->addr, SID_msg->data);

   return 0;
 }


void SID_msg_pipe_tx(void *arg)  //thread
{

  uint16_t dequeued = 0;
  SID_msg_t *SID_msg_out;

  while( 1 )
   {
    usleep(10);

    SID_msg_out = SID_dequeue_one_msg(&global_SID_msg_queue);

    if(SID_msg_out != NULL)
     {
      SID_write_msg(global_i2c_1_descriptor, global_i2c_1_io, SID_msg_out);
      dequeued++;
     }

   }

}

int SID_synth_threads_init(void)
{

   #define SYNTH_THREADS 10

   pthread_t threads[SYNTH_THREADS];
   int rc;
   long t;
   
   //here we will start various threads such as: message queue tx, software LFO's, MIDI IN routine, etc...

   global_SID_msg_queue.rpos = 0;
   global_SID_msg_queue.wpos = 0;
   
   rc = pthread_create(&threads[0], NULL, SID_msg_pipe_tx, (void *)t);
   rc = pthread_create(&threads[1], NULL, software_LFO1, (void *)t);
   rc = pthread_create(&threads[1], NULL, software_LFO2, (void *)t);

}


int SID_via_tca6416_reset(uint8_t i2c_dev, uint8_t ic_addr)
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

int SID_via_tca6416_write(uint8_t i2c_dev, uint8_t ic_addr, uint8_t addr, uint8_t data)
 {

  uint8_t cshi_rsthi_mask=160; // CS = 1, RST = 1, [101xxxxx] where x = address bits
  uint8_t cslo_rsthi_mask=32;  // CS = 0, RST = 1, [001xxxxx] where x = address bits
 
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


int SID_write_msg(uint8_t i2c_dev, uint8_t ic_addr, SID_msg_t *SID_msg)
 {

   uint8_t msg_byte_lo, msg_byte_hi;
   uint16_t bit_mask = 255;

   msg_byte_lo = bit_mask & SID_msg->data;
   msg_byte_hi = SID_msg->data >> 8;

   if(SID_DEBUG)
    printf("SID_write_msg: about to write %x, %x into SID...",SID_msg->data, SID_msg->addr);

   if(SID_msg->data > 255)
    {
     SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg->addr, msg_byte_lo);
     SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg->addr+1, msg_byte_hi);
    }
   else
    SID_via_tca6416_write(i2c_dev, ic_addr, SID_msg->addr, msg_byte_lo);

   return 0;
 }


int SID_play_note(uint16_t note, uint8_t len, uint8_t bpm)
 {

  SID_msg_t SID_msg;
  uint32_t note_duration;
  int note_len_divider[4] = {120,60,30,15};

  note_duration = (note_len_divider[len] * 1000000) / bpm;

  SID_msg.addr = SID_OSC1_FREQ_LO; 
  SID_msg.data = note;
  
  SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);

  SID_msg.addr = SID_OSC1_CTRL;
  SID_msg.data = 17;

  SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);

  usleep(note_duration);

  SID_msg.data = 16;
  SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);
  return 0;
 }


