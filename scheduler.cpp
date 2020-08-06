#include <Arduino.h>
#include "scheduler.h"

#define NTASKS 20
struct task {
    long time;
    void (*func)(int, int, int);
    int arg1;
    int arg2;
    int arg3;
};

static struct task schedule[NTASKS];

void schedule_init()
{
    (void) memset(schedule, 0, sizeof schedule);
}

void schedule_run()
{
    long now = millis();
    for (int i = 0; i<NTASKS; i++) {
        if (!schedule[i].time) continue;
        if (schedule[i].time > now) continue;
        schedule[i].time = 0;
        schedule[i].func(schedule[i].arg1, schedule[i].arg2, schedule[i].arg3);
    }
}

int schedule_insert(void(*func)(int, int, int), int arg1, int arg2, int arg3, long time)
{
    for (int i = 0; i<NTASKS; i++) {
        if (schedule[i].time) continue;
        schedule[i].time = time;
        schedule[i].func = func;
        schedule[i].arg1 = arg1;
        schedule[i].arg2 = arg2;
        schedule[i].arg3 = arg3;
        return 0;
    }
    Serial.println("SCHEDULER CONGESTION!");
    return 1;
}

void schedule_remove(void(*func)(int, int, int))
{
    for (int i = 0; i<NTASKS; i++) {
        if (schedule[i].func != func) continue;
        schedule[i].time = 0;
    }
}

void schedule_wipe()
{
    for (int i = 0; i<NTASKS; i++) {
        schedule[i].time = 0;
    }
}
