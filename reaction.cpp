#include <Arduino.h>
#include "tasks.h"
#include "scheduler.h"
#include "io.h"
#include "reaction.h"

void reaction()
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

