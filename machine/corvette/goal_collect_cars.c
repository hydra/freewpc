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

// TODO remove excess debug logging when things have settled down

#include <freewpc.h>

__local__ U8 cars_collected;

enum car_list {
	CARS_MIN = 0,
	CAR_1953_BLUEFLAME = CARS_MIN,
	CAR_1963_GRAND_SPORT,
	CAR_1963_FUELIE,
	CAR_1967_STINGRAY_L88,
	CAR_1971_STINGRAY_LT1,
	CAR_1978_STINGRAY_L82,
	CAR_1982_L83,
	CAR_1989_L98,
	CAR_1993_ZR1,
	CARS_MAX = CAR_1993_ZR1,
};

#define CAR_COUNT (CARS_MAX + 1) // 9 pf lights, 9 cars to collect

U8 car_lamp_map[CAR_COUNT] = {
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

sound_code_t car_award_sound_map[CAR_COUNT] = {
	SPCH_BEST_TAKE_CARE_OF_BLUE_FLAME,
	SPCH_BEST_TAKE_CARE_OF_GRAN_SPORT,
	SPCH_BEST_TAKE_CARE_OF_FUELIE,
	SPCH_BEST_TAKE_CARE_OF_67_STINGRAY,
	SPCH_BEST_TAKE_CARE_OF_LT1,
	SPCH_BEST_TAKE_CARE_OF_PACE_CAR,
	SPCH_BEST_TAKE_CARE_OF_L83,
	SPCH_BEST_TAKE_CARE_OF_CHALLENGE_CAR,
	SPCH_BEST_TAKE_CARE_OF_ZR1
};

void goal_car_awarded_deff(void) {
	// TODO sounds
	// TODO flashers / lamp effects
	dmd_alloc_low_clean ();
	dmd_draw_border (dmd_low_buffer);
	font_render_string_center (&font_supercar9, 64, 9, "CAR AWARDED");
	dmd_show_low();
	task_sleep_sec(2);
	deff_exit ();
}

bool have_collected_all_cars(void) {
	return (cars_collected >= CAR_COUNT);
}

// TODO rename to award_next_car() ?
void award_car(void) {
	if (have_collected_all_cars()) {
		return;
	}
	speech_start(car_award_sound_map[cars_collected], SL_4S);
	cars_collected++;
	callset_invoke(lamp_update);
	deff_start(DEFF_GOAL_CAR_AWARDED);
}

CALLSET_ENTRY (goal_collect_cars, start_player) {
	cars_collected = 0;
}

CALLSET_ENTRY (goal_collect_cars, lamp_update) {



	U8 car_lamp_index;
	for (car_lamp_index = CARS_MIN; car_lamp_index <= CARS_MAX; car_lamp_index++) {
		if (cars_collected >= car_lamp_index + 1) {
			lamp_on(car_lamp_map[car_lamp_index]);
		} else {
			lamp_off(car_lamp_map[car_lamp_index]);
		}

	}

}

