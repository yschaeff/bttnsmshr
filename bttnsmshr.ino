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
        delay(100);
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
    strip.setBrightness(255);

    boottest();
}

void colorWipe(uint32_t color, int wait)
{
    for(unsigned int i=0; i<strip.numPixels(); i++) { // For each pixel in strip... 
        strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
        strip.show();                          //  Update strip to match     
        delay(wait);                           //  Pause for a moment       
    }
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

void game0()
{
    for (int i = 0; i < 6; i++) {
        /*digitalWrite(ctrls[i][LED], btn_state[i]);*/
        if (btn_event[i] && btn_state[i]) {
            pulse(ctrls[i][LED], HIGH, 300);
        }
    }
}

void game1()
{
    int N = 6;
    static int state = 0;
    static int players[6];
    static unsigned long start;
    static unsigned long dt;
    if (state == 0) {
        printf("state: %d (init)\n\r", state);
        memset(&players, 0, sizeof(int[6]));
        for (int i = 0; i < N; i++) {
            digitalWrite(ctrls[i][LED], LOW);
        }
        state++;
        printf("state: %d (select payers)\n\r", state);
    } else if (state == 1) {
        //add new players
        for (int i = 0; i < N; i++) {
            if (btn_state[i]) {
                players[i] = 1;
                digitalWrite(ctrls[i][LED], HIGH);
            }
        }
        //have all players released?
        int nplayers=0;
        int released = 1;
        for (int i = 0; i < N; i++) {
            if (!players[i]) continue;
            nplayers++;
            if (btn_state[i]) released = 0;
        }
        if (nplayers > 1 && released) {
            state++;
            printf("state: %d (starting)\n\r", state);
        }
    } else if (state == 2) {
        //signal begin
        for (int i = 0; i < N; i++) {
            if (!players[i]) continue;
            digitalWrite(ctrls[i][LED], LOW);
        }
        delay(300);
        for (int i = 0; i < N; i++) {
            if (!players[i]) continue;
            printf("player %d plays\n\r", i);
            digitalWrite(ctrls[i][LED], HIGH);
            /*if (players[i]) pulse(ctrls[i][LED], HIGH, 300);*/
        }
        state++;
        printf("state: %d (hold down!)\n\r", state);
    } else if (state == 3) {
        //every body hold down
        int all_pressed = 1;
        for (int i = 0; i < N; i++) {
            if (!players[i]) continue;
            /*digitalWrite(ctrls[i][LED], !btn_state[i]);*/
            if (!btn_state[i]) {
                all_pressed = 0;
            } else if (btn_event[i]) {
                blink(ctrls[i][LED], HIGH, 200);
            }
        }
        if (all_pressed) {
            state++;
            printf("state: %d (okay)\n\r", state);
            schedule_wipe();
            for (int i = 0; i < N; i++) {
                if (!players[i]) continue;
                digitalWrite(ctrls[i][LED], LOW);
            }
            delay(10); //forgive the bounces
        }
    } else if (state == 4) {
        //begun. no one release
        start = millis();
        dt = 3000;
        state++;
        printf("state: %d (keep holding)\n\r", state);
    } else if (state == 5) {
        if (millis() < start+dt) {
            //first to release loses
            int released = 0;
            for (int i = 0; i < N; i++) {
                if (!players[i]) continue;
                if (btn_event[i]) {
                    printf("player %d slips!\n\r", i);
                    released = 1;
                    break;
                }
            }
            if (released) {
                for (int i = 0; i < N; i++) {
                    if (!players[i]) continue;
                    digitalWrite(ctrls[i][LED], HIGH);
                }
                for (int i = 0; i < N; i++) {
                    if (!players[i]) continue;
                    if (!btn_state[i])
                        digitalWrite(ctrls[i][LED], LOW);
                    else
                        printf("player %d won!\n\r", i);
                }
                delay(3000);
                state = 0;
            }
        } else {
            state++;
            printf("state: %d (wait for it!)\n\r", state);
        }
    } else if (state == 6) {
        //begun. no one release
        start = millis();
        dt = 10000;
        for (int i = 0; i < N; i++) {
            digitalWrite(ctrls[i][LED], HIGH);
        }
        state++;
        printf("state: %d (release!)\n\r", state);
    } else if (state == 7) {
        if (millis() < start+dt) {
            //first to release wins
            int released = 0;
            for (int i = 0; i < N; i++) {
                if (!players[i]) continue;
                if (btn_event[i]) {
                    released = 1;
                    break;
                }
            }
            if (released) {
                for (int i = 0; i < N; i++) {
                    digitalWrite(ctrls[i][LED], LOW);
                }
                for (int i = 0; i < N; i++) {
                    if (!players[i]) continue;
                    if (!btn_state[i]) {
                        digitalWrite(ctrls[i][LED], HIGH);
                        printf("player %d won!\n\r", i);
                    }
                }
                delay(3000);
                state = 0;
            }
        } else { //timeout
            state = 0;
        }
    }
}

void loop()
{
    /*colorWipe(strip.Color(255,   0,   0), 1); // Red  */
    /*colorWipe(strip.Color(  0, 255,   0), 1); // Green*/
    /*colorWipe(strip.Color(  0,   0, 255), 1); // Blue */

    proc_bttns();
    if (1) {
        game0();
    } else {
        game1();
    }
    schedule_run();
}
