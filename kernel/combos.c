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


//
// TODO needs to be in per-machine file, but in the same page as combos.c and switches.c (?)
//

const combo_step_t cstp_left_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, 0 }
	}
};

const combo_step_t cstp_left_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,    TIME_3S },
		{ SW_MIDDLE_ROLLOVER,  TIME_3S },
		{ SW_RIGHT_ROLLOVER,   TIME_3S }
	}
};


const combo_step_t cstp_right_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, 0 }
	}
};

const combo_step_t cstp_right_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,   TIME_3S },
		{ SW_MIDDLE_ROLLOVER, TIME_3S },
		{ SW_RIGHT_ROLLOVER,  TIME_3S }
	}
};

const combo_step_t cstp_wildcard_5sec = {
	.flags = CSTP_WILDCARD,
	.switches = 0,
	.time_allowed = TIME_5S
};

extern void callset_lr_rl_combo_shot(void);
const combo_def_t lr_rl_combo = {
	.fn = callset_lr_rl_combo_shot,
	.name = "LR RL",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

extern void callset_rl_lr_combo_shot(void);
const combo_def_t rl_lr_combo = {
	.fn = callset_rl_lr_combo_shot,
	.name = "RL LR",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

const combo_def_t rl_rl_combo = {
	.fn = null_function,
	.name = "RL RL",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

const combo_def_t lr_lr_combo = {
	.fn = null_function,
	.name = "LR LR",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

const combo_def_t ll_rr_combo = {
	.fn = null_function,
	.name = "LL RR",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_entry,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_entry
	}
};

const combo_def_t rr_ll_combo = {
	.fn = null_function,
	.name = "RR LL",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_entry,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_entry
	}
};

#define COMBO_COUNT 6

#define LR_RL_COMBO_ID 0
#define RL_LR_COMBO_ID 1
#define RL_RL_COMBO_ID 2
#define LR_LR_COMBO_ID 3
#define LL_RR_COMBO_ID 4
#define RR_LL_COMBO_ID 5

combo_def_t *machine_combos[COMBO_COUNT] = {
	&lr_rl_combo,
	&rl_lr_combo,
	&rl_rl_combo,
	&lr_lr_combo,
	&ll_rr_combo,
	&rr_ll_combo
};

U8 machine_combos_count;
U8 current_step_markers[COMBO_COUNT]; // 1-based index
U16 step_time_list[COMBO_COUNT]; // the system timer at the time the last-match step that counted
U16 step_time_allowed_list[COMBO_COUNT]; // the time that was allowed by the last-hit switch or step that counted

void combo_reset_current_step_markers(void) {
	last_matched_combo = 0;
	memset(current_step_markers, 0x00, sizeof(current_step_markers));
	memset(step_time_list, 0x00, sizeof(step_time_list));
	memset(step_time_allowed_list, 0x00, sizeof(step_time_allowed_list));
}

CALLSET_ENTRY(machine_combos, init, start_ball) {
	machine_combos_count = COMBO_COUNT;
	combo_reset_current_step_markers();
}

//
// TODO End of per machine file
//


extern void dump_switch_details(U8 sw);

// TODO implement flags
#define CF_NEVER               0
#define CF_ALWAYS              (1 << 0)
#define CF_SINGLE_BALL_ONLY    (1 << 1)
#define CF_MULTI_BALL_ONLY     (1 << 3)


/*
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


			callset_pointer_invoke(combo->fn);

 */

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
#endif

U8 *current_step_marker_ptr;
U16 *step_time_ptr;
U16 *step_time_allowed_ptr;
const combo_def_t *last_matched_combo;

/**
 * requires the following globals to be initialised correctly first
 *
 * sw_last_scheduled
 * sw_last_scheduled_time
 * step_time_ptr;
 * step_time_allowed_ptr;
 */
const combo_switch_t *combo_match_switch_to_steps(const combo_step_t *combo_step) {
	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		task_runs_long(); // XXX ?
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("checking switch: %d, time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);

		if (sw_last_scheduled == combo_switch->switch_id) {
			if (combo_switch->time_allowed == 0) {
				//dbprintf("no time specified on switch\n");
				if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
					dbprintf("time matched (A: %ld < %ld)\n", sw_last_scheduled_time, *step_time_ptr + *step_time_allowed_ptr);
					return combo_switch;
				}
			} else {
				if (sw_last_scheduled_time < *step_time_ptr + combo_switch->time_allowed) {
					dbprintf("time matched (B: %ld < %ld)\n", sw_last_scheduled_time, *step_time_ptr + combo_switch->time_allowed);
					return combo_switch;
				}
			}
		}
	}
	return 0;
}


void combo_process_switch_for_combo(const U8 combo_id, const combo_def_t *combo) {

	U8 retry = FALSE;
	bool advance;
	U16 time_allowed = 0;
	const combo_switch_t *matched_switch = 0;

#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("processing combo: %d\n", combo_id);
	dump_switch_details(sw_last_scheduled);
	dbprintf("combo: %s\n", combo->name);
#endif



	current_step_marker_ptr = &current_step_markers[combo_id];
	step_time_ptr = &step_time_list[combo_id];
	step_time_allowed_ptr = &step_time_allowed_list[combo_id];
#ifdef CONFIG_UNITTEST
	if (combo_id == UNITTEST_COMBO_ID) {
		current_step_marker_ptr = &unittest_current_step_marker;
		step_time_ptr = &unittest_step_time;
		step_time_allowed_ptr = &unittest_step_time_allowed;
	}
#endif

#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("current step (before): %d\n", *current_step_marker_ptr);
	dbprintf("step time (before): %ld\n", *step_time_ptr);
	dbprintf("step time allowed (before): %ld\n", *step_time_allowed_ptr);
#endif

	do {
#ifdef CONFIG_DEBUG_COMBOS
		if (retry) {
			dbprintf("current step (retry): %d\n", *current_step_marker_ptr);
			dbprintf("step time (retry): %ld\n", *step_time_ptr);
			dbprintf("step time allowed (retry): %ld\n", *step_time_allowed_ptr);
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
			//dbprintf("current step\n");
			dump_combo_step(current_step);
		} else {
			//dbprintf("at first step\n");
		}
		//dbprintf("next step\n");
		dump_combo_step(next_step);
#endif
		//
		// attempt to match switch against next step's switches
		//

		retry = FALSE;
		advance = FALSE;

		if (next_step->switches) {
			matched_switch = combo_match_switch_to_steps(next_step);
			if (matched_switch) {
				if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
					advance = TRUE;
					time_allowed = matched_switch->time_allowed;
				} else {
					dbprintf("... but not hit in time (A)\n");
				}
			}
		} else {
			//dbprintf("switch matches wildcard step\n");
			//if (sw_last_scheduled_time < *step_time_ptr + next_step->time_allowed) {
			//dbprintf("timer must be < %ld\n", *step_time_ptr + *step_time_allowed_ptr);
			if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < (*step_time_ptr) + (*step_time_allowed_ptr)) {
				advance = TRUE;


				if ((*current_step_marker_ptr) + 1 < combo->steps) {
					// who defines a combo that ends with a wildcard step anyway? ...
					//dbprintf("checking next step too\n");
					retry = TRUE; // see if the switch matches the next step too
				} else {
					//dbprintf("no more steps, not retrying\n");
				}
				// TODO don't retry if the next step is the last one
				time_allowed = next_step->time_allowed;
			} else {
				//dbprintf("... but not hit in time (B)\n");
				*current_step_marker_ptr = 0; // start again at the first step.
				*step_time_ptr = 0;
				*step_time_allowed_ptr = 0;
			}
		}

		if (advance) {
			*current_step_marker_ptr = (*current_step_marker_ptr) + 1;
			//dbprintf(">>> TA: %ld\n", time_allowed);
			*step_time_ptr = sw_last_scheduled_time;
			if (*current_step_marker_ptr < combo->steps && combo->step_list[*current_step_marker_ptr]->switches == 0) {
				//dbprintf("next step is wildcard\n");
				*step_time_allowed_ptr = combo->step_list[*current_step_marker_ptr]->time_allowed;
			} else {
				*step_time_allowed_ptr = time_allowed;
			}
		} else {
			//dbprintf("unmatched\n");
			if (current_step) {
				if (current_step->switches == 0) {
					//dbprintf("...but current switch is wildcard\n");
					if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
						//dbprintf("... and allowed time is not elapsed\n");
					} else {
						//dbprintf("... however allowed time has elapsed\n");
						*current_step_marker_ptr = 0; // start again at the first step.
						*step_time_ptr = 0;
						*step_time_allowed_ptr = 0;
					}
				} else {
					if (*current_step_marker_ptr > 1 && combo->step_list[(*current_step_marker_ptr) - 2]->switches == 0) {
						//dbprintf("preceding switch IS wildcard\n");
						//dbprintf("resetting marker to preceding\n");
						*current_step_marker_ptr = (*current_step_marker_ptr) - 1; // start again at the previous step.
					} else {
						//dbprintf("preceding switch is NOT wildcard\n");
						//dbprintf("resetting marker to start\n");
						*current_step_marker_ptr = 0; // start again at the first step.
						*step_time_ptr = 0;
						*step_time_allowed_ptr = 0;
					}
					retry = TRUE;
					//dbprintf("retrying\n");
				}

			}
		}
	} while (retry);


	if (*current_step_marker_ptr == combo->steps) {
		*current_step_marker_ptr = 0;
		*step_time_ptr = 0;
		*step_time_allowed_ptr = 0;
		//dbprintf("### combo matched! ###\n");

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


#endif
