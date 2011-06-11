/*
 * Copyright 2010 by Dominic Clifton <me@dominicclifton.name>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <freewpc.h>

__local__ U16 collected_cars;

enum car_list {
	FIRST_PLAYFIELD_LAMP = 0,
	CAR_1953_BLUEFLAME = FIRST_PLAYFIELD_LAMP,
	CAR_1963_GRAND_SPORT,
	CAR_1963_FUELIE,
	CAR_1967_STINGRAY_L88,
	CAR_1971_STINGRAY_LT1,
	CAR_1978_STINGRAY_L82,
	CAR_1982_L83,
	CAR_1989_L98,
	CAR_1993_ZR1,
	LAST_PLAYFIELD_LAMP = CAR_1993_ZR1,
	// The 1997_C5 does not have a playfield lamp, instead it has a back-box flasher.
	CAR_1997_C5,
	CARS_MAX = CAR_1997_C5
};

#define CAR_BIT(x) (1 << (x + 1))

#define CAR_BIT_NONE (0x00)
#define CAR_BIT_ALL (0x1FF)
#define is_car_collected(car_bit) (collected_cars & car_bit)

U8 car_lamp_map[LAST_PLAYFIELD_LAMP + 1] = {
	LM_CORVETTE_1,
	LM_CORVETTE_2,
	LM_CORVETTE_3,
	LM_CORVETTE_4,
	LM_CORVETTE_5,
	LM_CORVETTE_6,
	LM_CORVETTE_7,
	LM_CORVETTE_8,
	LM_CORVETTE_9
};

CALLSET_ENTRY (goal_collect_cars, start_player) {
	dbprintf("goal_collect_cars/start_player\n");
	dbprintf("collected cars: %ld\n", collected_cars);
	collected_cars = CAR_BIT_NONE;
}

CALLSET_ENTRY (goal_collect_cars, lamp_update) {
	dbprintf("goal_collect_cars/lamp_update - start\n");
	dbprintf("collected cars: %ld\n", collected_cars);
	U8 index;
	for (index = 0; index <= LAST_PLAYFIELD_LAMP; index++) {
		if (is_car_collected(CAR_BIT(index))) {
			dbprintf("car, index: %d, IS_COLLECTED\n", index);
			lamp_on(car_lamp_map[index]);
		} else {
			dbprintf("car, index: %d, NOT_COLLECTED\n", index);
			lamp_off(car_lamp_map[index]);
		}
	}
	dbprintf("goal_collect_cars/lamp_update - exit\n");
}

// XXX debugging
CALLSET_ENTRY (goal_collect_cars, sw_skid_pad_standup, sw_million_standup) {
	dbprintf("collected cars (before): %ld\n", collected_cars);
	collected_cars = ((collected_cars << 1) | 1) & CAR_BIT_ALL;
	dbprintf("collected cars (after): %ld\n", collected_cars);
	callset_invoke(lamp_update);
}

