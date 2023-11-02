
#ifndef INC_SCREENS_H_
#define INC_SCREENS_H_

// Comment Out if in actual dash
#define USE_SIMULATOR

#include <stdint.h>

enum SCREENS {
	DRIVER_SCREEN,
	DEBUG_SCREEN,
    ENERGY_SCREEN,
    LAP_SCREEN
};

// CALL THIS FIRST
void init_displays();

// CALL THIS SECOND
void init_screens();

void try_update_screen();

void update_screen();

void try_cycle_screens();

void cycle_screens();

// Helpers
void ms_to_minutes_seconds(uint32_t ms, uint32_t * minutes, uint32_t * seconds, uint32_t * milliseconds);

#endif /* INC_SCREENS_H_ */
