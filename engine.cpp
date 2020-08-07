#include <Arduino.h>
#include <EEPROM.h>
#include "scheduler.h"
#include "tasks.h"
#include "Adafruit_NeoPixel.h"
#include "engine.h"
#include "debounce.h"

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
int btn_state[N_BUTTONS];
int btn_event[N_BUTTONS];

//LEDSTRAND
#define RGB_PIN A0
#define RGBC 30
Adafruit_NeoPixel strip(RGBC, RGB_PIN, NEO_GRB + NEO_KHZ800);

#define SPKR_PIN A2

#include "reaction.include"

static void
boottest()
{
    btn_lights(HIGH);
    delay(150);
    btn_lights(LOW);
    delay(150);
    strip.fill(strip.Color(255, 255, 255), 15, 15);
    strip.show();
    delay(150);
    strip.clear();
    strip.show();
    delay(150);
    strip.fill(strip.Color(255, 255, 255), 0, 15);
    strip.show();
    delay(150);
    strip.clear();
    strip.show();
}

void
mainsetup()
{
    for (int i = 0; i < N_BUTTONS; i++) {
        pinMode(ctrls[i][BTN], INPUT_PULLUP);
        pinMode(ctrls[i][LED], OUTPUT);
    }
    pinMode(SPKR_PIN, OUTPUT);
    schedule_init();
    memset(&btn_state, 0, sizeof(int[N_BUTTONS]));
    memset(&btn_event, 0, sizeof(int[N_BUTTONS]));
    Serial.begin(9600);
    printf("EHLO\n\r");

    strip.begin();
    strip.show();
    strip.setBrightness(50);

    boottest();
}

static void
proc_bttns()
{
    int btn_pstate[N_BUTTONS];
    memcpy(&btn_pstate, &btn_state, sizeof(int[N_BUTTONS]));
    for (int i = 0; i < N_BUTTONS; i++) {
        if (ctrls[i][BTN] < 0) continue;
        btn_state[i] = !digitalRead(ctrls[i][BTN]);
        btn_event[i] = (btn_state[i] != btn_pstate[i]);
        //if (btn_event[i]) {
            //printf("%d:%d\n\r", ctrls[i][BTN], btn_state[i]);
        //}
    }
}

static void
buttontest()
{
    for (int i = 0; i < N_BUTTONS; i++) {
        if (btn_event[i] && btn_state[i]) {
            pulse(ctrls[i][LED], HIGH, 300);
        }
    }
}

static void
simonsays()
{
    static int8_t state = 0;
    static int8_t seq[15];
    static int8_t seqlen;
    static int8_t seqi;
    static int8_t highscore;
    if (state == 0) {
        highscore = EEPROM.read(0);
        if (highscore > 30) highscore = 0;
        for (int i = 0; i < 15; i++) {
            seq[i] = random(0, N_BUTTONS);
        }
        seqlen = 1;
        seqi = 0;
        strip.clear();
        strip.show();
        for (int i = 0; i < N_BUTTONS; i++) {
            digitalWrite(ctrls[i][LED], LOW);
        }
        state++;
    } else if (state == 1) {
        digitalWrite(ctrls[seq[seqlen-1]][LED], HIGH);
        state++;
    } else if (state == 2) { //seqlen == seqi+1
        strip.clear();
        strip.setPixelColor(seqlen-1, strip.Color(255, 0, 0));
        strip.setPixelColor(highscore, strip.Color(0, 0, 255));
        /*strip.fill(strip.Color(0, 255, 0), 0, seqi);*/
        for (int i = 0; i < seqi; i++) {
            strip.setPixelColor(i, strip.Color(0, 255, 0));
        }
        strip.show();

        for (int i = 0; i < N_BUTTONS; i++) {
            if (btn_event[i] && !btn_state[i]) {
                if (i == seq[seqi]) {
                    digitalWrite(ctrls[seq[seqlen-1]][LED], LOW);
                    seqi++;
                    if (seqi > highscore) {
                        highscore = seqi;
                        EEPROM.write(0, highscore);
                    }
                    if (seqi == seqlen-1) {
                        state = 1;
                    } else if (seqi == seqlen) {
                        seqlen++;
                        if (seqlen > 15) {
                            state = 0;
                            strip.fill(strip.Color(0, 255, 0), 0, strip.numPixels());
                            strip.show();
                            delay(3000);

                        }
                        seqi = 0;
                    }
                    break;
                } else {
                    state = 0;
                    strip.fill(strip.Color(255, 0, 0), 0, strip.numPixels());
                    strip.show();
                    delay(1000);
                    break;
                }
            }
        }
        delay(20);
    }
}

