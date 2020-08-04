#include "scheduler.h"
#include "tasks.h"
#include "Adafruit_NeoPixel.h"

#define printf(fmt, ...)\
    do{snprintf(_pf_buffer_, sizeof(_pf_buffer_), fmt, ##__VA_ARGS__);Serial.print(_pf_buffer_);}while(0)
#define _PRINTF_BUFFER_LENGTH_ 64
static char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];

#define BTN 0
#define LED 1
static const int8_t ctrls[6][2] = {{8, 2},{9, 3},{10, 4},{11, 5},{12, 6},{7, 13}};
int btn_state[6];
int btn_event[6];

//LEDSTRAND
#define RGB_PIN A0
#define RGBC 30
Adafruit_NeoPixel strip(RGBC, RGB_PIN, NEO_GRB + NEO_KHZ800);

#include "reaction.include"

void boottest()
{
    for (int i = 0; i < 6; i++) {
        digitalWrite(ctrls[i][LED], HIGH);
        delay(100);
        digitalWrite(ctrls[i][LED], LOW);
    }
    for(unsigned int i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
        strip.show();
        delay(50);
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}

void setup()
{
    for (int i = 0; i < 6; i++) {
        pinMode(ctrls[i][BTN], INPUT_PULLUP);
        pinMode(ctrls[i][LED], OUTPUT);
    }
    schedule_init();
    memset(&btn_state, 0, sizeof(int[6]));
    memset(&btn_event, 0, sizeof(int[6]));
    Serial.begin(9600);
    printf("EHLO\n\r");

    strip.begin();
    strip.show();
    strip.setBrightness(50);

    boottest();
}

void proc_bttns()
{
    int btn_pstate[6];
    memcpy(&btn_pstate, &btn_state, sizeof(int[6]));
    for (int i = 0; i < 6; i++) {
        if (ctrls[i][BTN] < 0) continue;
        btn_state[i] = !digitalRead(ctrls[i][BTN]);
        btn_event[i] = (btn_state[i] != btn_pstate[i]);
        if (btn_event[i]) {
            printf("%d:%d\n\r", ctrls[i][BTN], btn_state[i]);
        }
    }
}

void buttontest()
{
    for (int i = 0; i < 6; i++) {
        if (btn_event[i] && btn_state[i]) {
            pulse(ctrls[i][LED], HIGH, 300);
        }
    }
}

void simonsays()
{
    static int state = 0;
    static int seq[15];
    static int seqlen;
    static int seqi;
    if (state == 0) {
        randomSeed(millis());
        for (int i = 0; i < 15; i++) {
            seq[i] = random(0, 6);
            printf("%d ", seq[i]);
        }
        printf("\r\n");
        seqlen = 1;
        seqi = 0;
        for (int i = 0; i < 30; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        strip.show();
        for (int i = 0; i < 6; i++) {
            digitalWrite(ctrls[i][LED], LOW);
        }
        state++;
    } else if (state == 1) {
        digitalWrite(ctrls[seq[seqlen-1]][LED], HIGH);
        printf("seqlen: %d\n\r", seqlen);
        state++;
    } else if (state == 2) { //seqlen == seqi+1
        for (int i = 0; i < 30; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        strip.setPixelColor(seqlen-1, strip.Color(255, 0, 0));
        for (int i = 0; i < seqi; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
        }
        strip.show();

        for (int i = 0; i < 6; i++) {
            if (btn_event[i] && !btn_state[i]) {
                printf("key %d seqi %d seq[seqi] %d\n\r", i, seqi, seq[seqi]);
                if (i == seq[seqi]) {
                    digitalWrite(ctrls[seq[seqlen-1]][LED], LOW);
                    seqi++;
                    if (seqi == seqlen-1) {
                        printf("highlight last (next in seq:%d)\n\r", seqi);
                        state = 1;
                    } else if (seqi == seqlen) {
                        printf("reset sequence\n\r");
                        seqlen++;
                        seqi = 0;
                        state = 2;
                    } else {
                        printf("next in sequence\n\r");
                        state = 2;
                    }
                    break;
                } else {
                    printf("b0rk\n\r");
                    state = 0;
                    for (int i = 0; i < 30; i++) {
                        strip.setPixelColor(i, strip.Color(255, 0, 0));
                    }
                    strip.show();
                    delay(100);
                    break;
                }
            }
        }
        delay(20);
    }
}

void loop()
{
    proc_bttns();
    simonsays();
    /*if (1) {*/
        /*buttontest();*/
    /*} else {*/
        /*game1();*/
    /*}*/
    schedule_run();
}
