//
// Commodore SID control from Raspberry Pi via TCA6416A (16bit I2C I/O expander)
//

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "i2c_lib.h"
#include "tca6416a.h"
#include "rpi-sid.h"


#define I2C_FILE_NAME "/dev/i2c-1"

int main() {
    int counter = 0;
    i2c_debug = 0;
    SID_msg_t SID_msg;

    global_i2c_1_io = TCA6416A_I2C_ADDR_LO;
     
    // Open a connection to the I2C userspace control file.
    if ((global_i2c_1_descriptor = open(I2C_FILE_NAME, O_RDWR)) < 0) {
        perror("Unable to open i2c control file");
        exit(1);
    }

    //set all lines of TCA6416A to output  
    set_i2c_register(global_i2c_1_descriptor, global_i2c_1_io, TCA6416A_CONFIG_BANK_0, 0);
    set_i2c_register(global_i2c_1_descriptor, global_i2c_1_io, TCA6416A_CONFIG_BANK_1, 0);

    //reset SID
    SID_via_tca6416_reset(global_i2c_1_descriptor, global_i2c_1_io);

    //start various synth threads 
    SID_synth_threads_init();

    //set sound parameters
    SID_msg.addr = SID_OSC1_ATTACK; SID_msg.data = 9;
    SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);
    SID_msg.addr = SID_OSC1_SUSTAIN; SID_msg.data = 0;
    SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);
    SID_msg.addr = SID_FLT_MODE_VOL; SID_msg.data = 31;
    SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);

    SID_msg.addr = SID_FLT_MODE_VOL; SID_msg.data = 31;
    SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);

    SID_msg.addr = SID_FLT_RESO_ROUTE; SID_msg.data = 17;
    SID_queue_one_msg(&global_SID_msg_queue, &SID_msg);

    //play some note pattern
    while(counter < 100)
     {
       SID_play_note(0x0449, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0893, NOTE_EIGHTH ,BPM160);
       SID_play_note(0x0449, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0893, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0449, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0893, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0449, NOTE_EIGHTH, BPM160);
       SID_play_note(0x0893, NOTE_EIGHTH, BPM160);
       counter++;
     }

    return 0;

  }
