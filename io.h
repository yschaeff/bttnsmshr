#ifndef IO_H
#define IO_H

#define printf(fmt, ...)\
    do{snprintf(_pf_buffer_, sizeof(_pf_buffer_), fmt, ##__VA_ARGS__);Serial.print(_pf_buffer_);}while(0)
#define _PRINTF_BUFFER_LENGTH_ 64
static char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];

#define N_BUTTONS 6

#define btn_lights(state)\
    for (int i = 0; i < N_BUTTONS; i++) { digitalWrite(ctrls[i][LED], state); }

#define BTN 0
#define LED 1
static const int8_t ctrls[N_BUTTONS][2] = {{8, 7},{9, 2},{10, 3},{11, 4},{12, 5},{13, 6}};
extern bool btn_state[N_BUTTONS];
extern bool btn_event[N_BUTTONS];

//LEDSTRAND
#include "Adafruit_NeoPixel.h"
#define RGB_PIN A0
#define RGBC 30
extern Adafruit_NeoPixel strip;

#define SPKR_PIN A2

#endif
