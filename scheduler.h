#ifndef __SCHEDULE_H
#define __SCHEDULE_H

void schedule_init();
void schedule_run();
int schedule_insert(void(*func)(int, int, int), int arg1, int arg2, int arg3, long time);
void schedule_wipe();

#endif /*__SCHEDULE_H*/
