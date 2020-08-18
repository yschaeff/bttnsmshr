#include <Arduino.h>
#include <EEPROM.h>
#include "scheduler.h"
#include "tasks.h"
#include "Adafruit_NeoPixel.h"
#include "engine.h"
#include "debounce.h"
#include "io.h"
#include "reaction.h"

int btn_state[N_BUTTONS];
int btn_event[N_BUTTONS];
Adafruit_NeoPixel strip(RGBC, RGB_PIN, NEO_GRB + NEO_KHZ800);

static void
post()
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

    post();
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

struct tetris_state {
    char bar[RGBC];
    uint32_t shadow[RGBC];
};

static uint32_t
index2color(int index)
{
    switch (index) {
        case 0: return strip.Color(0, 0, 0);
        case 1: return strip.Color(255, 0, 0);
        case 2: return strip.Color(0, 255, 0);
        case 3: return strip.Color(0, 0, 255);
        default: return strip.Color(255, 0, 255);
    }
}

static void
tetris_tick(int stateptr, int dt, int arg3)
{
    struct tetris_state *state = (struct tetris_state *)stateptr;
    const int8_t barlen = 15;
    char *bar = state->bar;
    uint32_t *shadow = state->shadow;
    for (int player = 0; player < 2; player++) {
        int c;
        int cc = 0;
        int stable = 1;
        for (int i = player*barlen; i < player*barlen+barlen-1; i++) {
            if (bar[i] == 0) {
                if (bar[i+1]) stable = 0;
                bar[i] = bar[i+1];
                shadow[i+1] = index2color(bar[i+1]);
                bar[i+1] = 0;
                cc = 0;
            } else if (bar[i] == c) {
                cc++;
                if (cc == 3) {
                    shadow[i] = index2color(bar[i]);
                    shadow[i-1] = index2color(bar[i-1]);
                    shadow[i-2] = index2color(bar[i-2]);
                    bar[i] = 0;
                    bar[i-1] = 0;
                    bar[i-2] = 0;
                    cc = 0;
                    int other = !player;
                    char m = random(4, 255);
                    for (int j = other*barlen; j < other*barlen+barlen; j++) {
                        char n = bar[j];
                        bar[j] = m;
                        m = n;
                    }
                }
            } else {
                c = bar[i];
                cc = 1;
            }
        }
        if (stable) {
            if (bar[player*barlen+barlen-1]) {
                for (int i = player*barlen; i < player*barlen+barlen; i++)
                    bar[i] = (i&1)+1;
            } else {
                bar[player*barlen+barlen-1] = random(1,4);
            }
        }
    }
    schedule_insert(tetris_tick, stateptr, dt, arg3, millis()+dt);
}

static int
fade(uint32_t color, float ratio)
{
    uint8_t
      r = (uint8_t)(color >> 16),
      g = (uint8_t)(color >>  8),
      b = (uint8_t)color;
    r = r<10?0:r-10;
    g = g<10?0:g-10;
    b = b<10?0:b-10;
    //g *= ratio;
    //b *= ratio;
    //r *= ratio;
    //g *= ratio;
    //b *= ratio;
    return strip.Color(r,g,b);
}

static void
tetris_display_update(int dt, int stateptr, int barlen)
{
    struct tetris_state *state = (struct tetris_state *)stateptr;
    char *bar = state->bar;
    //uint32_t *shadow = state->shadow;
    for (int i = 0; i < barlen; i++) {
        //printf("i:%d bar:%d shadow:%ld \n\r", i, bar[i], shadow[i]);
        //if (bar[i]) {
            strip.setPixelColor(i, index2color(bar[i]));
        //} else {
            //strip.setPixelColor(i, index2color(1));
            //strip.setPixelColor(i, shadow[i]);
            //shadow[i] = fade(shadow[i], 0.99);
        //}
    }
    strip.show();
    schedule_insert(tetris_display_update, dt, stateptr, barlen, millis()+dt);
}

static void
tetris(int init)
{
    const int8_t colorcount = 3;
    const int8_t barlen = RGBC;
    static tetris_state state;
    char *bar = state.bar;
    uint32_t *shadow = state.shadow;
    if (init) {
        memset(bar, 0, sizeof(char[RGBC]));
        memset(shadow, 0, sizeof(uint32_t[RGBC]));
        schedule_insert(tetris_tick, (int)&state, 200, -1, millis());
        schedule_insert(tetris_display_update, 20, (int)&state, barlen, millis());
        digitalWrite(ctrls[0][LED], HIGH);
        digitalWrite(ctrls[4][LED], HIGH);
        schedule_insert(blink, ctrls[1][LED], HIGH, 100, millis());
        schedule_insert(blink, ctrls[5][LED], HIGH, 100, millis());
    }
    int btn1[2];
    btn1[0] = debounce(btn_state[0], 20, 0) && btn_state[0];
    btn1[1] = debounce(btn_state[4], 20, 2) && btn_state[4];
    int btn2[2];
    btn2[0] = debounce(btn_state[1], 20, 1) && btn_state[1];
    btn2[1] = debounce(btn_state[5], 20, 3) && btn_state[5];

    for (int player = 0; player < 2; player++)
    {
        int8_t pinx = -1;
        for (int i = player*15+15-1; i >= player*15; i--) {
            if(bar[i]) {
                pinx = i;
                break;
            }
        }
        int stable = (pinx == player*15 || bar[pinx-1]);
        if (!stable) {
            if (pinx >= player*15) {
                if (btn1[player]) {
                    bar[pinx] %= colorcount;
                    bar[pinx]++;
                }
                if (btn2[player]) {
                    int8_t freex = -1;
                    for (int i = pinx-1; i >= player*15; i--) {
                        if (bar[i]) break;
                        freex = i;
                    }
                    if (freex >= player*15) {
                        bar[freex] = bar[pinx];
                        bar[pinx] = 0;
                    }
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
            reaction();
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
