#ifndef __IO_H
#define __IO_H

#define printf(fmt, ...)\
    do{snprintf(_pf_buffer_, sizeof(_pf_buffer_), fmt, ##__VA_ARGS__);Serial.print(_pf_buffer_);}while(0)
#define _PRINTF_BUFFER_LENGTH_ 64
static char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];

#define N_BUTTONS 6

#define btn_lights(state)\
    for (int i = 0; i < N_BUTTONS; i++) { digitalWrite(ctrls[i][LED], state); }

#define BTN 0
#define LED 1
static const int8_t ctrls[N_BUTTONS][2] = {{8, 2},{9, 3},{10, 4},{11, 5},{12, 6},{7, 13}};
extern int btn_state[N_BUTTONS];
extern int btn_event[N_BUTTONS];

//LEDSTRAND
#include "Adafruit_NeoPixel.h"
#define RGB_PIN A0
#define RGBC 30
extern Adafruit_NeoPixel strip;

#define SPKR_PIN A2

#endif
