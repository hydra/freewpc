#include <freewpc.h>
#include "seatest.h"

/*
 * switches_tests.c
 *
 *  Created on: 25 Jun 2011
 *      Author: Hydra
 */

extern __fastram__ switch_bits_t sw_logical;

void test_that_switch_poll_on_inactive_switch_returns_false( void ) {
	U8 row = 0;
	U8 col = 0;
	for (col = 0; col < 8; col ++) {

		// save the vlaue and restore it later so we don't break other tests
		U8 old_value = sw_logical[col];
		for (row = 0; row < 8; row ++) {
			// set all bits on except the bit for the switch that should be off
			sw_logical[col] = (U8)~(1<<row);
			//printf("sw_logical[%d] = %d\n", row, sw_logical[col]);

			U8 switch_number = col * 8 + row;
			bool result = switch_poll(switch_number);
			assert_false(result);
		}
		sw_logical[col] = old_value;
	}
}

void test_that_switch_poll_on_active_switch_returns_true( void ) {

	U8 row = 0;
	U8 col = 0;
	for (col = 0; col < 8; col ++) {

		// save the vlaue and restore it later so we don't break other tests
		U8 old_value = sw_logical[col];
		for (row = 0; row < 8; row ++) {
			// set all bits off except the bit for the switch that should be on
			sw_logical[col] = (U8)(1<<row);
			//printf("sw_logical[%d] = %d\n", row, sw_logical[col]);

			U8 switch_number = col * 8 + row;
			bool result = switch_poll(switch_number);
			assert_true(result);
		}
		sw_logical[col] = old_value;
	}
}

extern void switch_short_detect (void);

void test_that_switch_row_short_detected( void )
{
	// setup
	extern U8 sw_short_timer;
	sw_short_timer = 0;

	// short row 8
	U8 i = 0;
	for (i = 0; i < 8; i ++) {
		sw_raw[i] = (1 << 7);
	}

	switch_short_detect();

	assert_true(sw_short_timer == 3);
}

void test_that_switch_column_short_detected( void )
{
	// setup
	extern U8 sw_short_timer;
	sw_short_timer = 0;

	// reset raw switches
	U8 i = 0;
	for (i = 0; i < 8; i ++) {
		sw_raw[i] = mach_opto_mask[0];
	}
	// short column 8
	sw_raw[7] = (U8)~mach_opto_mask[7];

	switch_short_detect();

	assert_true(sw_short_timer == 3);
}

void test_fixture_switches( void )
{
        test_fixture_start();
        run_test(test_that_switch_poll_on_inactive_switch_returns_false);
        run_test(test_that_switch_poll_on_active_switch_returns_true);
        run_test(test_that_switch_row_short_detected);
        run_test(test_that_switch_column_short_detected);
        test_fixture_end();
}
