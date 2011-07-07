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

/*
 * IMPORTANT
 *
 * There MUST be a call to combo_reset_current_step_markers() after machine_combos_count has been set via an 'init' callset handler.
 * See 'machine/corvette/combo_definitions.c'
 *
 */

#ifdef CONFIG_COMBOS

extern void dump_switch_details(U8 sw);

// TODO implement flags
#define CF_NEVER               0
#define CF_ALWAYS              (1 << 0)
#define CF_SINGLE_BALL_ONLY    (1 << 1)
#define CF_MULTI_BALL_ONLY     (1 << 3)


/*
 * TODO use this bit of old code
 *
	if (!(
			(combo->flags & CF_ALWAYS) ||
			((combo->flags & CF_SINGLE_BALL_ONLY) && single_ball_play()) ||
			((combo->flags & CF_MULTI_BALL_ONLY) && !single_ball_play())
	)) {
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("skipping, flags: %d, sbp: %d\n", combo->flags, single_ball_play());
#endif
		return;
	}


 */

/*
 * To avoid filling the ROM with loads of strings and for more readable code, we use combodbprintf instead of dbprintf and a #ifdef/#endif pair
 * The debug messages are only compiled in when in unit-test mode.
 */
#ifdef CONFIG_UNITTEST
#define combodbprintf(format, ...) dbprintf(format, ##__VA_ARGS__)
#else
#define combodbprintf(format, ...)
#endif

#if defined(CONFIG_DEBUG_COMBOS) || defined(CONFIG_UNITTEST)
void dump_combo_step(const combo_step_t *combo_step) {
	dbprintf("switches: %d\n", combo_step->switches);

	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("switch: %d, Time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);
	}
}

void dump_combo(const combo_def_t *combo) {
	dbprintf("combo: %s\n", combo->name);
	dbprintf("steps: %d\n", combo->steps);


	U8 step;
	for (step = 0; step < combo->steps; step ++) {
		dbprintf("step %d>\n", step);
		dbprintf("time_allowed: %ld\n", combo->step_list[step]->time_allowed);
		dump_combo_step(combo->step_list[step]);
	}
}
#endif

#ifdef CONFIG_UNITTEST
U8 combo_matches; // a counter so combo matches can be tested

U8 unittest_current_step_marker;
U16 unittest_step_time;
U16 unittest_step_time_allowed;
U16 unittest_wildcard_time;
#endif

U8 *current_step_marker_ptr;
U16 *step_time_ptr;
U16 *step_time_allowed_ptr;
U16 *wildcard_time_ptr;
const combo_def_t *last_matched_combo;
U8 machine_combos_count;


/**
 * requires the following globals to be initialised correctly first
 *
 * sw_last_scheduled
 * sw_last_scheduled_time
 * step_time_ptr;
 * step_time_allowed_ptr;
 */
const combo_switch_t *combo_match_switch_to_step(const combo_step_t *combo_step) {
	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		combodbprintf("checking switch: %d, time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);

		if (sw_last_scheduled == combo_switch->switch_id) {
			return combo_switch;
		}
	}
	return 0;
}

void reset_markers_for_current_combo(void) {
	*current_step_marker_ptr = 0; // start again at the first step.
	*step_time_ptr = 0;
	*step_time_allowed_ptr = 0;
}

