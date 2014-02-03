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

#define SID_FLT_CUTOFF_LO 21
#define SID_FLT_CUTOFF_HI 22
#define SID_FLT_RESO_ROUTE 23
#define SID_FLT_MODE_VOL 24

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

#define BPM160 160
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

SID_msg_queue_t global_SID_msg_queue;

uint8_t global_i2c_1_descriptor;
uint8_t global_i2c_1_io;

//synth data structures

uint16_t software_LFO1_rate;
uint8_t  software_LFO1_shape;
uint8_t  software_LFO1_depth;
uint16_t  software_LFO1_routing;

uint16_t software_LFO2_rate;
uint8_t  software_LFO2_shape;
uint8_t  software_LFO2_depth;
uint16_t  software_LFO2_routing;

#define LFO_ROUTING_NONE 0
#define LFO_ROUTING_CUTOFF 1
#define LFO_ROUTING_RESO 2
#define LFO_ROUTING_PW1 4
#define LFO_ROUTING_PW2 8
#define LFO_ROUTING_PW3 16
#define LFO_ROUTING_DETUNE1 32
#define LFO_ROUTING_DETUNE2 64
#define LFO_ROUTING_DETUNE3 128

#define LFO_SHAPE_SINE 1
#define LFO_SHAPE_TRIANGLE 2
#define LFO_SHAPE_SAWTOOTH 4
#define LFO_SHAPE_SQUARE 8
#define LFO_SHAPE_SH 16

//SID register mirror (only for multipurpose registers)

uint8_t SID_OSC1_control_register;
uint8_t SID_OSC1_attack_decay_register;
uint8_t SID_OSC1_sustain_release_register;

uint8_t SID_OSC2_control_register;
uint8_t SID_OSC2_attack_decay_register;
uint8_t SID_OSC2_sustain_release_register;

uint8_t SID_OSC3_control_register;
uint8_t SID_OSC3_attack_decay_register;
uint8_t SID_OSC3_sustain_release_register;

uint8_t SID_reso_route_register;
uint8_t SID_mode_vol_register;

//prototypes

int SID_via_tca6416_reset(uint8_t i2c_dev, uint8_t ic_addr);
int SID_via_tca6416_write(uint8_t i2c_dev, uint8_t ic_addr, uint8_t addr, uint8_t data);
int SID_write_msg(uint8_t i2c_dev, uint8_t ic_addr, SID_msg_t *SID_msg);
int SID_play_note(uint16_t note, uint8_t len, uint8_t bpm);
int SID_queue_one_msg(SID_msg_queue_t *queue, SID_msg_t *SID_msg);
SID_msg_t *SID_dequeue_one_msg(SID_msg_queue_t *queue);

int SID_apply_filter_cutoff(uint16_t cutoff_value);
int SID_apply_filter_resonance(uint8_t resonance_value);

//end
