#include <freewpc.h>
#include "seatest.h"

/*
 * switches_tests.c
 *
 *  Created on: 25 Jun 2011
 *      Author: Hydra
 */

#define CSTP_NO_FLAGS 0
#define CSTP_WILDCARD (1<<0)

typedef struct combo_switch_s {
	U8 switch_id;
	U16 time_allowed; // takes precedence over the value in combo_step_t that uses this combo_switch_t
} combo_switch_t;

typedef struct combo_step_s {
	U8 flags;
	U8 switches;
	U16 time_allowed;
	combo_switch_t switch_list[];
} combo_step_t;

typedef struct combo_def_s {
	char *name;
	U8 steps;
	combo_step_t *step_list[];
} combo_def_t;


static combo_step_t cstp_any_coin = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_COIN, 0},
		{ SW_CENTER_COIN, 0},
		{ SW_RIGHT_COIN, 0},
		{ SW_FOURTH_COIN, 0}
	}
};

static combo_step_t cstp_left_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, 0 }
	}
};

static combo_step_t cstp_left_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,    TIME_3S },
		{ SW_MIDDLE_ROLLOVER,  TIME_3S },
		{ SW_RIGHT_ROLLOVER,   TIME_3S }
	}
};


static combo_step_t cstp_right_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, 0 }
	}
};

static combo_step_t cstp_right_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,   TIME_3S },
		{ SW_MIDDLE_ROLLOVER, TIME_3S },
		{ SW_RIGHT_ROLLOVER,  TIME_3S }
	}
};

static combo_step_t cstp_wildcard_5sec = {
	.flags = CSTP_WILDCARD,
	.switches = 0,
	.time_allowed = TIME_5S
};


