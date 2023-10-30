
#ifndef INC_SCREENS_H_
#define INC_SCREENS_H_

// Comment Out if in actual dash
#define USE_SIMULATOR

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

#endif /* INC_SCREENS_H_ */