void combo_process_switch_for_combo(const U8 combo_id, const combo_def_t *combo) {

	U8 retry = FALSE;
	bool advance;
	U16 time_allowed = 0;
	const combo_switch_t *matched_switch = 0;
	U8 wildcard_marker;

#ifdef CONFIG_DEBUG_COMBOS
	combodbprintf("processing combo: %s (%d)\n", combo->name, combo_id);
#ifdef CONFIG_UNITTEST
	// when in unit test mode the switch strings for ID's used by unit-test combo are not valid.
	combodbprintf("switch hit: %d\n", sw_last_scheduled);
#else
	dump_switch_details(sw_last_scheduled);
#endif
#endif



	current_step_marker_ptr = &current_step_markers[combo_id];
	step_time_ptr = &step_time_list[combo_id];
	step_time_allowed_ptr = &step_time_allowed_list[combo_id];
	wildcard_time_ptr = &wildcard_time_list[combo_id];
#ifdef CONFIG_UNITTEST
	if (combo_id == UNITTEST_COMBO_ID) {
		current_step_marker_ptr = &unittest_current_step_marker;
		step_time_ptr = &unittest_step_time;
		step_time_allowed_ptr = &unittest_step_time_allowed;
		wildcard_time_ptr = &unittest_wildcard_time;
	}
#endif

#ifdef CONFIG_DEBUG_COMBOS
	combodbprintf("current step (before): %d\n", *current_step_marker_ptr);
	combodbprintf("step time (before): %ld\n", *step_time_ptr);
	combodbprintf("step time allowed (before): %ld\n", *step_time_allowed_ptr);
#endif

	do {
		task_runs_long(); // XXX ?
#ifdef CONFIG_DEBUG_COMBOS
		if (retry) {
			combodbprintf("current step (retry): %d\n", *current_step_marker_ptr);
			combodbprintf("step time (retry): %ld\n", *step_time_ptr);
			combodbprintf("step time allowed (retry): %ld\n", *step_time_allowed_ptr);
		}
#endif
		//
		// determine current and next step
		//

		const combo_step_t *current_step = 0;

		if (*current_step_marker_ptr > 0) {
			current_step = combo->step_list[(*current_step_marker_ptr) - 1];
		}
		const combo_step_t *next_step = combo->step_list[(*current_step_marker_ptr)];

#ifdef CONFIG_DEBUG_COMBOS
		if (current_step) {
			combodbprintf("current step\n");
			dump_combo_step(current_step);
		} else {
			combodbprintf("at first step\n");
		}
		combodbprintf("next step\n");
		dump_combo_step(next_step);
#endif
		//
		// attempt to match switch against next step's switches
		//

		retry = FALSE;
		advance = FALSE;
		time_allowed = 0;

		if (next_step->switches) {
			matched_switch = combo_match_switch_to_step(next_step);
			if (matched_switch) {
				combodbprintf("switch matched\n");
				advance = TRUE;
				if (*current_step_marker_ptr == 0) {
					combodbprintf("at first step, ignoring time\n");
				} else if (*step_time_allowed_ptr == 0 && matched_switch->time_allowed == 0) {
					combodbprintf("step time and switch time are both 0\n");
				} else if (*step_time_allowed_ptr > 0 && sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
					combodbprintf("%ld < %ld (%ld + %ld) (S)\n", sw_last_scheduled_time, *step_time_ptr + *step_time_allowed_ptr, *step_time_ptr, *step_time_allowed_ptr);
				} else if (matched_switch->time_allowed > 0 && sw_last_scheduled_time < *step_time_ptr + matched_switch->time_allowed) {
					combodbprintf("%ld < %ld (%ld + %ld) (M)\n", sw_last_scheduled_time, *step_time_ptr + matched_switch->time_allowed, *step_time_ptr, matched_switch->time_allowed);
				} else {
					combodbprintf("... but not hit in time (A)\n");
					reset_markers_for_current_combo();
					break; // bail!
				}
			}
		} else {
			combodbprintf("switch matches wildcard step\n");
			*wildcard_time_ptr = sw_last_scheduled_time;
			//if (sw_last_scheduled_time < *step_time_ptr + next_step->time_allowed) {
			combodbprintf("timer must be < %ld\n", *step_time_ptr + *step_time_allowed_ptr);
			if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
				advance = TRUE;


				if ((*current_step_marker_ptr) + 1 < combo->steps) { // who defines a combo that ends with a wildcard step anyway? ...
					combodbprintf("checking next step too\n");
					retry = TRUE; // see if the switch matches the next step too
				} else {
					combodbprintf("no more steps, not retrying\n");
				}
				time_allowed = next_step->time_allowed;

				// TODO based on combo step flags, call the combo's callset - the handler can then check the marker and react accordingly (for sounds, lights, etc);

			} else {
				combodbprintf("... but not hit in time (B)\n");
				reset_markers_for_current_combo();
			}
		}

		if (advance) {
			*current_step_marker_ptr = (*current_step_marker_ptr) + 1;
			combodbprintf("advancing to step: %d\n", *current_step_marker_ptr);
			combodbprintf("time_allowed: %ld\n", time_allowed);
			*step_time_ptr = sw_last_scheduled_time;
			if (*current_step_marker_ptr < combo->steps && combo->step_list[*current_step_marker_ptr]->switches == 0) {
				combodbprintf("next step is wildcard\n");
				*step_time_allowed_ptr = combo->step_list[*current_step_marker_ptr]->time_allowed;
			} else {
				*step_time_allowed_ptr = time_allowed;
			}
		} else {
			combodbprintf("step not unmatched\n");
			if (current_step) {
				if (current_step->switches == 0) {
					combodbprintf("...but current switch is wildcard\n");
				} else {

					// was there a wildcard in the combo before this switch? look for the previous wildcard

					wildcard_marker = combo->steps;
					do {
						if (combo->step_list[wildcard_marker - 1]->switches == 0) {
							break;
						}
						wildcard_marker--;
					} while (wildcard_marker != 0);

					if (*current_step_marker_ptr > 1 && wildcard_marker > 1) {
						combodbprintf("there was a preceding wildcard\n");
						combodbprintf("resetting marker to wildcard\n");
						*current_step_marker_ptr = wildcard_marker;
						*step_time_allowed_ptr = combo->step_list[wildcard_marker - 1]->time_allowed;
						*step_time_ptr = *wildcard_time_ptr;
					} else {
						combodbprintf("preceding switch is NOT wildcard\n");
						combodbprintf("resetting marker to start\n");
						reset_markers_for_current_combo();
					}
					retry = TRUE;
					combodbprintf("retrying\n");
				}

			}
		}
	} while (retry);


	if (*current_step_marker_ptr == combo->steps) {
		reset_markers_for_current_combo();
		combodbprintf("### combo matched! ###\n");

		last_matched_combo = combo;
#ifdef CONFIG_UNITTEST
		combo_matches++;
#else
		if (combo->fn) {
			callset_pointer_invoke(combo->fn);
		}
#endif
	}

#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("current step (after): %d\n", *current_step_marker_ptr);
	dbprintf("step time (after): %ld\n", *step_time_ptr);
	dbprintf("step time allowed (after): %ld\n", *step_time_allowed_ptr);
#endif

}

void combo_process_switch(void) {

	// iterate over each combo and update the last-hit markers

	U8 combo_index = 0;

	for (combo_index = 0; combo_index < machine_combos_count; combo_index++) {
		combo_process_switch_for_combo(combo_index, machine_combos[combo_index]);
	}
}

void combo_reset_current_step_markers(void) {
	last_matched_combo = 0;
	memset(current_step_markers, 0x00, sizeof(U8) * machine_combos_count);
	memset(step_time_list, 0x00,sizeof(U16) * machine_combos_count);
	memset(step_time_allowed_list, 0x00, sizeof(U16) * machine_combos_count);
	memset(wildcard_time_list, 0x00, sizeof(U16) * machine_combos_count);
#ifdef CONFIG_UNITTEST
	unittest_current_step_marker = 0;
#endif
}


#endif