static combo_def_t lr_rl_combo = {
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

static combo_def_t rl_lr_combo = {
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

static combo_def_t rl_rl_combo = {
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

static combo_def_t lr_lr_combo = {
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

static combo_def_t ll_rr_combo = {
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

static combo_def_t rr_ll_combo = {
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

static void dump_combo_step(const combo_step_t *combo_step) {
	dbprintf("switches: %d\n", combo_step->switches);

	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("switch: %d, Time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);
	}
}

static void dump_combo(const combo_def_t *combo) {
	dbprintf("combo: %s\n", combo->name);
	dbprintf("steps: %d\n", combo->steps);


	U8 step;
	for (step = 0; step < combo->steps; step ++) {
		dbprintf("step %d>\n", step);
		dbprintf("time_allowed: %ld\n", combo->step_list[step]->time_allowed);
		dump_combo_step(combo->step_list[step]);
	}
}

//
// from switches.c
//
U16 sw_last_scheduled_time; // TODO
extern void dump_switch_details(U8 sw);
extern U8 sw_last_scheduled;

//
// from mach-combos.c
//
#define COMBO_COUNT 6

U8 current_step_markers[COMBO_COUNT]; // 1-based index
U16 step_time_list[COMBO_COUNT]; // the system timer at the time the last-match step that counted
U16 step_time_allowed_list[COMBO_COUNT]; // the time that was allowed by the last-hit switch or step that counted

static combo_def_t *machine_combos[COMBO_COUNT] = {
	&lr_rl_combo,
	&rl_lr_combo,
	&rl_rl_combo,
	&lr_lr_combo,
	&ll_rr_combo,
	&rr_ll_combo
};

#define LR_RL_COMBO_ID 0
#define RL_LR_COMBO_ID 1
#define RL_RL_COMBO_ID 2
#define LR_LR_COMBO_ID 3
#define LL_RR_COMBO_ID 4
#define RR_LL_COMBO_ID 5

//
// from combos.h
//

#ifdef CONFIG_UNITTEST
#define UNITTEST_COMBO_ID 0xFF
#endif
//
// from combos.c
//

#ifdef CONFIG_UNITTEST
U8 combo_matches; // a counter so combo matches can be tested
#endif

U8 *current_step_marker_ptr;
U16 *step_time_ptr;
U16 *step_time_allowed_ptr;

void combo_reset_current_step_markers(void) {
	memset(current_step_markers, 0x00, sizeof(current_step_markers));
	memset(step_time_list, 0x00, sizeof(step_time_list));
	memset(step_time_allowed_list, 0x00, sizeof(step_time_allowed_list));
}


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
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("checking switch: %d, time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);

		if (sw_last_scheduled == combo_switch->switch_id) {
			/*
			dbprintf("switch matches\n");
			return combo_switch;
			*/
			dbprintf("switch matches, checking time\n");
			/*
			if (sw_last_scheduled_time < *step_time_ptr + combo_switch->time_allowed) {
				dbprintf("time matched (%ld < %ld)\n", sw_last_scheduled_time, *step_time_ptr + *step_time_allowed_ptr);
				return combo_switch;
			}
			*/

			if (combo_switch->time_allowed == 0) {
				dbprintf("no time specified on switch\n");
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

#ifdef CONFIG_UNITTEST
U8 unittest_current_step_marker;
U16 unittest_step_time;
U16 unittest_step_time_allowed;
#endif

static void combo_process_switch_for_combo(const U8 combo_id, const combo_def_t *combo) {
#ifdef DEBUGGER
	dbprintf("processing combo: %d\n", combo_id);
	dump_switch_details(sw_last_scheduled);
	printf("combo: %s\n", combo->name);
#endif

	U8 retry;
	bool advance;
	U16 time_allowed = 0;
	const combo_switch_t *matched_switch = 0;
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
	dbprintf("current step (before): %d\n", *current_step_marker_ptr);
	dbprintf("step time (before): %ld\n", *step_time_ptr);
	dbprintf("step time allowed (before): %ld\n", *step_time_allowed_ptr);

	do {
		if (retry) {
			dbprintf("current step (retry): %d\n", *current_step_marker_ptr);
			dbprintf("step time (retry): %ld\n", *step_time_ptr);
			dbprintf("step time allowed (retry): %ld\n", *step_time_allowed_ptr);
		}

		//
		// determine current and next step
		//

		const combo_step_t *current_step = 0;

		if (*current_step_marker_ptr > 0) {
			current_step = combo->step_list[(*current_step_marker_ptr) - 1];
		}
		const combo_step_t *next_step = combo->step_list[(*current_step_marker_ptr)];

		if (current_step) {
			dbprintf("current step\n");
			dump_combo_step(current_step);
		} else {
			dbprintf("at first step\n");
		}
		dbprintf("next step\n");
		dump_combo_step(next_step);

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
			dbprintf("switch matches wildcard step\n");
			//if (sw_last_scheduled_time < *step_time_ptr + next_step->time_allowed) {
			dbprintf("timer must be < %ld\n", *step_time_ptr + *step_time_allowed_ptr);
			if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < (*step_time_ptr) + (*step_time_allowed_ptr)) {
				advance = TRUE;


				if ((*current_step_marker_ptr) + 1 < combo->steps) {
					// who defines a combo that ends with a wildcard step anyway? ...
					dbprintf("checking next step too\n");
					retry = TRUE; // see if the switch matches the next step too
				} else {
					dbprintf("no more steps, not retrying\n");
				}
				// TODO don't retry if the next step is the last one
				time_allowed = next_step->time_allowed;
			} else {
				dbprintf("... but not hit in time (B)\n");
				*current_step_marker_ptr = 0; // start again at the first step.
				*step_time_ptr = 0;
				*step_time_allowed_ptr = 0;
			}
		}

		if (advance) {
			*current_step_marker_ptr = (*current_step_marker_ptr) + 1;
			dbprintf(">>> TA: %ld\n", time_allowed);
			*step_time_ptr = sw_last_scheduled_time;
			if (*current_step_marker_ptr < combo->steps && combo->step_list[*current_step_marker_ptr]->switches == 0) {
				dbprintf("next step is wildcard\n");
				*step_time_allowed_ptr = combo->step_list[*current_step_marker_ptr]->time_allowed;
			} else {
				*step_time_allowed_ptr = time_allowed;
			}
		} else {
			dbprintf("unmatched\n");
			if (current_step) {
				if (current_step->switches == 0) {
					dbprintf("...but current switch is wildcard\n");
					if (*step_time_allowed_ptr == 0 || sw_last_scheduled_time < *step_time_ptr + *step_time_allowed_ptr) {
						dbprintf("... and allowed time is not elapsed\n");
					} else {
						dbprintf("... however allowed time has elapsed\n");
						*current_step_marker_ptr = 0; // start again at the first step.
						*step_time_ptr = 0;
						*step_time_allowed_ptr = 0;
					}
				} else {
					if (*current_step_marker_ptr > 1 && combo->step_list[(*current_step_marker_ptr) - 2]->switches == 0) {
						dbprintf("preceding switch IS wildcard\n");
						dbprintf("resetting marker to preceding\n");
						*current_step_marker_ptr = (*current_step_marker_ptr) - 1; // start again at the previous step.
					} else {
						dbprintf("preceding switch is NOT wildcard\n");
						dbprintf("resetting marker to start\n");
						*current_step_marker_ptr = 0; // start again at the first step.
						*step_time_ptr = 0;
						*step_time_allowed_ptr = 0;
					}
					retry = TRUE;
					dbprintf("retrying\n");
				}

			}
		}
	} while (retry);


	if (*current_step_marker_ptr == combo->steps) {
		*current_step_marker_ptr = 0;
		*step_time_ptr = 0;
		*step_time_allowed_ptr = 0;
		dbprintf("### combo matched! ###\n");
#ifdef CONFIG_UNITTEST
		combo_matches++;
#endif
	}



	dbprintf("current step (after): %d\n", *current_step_marker_ptr);
	dbprintf("step time (after): %ld\n", *step_time_ptr);
	dbprintf("step time allowed (after): %ld\n", *step_time_allowed_ptr);

}

static void combo_process_switch(void) {

	// iterate over each combo and update the last-hit markers

	U8 combo_index = 0;

	for (combo_index = 0; combo_index < COMBO_COUNT; combo_index++) {
		combo_process_switch_for_combo(combo_index, machine_combos[combo_index]);
	}
}


//
// unit test code follows
//

typedef struct combo_test_data_item_s {
	U8 switch_to_trigger;
	U16 time_index_to_use;
} combo_test_data_item_t;

void test_combo(const U8 combo_id_to_test, const combo_def_t *combo_to_test, const combo_test_data_item_t test_data_items[], const U8 expected_step_markers[], U8 test_data_steps) {

	sw_last_scheduled_time = 10000; // using a number other than 0 to make it easier to spot the values
	U8 test_data_step;
	for (test_data_step = 0; test_data_step < test_data_steps; test_data_step++) {

		const combo_test_data_item_t *test_data_item = &test_data_items[test_data_step];

		dbprintf("\n**** STEP %d ****\n\n", test_data_step + 1);

		sw_last_scheduled = test_data_item->switch_to_trigger;
		sw_last_scheduled_time += test_data_item->time_index_to_use; // increment a fake system timer.
		dbprintf("fake timer now: %ld\n", sw_last_scheduled_time);

		combo_process_switch_for_combo(combo_id_to_test, combo_to_test);

		if (combo_id_to_test == UNITTEST_COMBO_ID) {
			assert_int_equal(expected_step_markers[test_data_step], unittest_current_step_marker);
		} else {
			assert_int_equal(expected_step_markers[test_data_step], current_step_markers[combo_id_to_test]);
		}
	}
}

static combo_test_data_item_t scenario_1_test_data_items[] = {
	// no timer data is used for this scenario, we're just interested in switch orders and position matching
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_LEFT_SLINGSHOT, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_LEFT_SLINGSHOT, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
};

void test_lr_rl_combo_for_scenario_1( void )
{
	dump_combo(&lr_rl_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_lr_rl_combo[] = {1,1,2,3,4,4,4,0,0,1,2};
	test_combo(LR_RL_COMBO_ID, &lr_rl_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_lr_combo_for_scenario_1( void )
{
	dump_combo(&rl_lr_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_rl_lr_combo[] = {0,0,1,0,1,1,1,2,3,4,0};
	test_combo(RL_LR_COMBO_ID, &rl_lr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_rl_combo_for_scenario_1( void )
{
	dump_combo(&rl_rl_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_rl_rl_combo[] = {0,0,1,0,1,1,1,2,3,3,4};
	test_combo(RL_RL_COMBO_ID, &rl_rl_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

void test_lr_lr_combo_for_scenario_1( void )
{
	dump_combo(&lr_lr_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_lr_lr_combo[] = {1,1,2,3,3,3,3,4,3,4,0};
	test_combo(LR_LR_COMBO_ID, &lr_lr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_ll_rr_combo_for_scenario_1( void )
{
	dump_combo(&ll_rr_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_ll_rr_combo[] = {1,2,4,3,4,0,0,1,0,1,0};
	test_combo(LL_RR_COMBO_ID, &ll_rr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_ll_rr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rr_ll_combo_for_scenario_1( void )
{
	dump_combo(&rr_ll_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_rr_ll_combo[] = {0,0,1,0,1,2,3,4,3,4,3};
	test_combo(RR_LL_COMBO_ID, &rr_ll_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rr_ll_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

/**
 * This sequence of switches should count 4 Left->Right->Right->Left orbits which uses each of the 3 rollovers and the opposite orbit at least once.
 */
static combo_test_data_item_t scenario_2_test_data_items[] = {
	// LARCLBRBLCRALRRL (where L/R = left/right orbits and A/B/C = left/middle/right rollovers
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_LEFT_ROLLOVER, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_RIGHT_ROLLOVER, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_MIDDLE_ROLLOVER, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_MIDDLE_ROLLOVER, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_RIGHT_ROLLOVER, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_LEFT_ROLLOVER, 0},
	{ SW_LEFT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_RIGHT_OUTER_LOOP, 0},
	{ SW_LEFT_OUTER_LOOP, 0}
};



void test_lr_rl_combo_for_scenario_2( void )
{
	dump_combo(&lr_rl_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_2_and_lr_rl_combo[] = {1,2,4,0,1,2,4,0,1,2,4,0,1,2,4,0};
	test_combo(LR_RL_COMBO_ID, &lr_rl_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_lr_rl_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

void test_ll_rr_combo_for_scenario_2( void )
{
	dump_combo(&ll_rr_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_2_and_ll_rr_combo[] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1};
	test_combo(LL_RR_COMBO_ID, &ll_rr_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_ll_rr_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

void test_rr_ll_combo_for_scenario_2( void )
{
	dump_combo(&rr_ll_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_2_and_rr_ll_combo[] = {0,0,1,0,0,0,1,0,0,0,1,0,0,1,2,4};
	test_combo(RR_LL_COMBO_ID, &rr_ll_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_rr_ll_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

static combo_def_t bad1_combo = {
	.name = "BAD1",
	.steps = 1,
	.step_list = {
		&cstp_wildcard_5sec // don't use just a wildcard!
	}
};

static combo_def_t bad2_combo = {
	.name = "BAD2",
	.steps = 2,
	.step_list = {
		&cstp_any_coin,
		&cstp_wildcard_5sec // don't end with a wildcard!
	}
};

static combo_def_t bad3_combo = {
	.name = "BAD3",
	.steps = 2,
	.step_list = {
		&cstp_wildcard_5sec, // don't start with a wildcard!
		&cstp_any_coin
	}
};

static combo_test_data_item_t scenario_3_test_data_items[] = {
	{ SW_LEFT_COIN, 0},
	{ SW_CENTER_COIN, 0},
	{ SW_RIGHT_COIN, 0},
	{ SW_FOURTH_COIN, 0}
};

extern U8 test_marker;

void test_bad1_combo_is_handled_ok( void )
{
	dump_combo(&bad1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_3_and_bad1_combo[] = {0,0,0,0};
	test_combo(UNITTEST_COMBO_ID, &bad1_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad1_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

void test_bad2_combo_is_handled_ok( void )
{
	dump_combo(&bad2_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_3_and_bad2_combo[] = {1,0,1,0};
	test_combo(UNITTEST_COMBO_ID, &bad2_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad2_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(2, combo_matches);
}

void test_bad3_combo_is_handled_ok( void )
{
	dump_combo(&bad3_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_3_and_bad3_combo[] = {0,0,0,0};
	test_combo(UNITTEST_COMBO_ID, &bad3_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad3_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

static combo_step_t cstp_left_standup1 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_1, 0 }
	}
};
static combo_step_t cstp_left_standup2 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_2, 0 }
	}
};
static combo_step_t cstp_left_standup3 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_3, 0 }
	}
};

static combo_def_t ordered_standup1_combo = {
	.name = "OSC 1",
	.steps = 5,
	.step_list = {
		&cstp_left_standup1,
		&cstp_wildcard_5sec,
		&cstp_left_standup2,
		&cstp_wildcard_5sec,
		&cstp_left_standup3
	}
};

static combo_test_data_item_t scenario_4_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},             // should match
	{ SW_RIGHT_SLINGSHOT, TIME_100MS},   // should match, and within 5s
	{ SW_LEFT_STANDUP_2, TIME_500MS},    // should match, and still within 5s
	{ SW_RIGHT_SLINGSHOT, TIME_250MS},   // TODO complete comments
	{ SW_LEFT_STANDUP_2, TIME_500MS},
	{ SW_RIGHT_SLINGSHOT, TIME_250MS},
	{ SW_LEFT_STANDUP_3, TIME_100MS}
};

void test_ordered_standup_combo_1_combo_is_matched_for_scenario4( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_4_and_ordered_standup1_combo[] = {1,2,3,4,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_4_test_data_items, expected_step_markers_for_scenario_4_and_ordered_standup1_combo, sizeof(scenario_4_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

static combo_test_data_item_t scenario_5_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_6S}
};

void test_ordered_standup_combo_1_combo_is_not_matched_for_scenario5( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_5_and_ordered_standup1_combo[] = {1,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_5_test_data_items, expected_step_markers_for_scenario_5_and_ordered_standup1_combo, sizeof(scenario_5_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

static combo_test_data_item_t scenario_6_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_6S}
};

void test_ordered_standup_combo_1_combo_is_not_matched_for_scenario6( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_6_and_ordered_standup1_combo[] = {1,3,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_6_test_data_items, expected_step_markers_for_scenario_6_and_ordered_standup1_combo, sizeof(scenario_6_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

static combo_test_data_item_t scenario_7_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_6S}
};

void test_ordered_standup_combo_1_combo_is_not_matched_for_scenario7( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_7_and_ordered_standup1_combo[] = {1,3,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_7_test_data_items, expected_step_markers_for_scenario_7_and_ordered_standup1_combo, sizeof(scenario_7_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

static combo_test_data_item_t scenario_8_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_1, TIME_1S}, // same as previous, but it's a wildcard
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_1, TIME_1S},
	{ SW_LEFT_STANDUP_1, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_1S}
};

void test_ordered_standup_combo_1_combo_is_matched_for_scenario8( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_8_and_ordered_standup1_combo[] = {1,2,3,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_8_test_data_items, expected_step_markers_for_scenario_8_and_ordered_standup1_combo, sizeof(scenario_8_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

static combo_test_data_item_t scenario_9_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_1, TIME_5S}, // time since previous switches was too high to match, but it's the same switch so start again.
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_1, TIME_1S},
	{ SW_LEFT_STANDUP_1, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_1S}
};

void test_ordered_standup_combo_1_combo_is_matched_for_scenario9( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_9_and_ordered_standup1_combo[] = {1,1,3,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_9_test_data_items, expected_step_markers_for_scenario_9_and_ordered_standup1_combo, sizeof(scenario_9_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

/**
 * This test illustrates the problem faces with matching the switches in reverse order which was initially tried
 * What would happen is that the combo would be matched twice.
 */
static combo_test_data_item_t scenario_10_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_1S} // press the same switch again
};

void test_ordered_standup_combo_1_combo_is_matched_only_once_for_scenario10( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_10_and_ordered_standup1_combo[] = {1,3,0,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_10_test_data_items, expected_step_markers_for_scenario_10_and_ordered_standup1_combo, sizeof(scenario_10_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_fixture_combos( void )
{
	test_fixture_start();

	run_test(test_ordered_standup_combo_1_combo_is_matched_for_scenario4);
	run_test(test_ordered_standup_combo_1_combo_is_not_matched_for_scenario5);
	run_test(test_ordered_standup_combo_1_combo_is_not_matched_for_scenario6);
	run_test(test_ordered_standup_combo_1_combo_is_not_matched_for_scenario7);
	run_test(test_ordered_standup_combo_1_combo_is_matched_for_scenario8);
	run_test(test_ordered_standup_combo_1_combo_is_matched_for_scenario9);
	run_test(test_ordered_standup_combo_1_combo_is_matched_only_once_for_scenario10);

	run_test(test_bad1_combo_is_handled_ok);
	run_test(test_bad2_combo_is_handled_ok);
	run_test(test_bad3_combo_is_handled_ok);

	run_test(test_lr_rl_combo_for_scenario_1);
	run_test(test_rl_lr_combo_for_scenario_1);
	run_test(test_rl_rl_combo_for_scenario_1);
	run_test(test_lr_lr_combo_for_scenario_1);
	run_test(test_ll_rr_combo_for_scenario_1);
	run_test(test_rr_ll_combo_for_scenario_1);

	run_test(test_lr_rl_combo_for_scenario_2);
	run_test(test_ll_rr_combo_for_scenario_2);
	run_test(test_rr_ll_combo_for_scenario_2);

	test_fixture_end();
}