static int8_t
game_selection()
{
    int ngames = 5;
    static int8_t state = 0;
    if (state == 0) {
        schedule_insert(kit, 4000, 0, 15, millis());
        for (int i = 0; i < ngames; i++) {
            schedule_insert(glitch_task, ctrls[i][LED], HIGH, 0, millis());
        }
        state++;
    }

    for (int i = 0; i < N_BUTTONS; i++) {
        if (btn_event[i] && !btn_state[i]) {
            schedule_remove(kit);
            schedule_remove(glitch_task);
            strip.clear();
            strip.show();
            for (int i = 0; i < N_BUTTONS; i++) {
                digitalWrite(ctrls[i][LED], LOW);
            }
            randomSeed(millis());
            return i;
        }
    }
    return -1;
}

static void
beep(int pin, int state, int dt)
{
    digitalWrite(pin, state);
    schedule_insert(beep, pin, !state, dt, millis()+dt);
}

static void
music()
{
    static int8_t state = 0;
    if (state == 0) {
        schedule_insert(beep, SPKR_PIN, HIGH, 0, millis());
        schedule_insert(blink, ctrls[4][LED], HIGH, 500, millis());
        state++;
    }
}

static void
tetris_tick(int barptr, int dt, int arg3)
{
    const int8_t barlen = 15;
    static char *bar = (char *)barptr;
    int c;
    int cc = 0;
    int stable = 1;
    for (int i = 0; i < barlen-1; i++) {
        if (bar[i] == 0) {
            if (bar[i+1]) stable = 0;
            bar[i] = bar[i+1];
            bar[i+1] = 0;
            cc = 0;
        } else if (bar[i] == c) {
            cc++;
            if (cc == 3) {
                bar[i] = 0;
                bar[i-1] = 0;
                bar[i-2] = 0;
                cc = 0;
            }
        } else {
            c = bar[i];
            cc = 1;
        }
    }
    if (stable) {
        if (bar[barlen-1]) {
            for (int i = 0; i < barlen; i++)
                bar[i] = 1;
        } else {
            bar[barlen-1] = random(1,4);
        }
    }
    schedule_insert(tetris_tick, barptr, dt, arg3, millis()+dt);
}

static void
tetris_display_update(int dt, int barptr, int barlen)
{
    static char *bar = (char *)barptr;
    for (int i = 0; i < barlen; i++) {
        switch (bar[i]) {
        case 0:
            strip.setPixelColor(i, strip.Color(0, 0, 0));
            break;
        case 1:
            strip.setPixelColor(i, strip.Color(255, 0, 0));
            break;
        case 2:
            strip.setPixelColor(i, strip.Color(0, 255, 0));
            break;
        case 3:
            strip.setPixelColor(i, strip.Color(0, 0, 255));
        }
    }
    strip.show();
    schedule_insert(tetris_display_update, dt, barptr, barlen, millis()+dt);
}

static void
tetris(int init)
{
    int8_t colorcount = 3;
    const int8_t barlen = 15;
    static char bar[barlen];
    if (init) {
        for (int i = 0; i < barlen; i++)
            bar[i] = 0;
        schedule_insert(tetris_tick, (int)bar, 200, -1, millis());
        schedule_insert(tetris_display_update, 20, (int)bar, barlen, millis());
    }
    int8_t pinx = -1;
    for (int i = barlen-1; i >= 0; i--) {
        if(bar[i]) {
            pinx = i;
            break;
        }
    }
    int btn1 = debounce(btn_state[0], 20, 0) && btn_state[0];
    int btn2 = debounce(btn_state[1], 20, 1) && btn_state[1];
    int stable = (pinx == 0 || bar[pinx-1]);
    if (!stable) {
        if (pinx >= 0) {
            if (btn1) {
                bar[pinx] %= colorcount;
                bar[pinx]++;
            }
            if (btn2) {
                int8_t freex = -1;
                for (int i = pinx-1; i >= 0; i--) {
                    if (bar[i]) break;
                    freex = i;
                }
                if (freex >= 0) {
                    bar[freex] = bar[pinx];
                    bar[pinx] = 0;
                }
            }
        }
    }
}

void
mainloop()
{
    static int8_t selection = -1;
    static int8_t init = 1;
    proc_bttns();

    switch (selection) {
        case 0:
            simonsays();
            break;
        case 1:
            buttontest();
            break;
        case 2:
            game1();
            break;
        case 3:
            music();
            break;
        case 4:
            tetris(init);
            init = 0;
            break;
        default:
            selection = game_selection();
            delay(10);//debounce
            if (selection >= 0) {
                for (int i = 0; i < N_BUTTONS; i++) {
                    digitalWrite(ctrls[i][LED], LOW);
                }
                init = 1;
            }
    }

    schedule_run();
}
