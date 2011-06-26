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


static combo_def_t lr_lr_combo = {
	.name = "LR LR",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

static void dump_combo_step(const combo_step_t *combo_step) {
	dbprintf("switches: %d\n", combo_step->switches);

	U8 switch_index;
	for (switch_index = 0; switch_index < combo_step->switches; switch_index ++) {
		const combo_switch_t *combo_switch = &combo_step->switch_list[switch_index];
		dbprintf("Switch: %d, Time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);
	}
}

static void dump_combo(const combo_def_t *combo) {
	dbprintf("Combo: %s\n", combo->name);
	dbprintf("steps: %d\n", combo->steps);

	U8 step;
	for (step = 0; step < combo->steps; step ++) {
		dbprintf("Step %d >\n", step);
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
#define COMBO_COUNT 1
U8 current_step_markers[COMBO_COUNT]; // 1-based index
static combo_def_t *machine_combos[COMBO_COUNT] = {
	&lr_lr_combo
};

#define COMBO_ID_LR_LR_COMBO 0


void combo_reset_current_step_markers(void) {
	memset(current_step_markers, 0x00, sizeof(current_step_markers));
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

	//
	// determine current and next step
	//
	U8 *current_step_marker_ptr = &current_step_markers[combo_id];

	dbprintf("current step (before): %d\n", *current_step_marker_ptr);
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

	bool advance = FALSE;
	U16 time_allowed = 0;

	//
	// attempt to match switch against next step's switches
	//

	if (next_step->switches) {
		U8 switch_index;
		for (switch_index = 0; switch_index < next_step->switches; switch_index ++) {
			const combo_switch_t *combo_switch = &next_step->switch_list[switch_index];
			dbprintf("Checking switch: %d, Time: %d\n", combo_switch->switch_id, combo_switch->time_allowed);

			if (sw_last_scheduled == combo_switch->switch_id) {
				dbprintf("switch matches!\n");
				advance = TRUE;
				time_allowed = combo_switch->time_allowed;
				break;
			}

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
		if (current_step && current_step->switches == 0) {
			dbprintf("preceding switch is NOT wildcard\n");
			dbprintf("resetting marker\n");
			*current_step_marker_ptr = 0;
		}
	}

	if (*current_step_marker_ptr == combo->steps) {
		*current_step_marker_ptr = 0;
		dbprintf("Combo matched!\n");
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

		sw_last_scheduled = test_data_item->switch_to_trigger;
		sw_last_scheduled_time = test_data_item->time_index_to_use;

		combo_process_switch();

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

void test_lr_lr_combo_for_scenario_1( void )
{
	dump_combo(&lr_lr_combo); // XXX

	combo_reset_current_step_markers();

	U8 expected_step_markers_for_scenario_1_and_lr_lr_combo[] = {1,1,2,3,4,4,4,0,0,1,2};

	test_combo(COMBO_ID_LR_LR_COMBO, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
}


void test_fixture_combos( void )
{
	test_fixture_start();
	run_test(test_lr_lr_combo_for_scenario_1);
	test_fixture_end();
}
