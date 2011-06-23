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

extern combo_definition_t *last_matched_combo;

void combo_name_deff (void)
{
	sprintf("%s", last_matched_combo->name);
	flash_and_exit_deff (20, TIME_100MS);
}

//extern U8 sw_last_scheduled;

/*
//CALLSET__ENTRY (combo_handler, any_pf_switch)
{
	U16 now = get_sys_time();
#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("now: %ld, sw: %d\n", now, sw_last_scheduled);
#endif
	recent_switch_t *switch_hit = &recent_switches[next_recent_switch];
	switch_hit->hit_time = now;
	switch_hit->switch_id = sw_last_scheduled;

	next_recent_switch++;
	if (unlikely( next_recent_switch >= MAX_RECENT_SWITCHES)) {
		next_recent_switch = 0;
	}

#ifdef CONFIG_DEBUG_COMBOS
	dump_recent_switches();
#endif
	process_combos();
}
*/



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
