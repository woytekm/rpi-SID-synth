//
// RPi SID synth defines
//

#include <stdint.h>

#ifndef SID_DEBUG

#define SID_DEBUG 0

#endif

#define SID_OSC1_FREQ_LO 0
#define SID_OSC1_FREQ_HI 1
#define SID_OSC1_PW_LO 2
#define SID_OSC1_PW_HI 3
#define SID_OSC1_CTRL 4
#define SID_OSC1_ATTACK 5
#define SID_OSC1_SUSTAIN 6

#define SID_OSC2_FREQ_LO 7
#define SID_OSC2_FREQ_HI 8
#define SID_OSC2_PW_LO 9
#define SID_OSC2_PW_HI 10
#define SID_OSC2_CTRL 11
#define SID_OSC2_ATTACK 12
#define SID_OSC2_SUSTAIN 13

#define SID_OSC3_FREQ_LO 14
#define SID_OSC3_FREQ_HI 15
#define SID_OSC3_PW_LO 16
#define SID_OSC3_PW_HI 17
#define SID_OSC3_CTRL 18
#define SID_OSC3_ATTACK 19
#define SID_OSC3_SUSTAIN 20

#define SID_FLT_CUTTOF_LO 21
#define SID_FLT_CUTTOF_HI 22
#define SID_FLT_RESO_ROUTE 23
#define SID_FTL_MODE_VOL 24

#define SID_POTX 25
#define SID_POTY 26
#define SID_OSC3 27
#define SID_ENV3 28

#define SID_MSG_QUEUE_LEN 64

//some basic note and tempo definitions (mainly for testing purposes)

#define NOTE_HALF 0
#define NOTE_QUATER 1
#define NOTE_EIGHTH 2
#define NOTE_SIXTEENTH 3

#define BPM120 120
#define BPM100 100
#define BPM60 60

//data structures

typedef struct _SID_msg {
	uint8_t addr;
        uint16_t data;
        uint8_t prio;
       } SID_msg_t;

typedef struct  _SID_msg_queue_t{
        uint16_t rpos;
        uint16_t wpos;
        SID_msg_t SID_msg_pipe[SID_MSG_QUEUE_LEN];
       } SID_msg_queue_t;

typedef struct _I2C_data_t {
        uint16_t i2c_descriptor;
        uint8_t chip_addr;
       } I2C_data_t;

SID_msg_queue_t global_SID_msg_queue;
uint8_t global_i2c_1_descriptor;
uint8_t global_i2c_1_io;

// prototypes

int SID_via_tca6416_reset(uint8_t i2c_dev, uint8_t ic_addr);
int SID_via_tca6416_write(uint8_t i2c_dev, uint8_t ic_addr, uint8_t addr, uint8_t data);
int SID_write_msg(uint8_t i2c_dev, uint8_t ic_addr, SID_msg_t *SID_msg);
int SID_play_note(uint16_t note, uint8_t len, uint8_t bpm);

//end
