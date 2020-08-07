/**
 * Multislot debouncer
 *
 * This debouncer does not require any initialization. Additional debounce slots
 * are allocated on the go (but never free'd). Each perhipal SHOULD get assigned
 * its own slot by the caller.
 *
 * \param value: current value of perhiperal
 * \param cooldown: time in ms since previous event that should minimally have
 *      elapsed for this event to be considered valid.
 * \param slot: Which slot to store the state in
 * \Return: 1 if event is considered valid.
 **/
int debounce(int value, int cooldown, int slot);
