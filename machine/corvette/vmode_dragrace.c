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

/**
 * TODO pause the system timer while a drag race is running
 */
#include <freewpc.h>
#include <corvette/racetrack.h>
#include <corvette/vmode_dragrace.h>

//
// DRAGRACE
//
// a video mode where the player bashes alternate flipper buttons to get the blue car to outrun the red
// car, ala track-and-field.
// Each drag-race one should increase the difficulty of the next one
// The more cars you have collected, the easier it should be to win a drag race (as you'd be using your best car right!)
// If the player wins a drag race award them something suitable (for now, just points).

U8 dragrace_counter_max;
U8 dragrace_counter;

U8 player_car_position;
U8 computer_car_position;

/** holds the gid of the last thing to start a dragrace, for use in dragrace_won/dragrace_lost callset handlers */
U8 dragrace_starter_gid;

enum dragrace_button_expectations {
	EXPECT_LEFT = 0,
	EXPECT_RIGHT
};
enum dragrace_button_expectations dragrace_button_expectation;

#define DRAGRACE_TICKS_PER_SECOND 10 // see dragrace_deff()

__local__ U8 dragraces_won; // starts at 0, increases by one for each drag-race won.

CALLSET_ENTRY(dragrace, start_game) {
	dragraces_won = 0;
}

void dragrace_draw( void ) {
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 10, "DRAGRACE");

	// TODO replace these counters with pretty dragrace progress animation/graphics.

	sprintf("%d", dragrace_counter_max - dragrace_counter); // count down
	font_render_string_center (&font_supercar9, 64, 20, sprintf_buffer);

	sprintf("%d", player_car_position);
	font_render_string_center (&font_supercar9, 32, 20, sprintf_buffer);
	sprintf("%d", computer_car_position);
	font_render_string_center (&font_supercar9, 96, 20, sprintf_buffer);

	dmd_show_low ();
}

void dragrace_start_anim (void) {
	//ball_search_timer_reset();

	// TODO show graphic of racetrack starting grid or something suitable

	// ready
	lamp_tristate_on(LM_TREE_TOP_YELLOW);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "READY");
	dmd_show_low ();
	sample_start (SND_RACE_STARTER_01, SL_500MS);
	task_sleep_sec(1);

	// set
	lamp_tristate_on(LM_TREE_BOTTOM_YELLOW);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "SET");
	dmd_show_low ();
	sample_start (SND_RACE_STARTER_01, SL_500MS);
	task_sleep_sec(1);

	// go
	lamp_tristate_flash(LM_LEFT_TREE_GREEN);
	lamp_tristate_flash(LM_RIGHT_TREE_GREEN);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "GO");
	dmd_show_low ();
	sample_start (SND_RACE_STARTER_02, SL_2S);
}

void dragrace_loser_anim(void) {

	dbprintf ("dragrace_loser_anim: start\n");
	//ball_search_timer_reset();

	// TODO sound
	// TODO animation/graphics
	lamp_tristate_off(LM_LEFT_TREE_GREEN);
	lamp_tristate_on(LM_RIGHT_TREE_GREEN);
	lamp_tristate_on(LM_LEFT_TREE_RED);
	lamp_tristate_off(LM_RIGHT_TREE_RED);

	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "LOSER");
	dmd_show_low ();

	task_sleep_sec(3);
	dbprintf ("dragrace_loser_anim: exit\n");
}

void dragrace_winner_anim(void) {

	dbprintf ("dragrace_winner_anim: start\n");
	//ball_search_timer_reset();

	// TODO sound
	// TODO animation/graphics
	lamp_tristate_on(LM_LEFT_TREE_GREEN);
	lamp_tristate_off(LM_RIGHT_TREE_GREEN);
	lamp_tristate_off(LM_LEFT_TREE_RED);
	lamp_tristate_on(LM_RIGHT_TREE_RED);

	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "WINNER");
	dmd_show_low ();

	task_sleep_sec(3);
	dbprintf ("dragrace_winner_anim: exit\n");
}

