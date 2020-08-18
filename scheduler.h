#ifndef SCHEDULE_H
#define SCHEDULE_H

void schedule_init();
void schedule_run();
int schedule_insert(void(*func)(int, int, int), int arg1, int arg2, int arg3, long time);
void schedule_remove(void(*func)(int, int, int));
void schedule_wipe();

#endif
