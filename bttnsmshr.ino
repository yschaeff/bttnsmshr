#include "scheduler.h"
#include "tasks.h"
#include "Adafruit_NeoPixel.h"
#include <EEPROM.h>

#define printf(fmt, ...)\
    do{snprintf(_pf_buffer_, sizeof(_pf_buffer_), fmt, ##__VA_ARGS__);Serial.print(_pf_buffer_);}while(0)
#define _PRINTF_BUFFER_LENGTH_ 64
static char _pf_buffer_[_PRINTF_BUFFER_LENGTH_];

#define btn_lights(state)\
    for (int i = 0; i < 6; i++) { digitalWrite(ctrls[i][LED], state); }


#define BTN 0
#define LED 1
static const int8_t ctrls[6][2] = {{8, 2},{9, 3},{10, 4},{11, 5},{12, 6},{7, 13}};
int btn_state[6];
int btn_event[6];

//LEDSTRAND
#define RGB_PIN A0
#define RGBC 30
Adafruit_NeoPixel strip(RGBC, RGB_PIN, NEO_GRB + NEO_KHZ800);

#define SPKR_PIN A2

#include "reaction.include"

void boottest()
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

void setup()
{
    for (int i = 0; i < 6; i++) {
        pinMode(ctrls[i][BTN], INPUT_PULLUP);
        pinMode(ctrls[i][LED], OUTPUT);
    }
    pinMode(SPKR_PIN, OUTPUT);
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
    static int8_t state = 0;
    static int8_t seq[15];
    static int8_t seqlen;
    static int8_t seqi;
    static int8_t highscore;
    if (state == 0) {
        highscore = EEPROM.read(0);
        if (highscore > 30) highscore = 0;
        randomSeed(millis());
        for (int i = 0; i < 15; i++) {
            seq[i] = random(0, 6);
        }
        seqlen = 1;
        seqi = 0;
        strip.clear();
        strip.show();
        for (int i = 0; i < 6; i++) {
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

        for (int i = 0; i < 6; i++) {
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
    static int8_t state = 0;
    if (state == 0) {
        schedule_insert(kit, 4000, 0, 15, millis());
        state++;
    }

    int ngames = 4;
    for (int i = 0; i < ngames; i++) {
        digitalWrite(ctrls[i][LED], HIGH);
    }
    for (int i = 0; i < 6; i++) {
        if (btn_event[i] && !btn_state[i]) {
            schedule_remove(kit);
            strip.clear();
            strip.show();
            return i;
        }
    }
    return -1;
}

void
beep(int pin, int state, int dt)
{
    /*if (state) {*/
        /*dt = millis()/440*/
    digitalWrite(pin, state);
    schedule_insert(beep, pin, !state, dt, millis()+dt);
}

void
music()
{
    static int8_t state = 0;
    if (state == 0) {
        schedule_insert(beep, SPKR_PIN, HIGH, 0, millis());
        schedule_insert(blink, ctrls[4][LED], HIGH, 500, millis());
        state++;
    }
}

void loop()
{
    static int8_t selection = -1;
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
        default:
            selection = game_selection();
            delay(10);//debounce
            if (selection >= 0) {
                for (int i = 0; i < 6; i++) {
                    digitalWrite(ctrls[i][LED], LOW);
                }
            }
    }

    schedule_run();
}