void dragrace_deff (void) {
	dbprintf ("dragrace_deff: start\n");

	dbprintf ("dragraces won: %d\n", dragraces_won);
	// TODO wait for cars to return if racetrack is working while resetting ball search timer

	dragrace_start_anim();

	racetrack_race();

	// start with an 8 second race and shorten by one second for each dragrace won.
	dragrace_counter_max = (DRAGRACE_TICKS_PER_SECOND * 8) - (dragraces_won * DRAGRACE_TICKS_PER_SECOND);
	if (dragrace_counter_max < DRAGRACE_TICKS_PER_SECOND * 5) {
		// race must be at least 5 seconds long
		dragrace_counter_max = DRAGRACE_TICKS_PER_SECOND * 5;
	}
	dbprintf ("dragraces won (before): %d\n", dragraces_won);

	player_car_position = 0;
	computer_car_position = 0;
	dragrace_button_expectation = EXPECT_LEFT;
	dragrace_counter = 0;

	//for (dragrace_counter = 0; dragrace_counter < dragrace_counter_max && player_car_position != 100 && computer_car_position != 100; dragrace_counter++) {
	while (1) {
		dbprintf ("dragrace status:\n");
		dbprintf ("counter: %d\n", dragrace_counter);
		dbprintf ("computer_car_position: %d\n", computer_car_position);
		dbprintf ("player_car_position: %d\n", player_car_position);
		dbprintf ("dragrace_button_expectation: %d\n", dragrace_button_expectation);
		if (dragrace_counter == dragrace_counter_max) {
			break;
		}
		if (player_car_position == 100) {
			break;
		}
		if (computer_car_position == 100) {
			break;
		}
		dragrace_counter++;

		// FIXME calling this causes a random crash!
		//ball_search_timer_reset();

		// c = dragrace_counter, cm = dragrace_counter_max, ccp = computer_car_position
		// Hard Race: 20(c) / 50(cm) = 0.4, 0.4 * 100 = 40% = ccp
		// Easy Race: 40(c) / 100(cm) = 0.2, 0.2 * 100 = 40% = ccp
		computer_car_position = (dragrace_counter * (U16)100) / dragrace_counter_max;


		racetrack_set_desired_car_position(LANE_RIGHT, computer_car_position);
		racetrack_set_desired_car_position(LANE_LEFT, player_car_position);

		// TODO engine sounds

		// leave 'GO' on the screen for 1 second, after which draw the race status

		if (dragrace_counter > DRAGRACE_TICKS_PER_SECOND) {
			dragrace_draw();
		}

		task_sleep(TIME_100MS); // if changed adjust DRAGRACE_TICKS_PER_SECOND
	}

	lamp_tristate_off(LM_TREE_TOP_YELLOW);
	lamp_tristate_off(LM_TREE_BOTTOM_YELLOW);

	if (player_car_position == 100 && computer_car_position < 100) {
		dragraces_won++;
		dragrace_winner_anim();
	} else {
		dragrace_loser_anim();
	}

	lamp_tristate_off(LM_LEFT_TREE_GREEN);
	lamp_tristate_off(LM_RIGHT_TREE_GREEN);
	lamp_tristate_off(LM_LEFT_TREE_RED);
	lamp_tristate_off(LM_RIGHT_TREE_RED);


	dbprintf ("dragraces won (after): %d\n", dragraces_won);

	dbprintf ("dragrace_deff: exit\n");
	deff_exit ();
}

void dragrace_stop( void ) {
	flipper_enable();
	ball_search_monitor_start();
	racetrack_car_return();
	global_flag_off(GLOBAL_FLAG_DRAGRACE_IN_PROGRESS);
	task_kill_gid(GID_DRAGRACE_TASK);
}

void dragrace_task( void ) {
	deff_start_sync(DEFF_DRAGRACE);
	// fire off appropriate events
	// must be done outside of the deff otherwise other deffs won't start due to deff priorities
	if (player_car_position == 100 && computer_car_position < 100) {
		callset_invoke(vmode_dragrace_won);
	} else {
		callset_invoke(vmode_dragrace_lost);
	}
	dragrace_stop();
	task_exit();
}

void dragrace_start( U8 starter_gid ) {
	if (global_flag_test(GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return;
	}
	dragrace_starter_gid = starter_gid;
	global_flag_on(GLOBAL_FLAG_DRAGRACE_IN_PROGRESS);
	flipper_disable();
	ball_search_monitor_stop();
	racetrack_car_return();
	task_kill_gid(GID_DRAGRACE_TASK);
	task_create_gid1(GID_DRAGRACE_TASK, dragrace_task);
}

CALLSET_ENTRY (dragrace, sw_left_button) {
	if (!global_flag_test (GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return;
	}
	if (dragrace_button_expectation != EXPECT_LEFT || player_car_position == 100) {
		return;
	}
	dragrace_button_expectation = EXPECT_RIGHT;
	player_car_position++;
}

CALLSET_ENTRY (dragrace, sw_right_button) {
	if (!global_flag_test (GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return;
	}
	if (dragrace_button_expectation != EXPECT_RIGHT || player_car_position == 100) {
		return;
	}
	dragrace_button_expectation = EXPECT_LEFT;
	player_car_position++;
}

// these empty two callsets are just to shut the linker up.

CALLSET_ENTRY(dragrace, ball_start) {
	dragrace_starter_gid = 0;
}

CALLSET_BOOL_ENTRY(dragrace, dev_route_66_popper_kick_request) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return TRUE;
	}

	// hold the kickout for a bit.
	return FALSE;
}

void dragrace_disable( void ) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}

	lamp_tristate_off(LM_RACE_TODAY);
	lamp_tristate_off(LM_ROUTE_66_ARROW);

	global_flag_off(GLOBAL_FLAG_DRAGRACE_ENABLED);
	global_flag_off(GLOBAL_FLAG_DIVERTER_OPENED);
}

void dragrace_enable( void ) {
	global_flag_on(GLOBAL_FLAG_DRAGRACE_ENABLED);
	global_flag_on(GLOBAL_FLAG_DIVERTER_OPENED);
}

CALLSET_ENTRY(dragrace, dev_route_66_popper_enter) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}
	// kill the shot/arrow lights
	dragrace_disable();

	dragrace_start(GID_DRAGRACE);
}

CALLSET_ENTRY (dragrace, lamp_update) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}

	lamp_tristate_flash(LM_RACE_TODAY);
	lamp_tristate_flash(LM_ROUTE_66_ARROW);
}

CALLSET_ENTRY(dragrace, vmode_dragrace_won) {
	if (!was_dragrace_started_by(GID_DRAGRACE)) {
		return;
	}
	score(SC_25M); // TODO score based on how hard the race was?
}

CALLSET_ENTRY(dragrace, vmode_dragrace_lost) {
}
