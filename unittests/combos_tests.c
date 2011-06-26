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
		dbprintf("step %d >\n", step);
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
#define LL_LL_COMBO_ID 4
#define RR_RR_COMBO_ID 5

//
// from combos.c
//

#ifdef CONFIG_UNITTEST
U8 combo_matches; // a counter so combo matches can be tested
#endif

void combo_reset_current_step_markers(void) {
	memset(current_step_markers, 0x00, sizeof(current_step_markers));
}


const combo_switch_t *combo_match_switch_to_steps(const U8 switch_id, const combo_step_t *combo_step) {
	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("checking switch: %d, Time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);

		if (switch_id == combo_switch->switch_id) {
			dbprintf("switch matches!\n");
			return combo_switch;
		}
	}
	return 0;
}

/**
 * expects sw_last_scheduled to be set appropriately
 */
static void combo_process_switch_for_combo(U8 combo_id, combo_def_t *combo) {
#ifdef DEBUGGER
	dbprintf("processing combo: %d\n", combo_id);
	dump_switch_details(sw_last_scheduled);
	printf("combo: %s\n", combo->name);
#endif

	U8 retry;
	bool advance;
	U16 time_allowed = 0;
	const combo_switch_t *matched_switch = 0;
	U8 *current_step_marker_ptr = &current_step_markers[combo_id];
	dbprintf("current step (before): %d\n", *current_step_marker_ptr);

	do {
		if (retry) {
			dbprintf("current step (retry): %d\n", *current_step_marker_ptr);
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
			dbprintf("last step\n");
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
			matched_switch = combo_match_switch_to_steps(sw_last_scheduled, next_step);
			if (matched_switch) {
				advance = TRUE;
				time_allowed = matched_switch->time_allowed;
			}
		} else {
			dbprintf("switch matches wildcard step!\n");
			advance = TRUE;
			time_allowed = next_step->time_allowed;
		}

		if (advance) {
			*current_step_marker_ptr = (*current_step_marker_ptr) + 1;
		} else {
			dbprintf("unmatched\n");
			if (current_step) {
				if (*current_step_marker_ptr > 1 && combo->step_list[(*current_step_marker_ptr) - 2]->switches == 0) {
					dbprintf("preceding switch IS wildcard\n");
					dbprintf("resetting market to preceding\n");
					*current_step_marker_ptr = (*current_step_marker_ptr) - 1; // start again at the previous step.
				} else {
					dbprintf("preceding switch is NOT wildcard\n");
					dbprintf("resetting marker to start\n");
					*current_step_marker_ptr = 0; // start again at the first step.
				}
				retry = TRUE;
				dbprintf("retrying\n");

			}
		}
	} while (retry);

	if (*current_step_marker_ptr == combo->steps) {
		*current_step_marker_ptr = 0;
		dbprintf("### combo matched! ###\n");
#ifdef CONFIG_UNITTEST
		combo_matches++;
#endif
	}



	dbprintf("current step (after): %d\n", *current_step_marker_ptr);

}

static void combo_process_switch(void) {

	// iterate over each combo and update the last-hit markers

	U8 combo_index = 0;

	for (combo_index = 0; combo_index < COMBO_COUNT; combo_index++) {
		combo_process_switch_for_combo(combo_index, machine_combos[combo_index]);
	}
}

typedef struct combo_test_data_item_s {
	U8 switch_to_trigger;
	U8 time_index_to_use;
	U8 expected_step_marker;
} combo_test_data_item_t;

void test_combo(const U8 combo_id_to_test, const combo_test_data_item_t test_data_items[], const U8 expected_step_markers[], U8 test_data_steps) {

	U8 test_data_step;
	for (test_data_step = 0; test_data_step < test_data_steps; test_data_step++) {

		const combo_test_data_item_t *test_data_item = &test_data_items[test_data_step];

		dbprintf("\n**** STEP %d ****\n\n", test_data_step + 1);

		sw_last_scheduled = test_data_item->switch_to_trigger;
		sw_last_scheduled_time = test_data_item->time_index_to_use;

		combo_process_switch_for_combo(combo_id_to_test, machine_combos[combo_id_to_test]);

		assert_int_equal(expected_step_markers[test_data_step], current_step_markers[combo_id_to_test]);
	}
}

static combo_test_data_item_t scenario_1_test_data_items[] = {
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
	test_combo(LR_RL_COMBO_ID, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_lr_combo_for_scenario_1( void )
{
	dump_combo(&rl_lr_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_rl_lr_combo[] = {0,0,1,0,1,1,1,2,3,4,0};
	test_combo(RL_LR_COMBO_ID, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_rl_combo_for_scenario_1( void )
{
	dump_combo(&rl_rl_combo); // XXX
	combo_reset_current_step_markers();
	combo_matches = 0;
	static U8 expected_step_markers_for_scenario_1_and_rl_rl_combo[] = {0,0,1,0,1,1,1,2,3,3,4};
	test_combo(RL_RL_COMBO_ID, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}


void test_fixture_combos( void )
{
	test_fixture_start();
	run_test(test_lr_rl_combo_for_scenario_1);
	run_test(test_rl_lr_combo_for_scenario_1);
	run_test(test_rl_rl_combo_for_scenario_1);
	test_fixture_end();
}
