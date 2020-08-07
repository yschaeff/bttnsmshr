
#include <Arduino.h>

int debounce(int value, int cooldown, int slot)
{
    static int *state = NULL;
    static unsigned long *last_change = NULL;
    static int DEBOUNCE_SLOTS = 0;
    unsigned long now = millis();

    /* Dynamic allocation of slots */
    if (slot >= DEBOUNCE_SLOTS) {
        int old_slots = DEBOUNCE_SLOTS;
        DEBOUNCE_SLOTS = slot+1;
        state = (int*)realloc(state, DEBOUNCE_SLOTS * sizeof(int));
        last_change = (unsigned long*)realloc(last_change, DEBOUNCE_SLOTS * sizeof(unsigned long));
        /* New slots should be cleared */
        memset(last_change+old_slots, 0, (DEBOUNCE_SLOTS-old_slots) * sizeof(unsigned long));
    }

    /*
     * If this is the first even for the slot, allow the event.
     * otherwise only when sufficient time has elapsed
     */
    if (!last_change[slot] || (state[slot] != value && now >= last_change[slot] + cooldown)) {
        last_change[slot] = now;
        state[slot] = value;
        return 1;
    }
    return 0;
}

