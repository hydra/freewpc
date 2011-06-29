/*
 * Copyright 2011 by Dominic Clifton <me@dominicclifton.name>
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

#ifdef CONFIG_COMBOS

extern combo_definition_t *last_matched_combo;

void combo_name_deff (void)
{
	sprintf("%s", last_matched_combo->name);
	flash_and_exit_deff (20, TIME_100MS);
}

CALLSET_ENTRY (combo, left_orbit_combo_shot)
{
	dbprintf("single_left_orbit_shot handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_1M); // XXX
}

CALLSET_ENTRY (combo, right_orbit_combo_shot)
{
	dbprintf("single_right_orbit_shot handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_1M); // XXX
}

CALLSET_ENTRY (combo, rl_orbit_rl_ramp_combo_shot)
{
	dbprintf("single_right_orbit_shot handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_10M); // XXX
}

CALLSET_ENTRY (combo, ordered_bank_1_combo_shot)
{
	dbprintf("single_right_orbit_shot handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_5M); // XXX
}
#else
__far__(C_STRING(MACHINE_PAGE)) void combo_ordered_bank_1_combo_shot (void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_left_orbit_combo_shot (void) {};
__far__(C_STRING(MACHINE_PAGE)) void combo_right_orbit_combo_shot (void) {};
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_orbit_rl_ramp_combo_shot (void) {};
#endif
