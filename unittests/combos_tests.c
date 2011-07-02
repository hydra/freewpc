#include <freewpc.h>
#include "seatest.h"

/*
 * combos_tests.c
 *
 *  Created on: 25 Jun 2011
 *      Author: Hydra
 */

extern void combo_process_switch_for_combo(const U8 combo_id, const combo_def_t *combo);
extern U8 combo_matches;
extern U8 unittest_current_step_marker;
extern U16 unittest_step_time;
extern U16 unittest_step_time_allowed;

const combo_step_t cstp_any_coin = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_COIN, 0},
		{ SW_CENTER_COIN, 0},
		{ SW_RIGHT_COIN, 0},
		{ SW_FOURTH_COIN, 0}
	}
};


//
// from mach-combos.c
//
extern void combo_reset_current_step_markers(void);
extern combo_def_t lr_rl_combo;
extern combo_def_t rl_lr_combo;
extern combo_def_t rl_rl_combo;
extern combo_def_t lr_lr_combo;
extern combo_def_t ll_rr_combo;
extern combo_def_t rr_ll_combo;
extern combo_step_t cstp_wildcard_5sec;

//
// from switches.c
//
U16 sw_last_scheduled_time; // TODO
extern U8 sw_last_scheduled;

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

const combo_test_data_item_t scenario_1_test_data_items[] = {
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
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_lr_rl_combo[] = {1,1,2,3,4,4,4,0,0,1,2};
	test_combo(UNITTEST_COMBO_ID, &lr_rl_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_lr_combo_for_scenario_1( void )
{
	dump_combo(&rl_lr_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_rl_lr_combo[] = {0,0,1,0,1,1,1,2,3,4,0};
	test_combo(UNITTEST_COMBO_ID, &rl_lr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rl_rl_combo_for_scenario_1( void )
{
	dump_combo(&rl_rl_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_rl_rl_combo[] = {0,0,1,0,1,1,1,2,3,3,4};
	test_combo(UNITTEST_COMBO_ID, &rl_rl_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rl_rl_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

void test_lr_lr_combo_for_scenario_1( void )
{
	dump_combo(&lr_lr_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_lr_lr_combo[] = {1,1,2,3,3,3,3,4,3,4,0};
	test_combo(UNITTEST_COMBO_ID, &lr_lr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_lr_lr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_ll_rr_combo_for_scenario_1( void )
{
	dump_combo(&ll_rr_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_ll_rr_combo[] = {1,2,4,3,4,0,0,1,0,1,0};
	test_combo(UNITTEST_COMBO_ID, &ll_rr_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_ll_rr_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

void test_rr_ll_combo_for_scenario_1( void )
{
	dump_combo(&rr_ll_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_1_and_rr_ll_combo[] = {0,0,1,0,1,2,3,4,3,4,3};
	test_combo(UNITTEST_COMBO_ID, &rr_ll_combo, scenario_1_test_data_items, expected_step_markers_for_scenario_1_and_rr_ll_combo, sizeof(scenario_1_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

/**
 * This sequence of switches should count 4 Left->Right->Right->Left orbits which uses each of the 3 rollovers and the opposite orbit at least once.
 */
const combo_test_data_item_t scenario_2_test_data_items[] = {
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
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_2_and_lr_rl_combo[] = {1,2,4,0,1,2,4,0,1,2,4,0,1,2,4,0};
	test_combo(UNITTEST_COMBO_ID, &lr_rl_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_lr_rl_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

void test_ll_rr_combo_for_scenario_2( void )
{
	dump_combo(&ll_rr_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_2_and_ll_rr_combo[] = {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1};
	test_combo(UNITTEST_COMBO_ID, &ll_rr_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_ll_rr_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

void test_rr_ll_combo_for_scenario_2( void )
{
	dump_combo(&rr_ll_combo); // XXX
	combo_reset_current_step_markers();
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_2_and_rr_ll_combo[] = {0,0,1,0,0,0,1,0,0,0,1,0,0,1,2,4};
	test_combo(UNITTEST_COMBO_ID, &rr_ll_combo, scenario_2_test_data_items, expected_step_markers_for_scenario_2_and_rr_ll_combo, sizeof(scenario_2_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

const combo_def_t bad1_combo = {
	.name = "BAD1",
	.steps = 1,
	.step_list = {
		&cstp_wildcard_5sec // don't use just a wildcard!
	}
};

const combo_def_t bad2_combo = {
	.name = "BAD2",
	.steps = 2,
	.step_list = {
		&cstp_any_coin,
		&cstp_wildcard_5sec // don't end with a wildcard!
	}
};

const combo_def_t bad3_combo = {
	.name = "BAD3",
	.steps = 2,
	.step_list = {
		&cstp_wildcard_5sec, // don't start with a wildcard!
		&cstp_any_coin
	}
};

const combo_test_data_item_t scenario_3_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_3_and_bad1_combo[] = {0,0,0,0};
	test_combo(UNITTEST_COMBO_ID, &bad1_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad1_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

void test_bad2_combo_is_handled_ok( void )
{
	dump_combo(&bad2_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_3_and_bad2_combo[] = {1,0,1,0};
	test_combo(UNITTEST_COMBO_ID, &bad2_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad2_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(2, combo_matches);
}

void test_bad3_combo_is_handled_ok( void )
{
	dump_combo(&bad3_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_3_and_bad3_combo[] = {0,0,0,0};
	test_combo(UNITTEST_COMBO_ID, &bad3_combo, scenario_3_test_data_items, expected_step_markers_for_scenario_3_and_bad3_combo, sizeof(scenario_3_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(4, combo_matches);
}

const combo_step_t cstp_left_standup1 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_1, 0 }
	}
};
const combo_step_t cstp_left_standup2 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_2, 0 }
	}
};
const combo_step_t cstp_left_standup3 = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_STANDUP_3, 0 }
	}
};

const combo_def_t ordered_standup1_combo = {
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

const combo_test_data_item_t scenario_4_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_4_and_ordered_standup1_combo[] = {1,2,3,4,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_4_test_data_items, expected_step_markers_for_scenario_4_and_ordered_standup1_combo, sizeof(scenario_4_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

const combo_test_data_item_t scenario_5_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_6S}
};

void test_ordered_standup_combo_1_combo_is_not_matched_for_scenario5( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_5_and_ordered_standup1_combo[] = {1,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_5_test_data_items, expected_step_markers_for_scenario_5_and_ordered_standup1_combo, sizeof(scenario_5_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

const combo_test_data_item_t scenario_6_test_data_items[] = {
	{ SW_LEFT_STANDUP_1, 0},
	{ SW_LEFT_STANDUP_2, TIME_1S},
	{ SW_LEFT_STANDUP_3, TIME_6S}
};

void test_ordered_standup_combo_1_combo_is_not_matched_for_scenario6( void )
{
	dump_combo(&ordered_standup1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_6_and_ordered_standup1_combo[] = {1,3,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_6_test_data_items, expected_step_markers_for_scenario_6_and_ordered_standup1_combo, sizeof(scenario_6_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

const combo_test_data_item_t scenario_7_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_7_and_ordered_standup1_combo[] = {1,3,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_7_test_data_items, expected_step_markers_for_scenario_7_and_ordered_standup1_combo, sizeof(scenario_7_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
}

const combo_test_data_item_t scenario_8_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_8_and_ordered_standup1_combo[] = {1,2,3,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_8_test_data_items, expected_step_markers_for_scenario_8_and_ordered_standup1_combo, sizeof(scenario_8_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

const combo_test_data_item_t scenario_9_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_9_and_ordered_standup1_combo[] = {1,1,3,4,4,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_9_test_data_items, expected_step_markers_for_scenario_9_and_ordered_standup1_combo, sizeof(scenario_9_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

/**
 * This test illustrates the problem faced with matching the switches in reverse order which was initially tried
 * What would happen is that the combo would be matched twice.
 */
const combo_test_data_item_t scenario_10_test_data_items[] = {
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
	const U8 expected_step_markers_for_scenario_10_and_ordered_standup1_combo[] = {1,3,0,0};
	test_combo(UNITTEST_COMBO_ID, &ordered_standup1_combo, scenario_10_test_data_items, expected_step_markers_for_scenario_10_and_ordered_standup1_combo, sizeof(scenario_10_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

#define SW_UNITTEST_A 1
#define SW_UNITTEST_B 2
#define SW_UNITTEST_C 3
#define SW_UNITTEST_D 4
#define SW_UNITTEST_E 5
#define SW_UNITTEST_F 6
#define SW_UNITTEST_G 7
#define SW_UNITTEST_H 8
#define SW_UNITTEST_I 9
#define SW_UNITTEST_J 10
#define SW_UNITTEST_K 11
#define SW_UNITTEST_L 12
#define SW_UNITTEST_M 13
#define SW_UNITTEST_N 14
#define SW_UNITTEST_O 15
#define SW_UNITTEST_P 16
#define SW_UNITTEST_Q 17
#define SW_UNITTEST_R 18
#define SW_UNITTEST_S 19
#define SW_UNITTEST_T 20
#define SW_UNITTEST_U 21
#define SW_UNITTEST_V 22
#define SW_UNITTEST_W 23
#define SW_UNITTEST_X 24
#define SW_UNITTEST_Y 25
#define SW_UNITTEST_Z 26

const combo_step_t cstp_test_a = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_UNITTEST_A, TIME_2S}
	}
};

const combo_step_t cstp_test_b = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_UNITTEST_B, TIME_2S}
	}
};

const combo_step_t cstp_test_c = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_UNITTEST_C, TIME_2S}
	}
};

const combo_step_t cstp_test_l = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_UNITTEST_L, TIME_2S}
	}
};


const combo_def_t test1_combo = {
	.name = "TEST1",
	.steps = 5,
	.step_list = {
		&cstp_test_l,
		&cstp_wildcard_5sec,
		&cstp_test_a,
		&cstp_test_b,
		&cstp_test_c
	}
};

const combo_test_data_item_t scenario_11_test_data_items[] = {
	{ SW_UNITTEST_L, 0},
	{ SW_UNITTEST_A, TIME_1S},
	{ SW_UNITTEST_B, TIME_1S},
	{ SW_UNITTEST_D, TIME_3S},
	{ SW_UNITTEST_A, TIME_1S - 1},  // the time allowed to hit this should be the time allowed for the wildcard (5s) taken from step 2.
	{ SW_UNITTEST_B, TIME_1S},
	{ SW_UNITTEST_C, TIME_1S}
};

void test_test1_combo_is_matched_for_scenario11( void )
{
	dump_combo(&test1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_11_and_test1_combo[] = {1,3,4,2,3,4,0};
	test_combo(UNITTEST_COMBO_ID, &test1_combo, scenario_11_test_data_items, expected_step_markers_for_scenario_11_and_test1_combo, sizeof(scenario_11_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(1, combo_matches);
}

const combo_test_data_item_t scenario_12_test_data_items[] = {
	{ SW_UNITTEST_L, 0},
	{ SW_UNITTEST_A, TIME_1S},
	{ SW_UNITTEST_B, TIME_1S},
	{ SW_UNITTEST_D, TIME_3S},
	{ SW_UNITTEST_A, TIME_1S}, // the player hit the switch just 1 cycle late.
};

void test_test1_combo_is_not_matched_for_scenario12( void )
{
	dump_combo(&test1_combo); // XXX
	unittest_current_step_marker = 0;
	combo_matches = 0;
	const U8 expected_step_markers_for_scenario_12_and_test1_combo[] = {1,3,4,2,0};
	test_combo(UNITTEST_COMBO_ID, &test1_combo, scenario_12_test_data_items, expected_step_markers_for_scenario_12_and_test1_combo, sizeof(scenario_12_test_data_items) / sizeof(combo_test_data_item_t));
	assert_int_equal(0, combo_matches);
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
	run_test(test_test1_combo_is_matched_for_scenario11);
	run_test(test_test1_combo_is_not_matched_for_scenario12);

	test_fixture_end();
}
