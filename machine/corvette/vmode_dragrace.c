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
#include <corvette/cars.h>

#include <corvette/racetrack.h>
#include <corvette/vmode_dragrace.h>
#include <button_expectation.h>

extern __local__ U8 cars_collected;
extern char *car_names[];

//
// DRAGRACE
//
// a video mode where the player bashes alternate flipper buttons to get the blue car to outrun the red
// car, ala track-and-field.
// Each drag-race one should increase the difficulty of the next one
// The more cars you have collected, the easier it should be to win a drag race (as you'd be using your best car right!)
// If the player wins a drag race award them something suitable (for now, just points).

U8 dragrace_counter;

U8 player_car_position;
U8 computer_car_position;

/** holds the gid of the last thing to start a dragrace, for use in dragrace_won/dragrace_lost callset handlers */
U8 dragrace_starter_gid;

#define DRAGRACE_TICKS_PER_SECOND 10 // see dragrace_deff()

__local__ U8 total_dragraces_won; // starts at 0, increases by one for each drag-race won.

enum car_list computer_car;
enum car_list player_car;

CALLSET_ENTRY(dragrace, start_game) {
	total_dragraces_won = 0;
}

void dragrace_draw( void ) {
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 10, "DRAGRACE");

	// TODO replace these counters with pretty dragrace progress animation/graphics.

	/*
	sprintf("%d", dragrace_counter);
	font_render_string_center (&font_supercar9, 64, 20, sprintf_buffer);
	*/

	sprintf("%s %d", car_names[player_car], player_car_position);
	font_render_string_center (&font_var5, 64, 20, sprintf_buffer);
	sprintf("%s %d", car_names[computer_car], computer_car_position);
	font_render_string_center (&font_var5, 64, 29, sprintf_buffer);

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

	dragracedbprintf ("dragrace_loser_anim: start\n");
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
	dragracedbprintf ("dragrace_loser_anim: exit\n");
}

void dragrace_winner_anim(void) {

	dragracedbprintf ("dragrace_winner_anim: start\n");
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
	dragracedbprintf ("dragrace_winner_anim: exit\n");
}

void dragrace_advance_car(U8 *car_position_ptr, enum car_list car, enum car_list other_car, U8 base) {
	*car_position_ptr = *car_position_ptr + base;

	S8 advantage = car - other_car;
	if (advantage > 0) {
		*car_position_ptr = *car_position_ptr + advantage;
	}

	if (*car_position_ptr > 100) {
		*car_position_ptr = 100;
	}
}

void dragrace_advance_player_car( void ) {
	dragrace_advance_car(&player_car_position, player_car, computer_car, 1);
}

void dragrace_select_cars( void ) {
	computer_car = total_dragraces_won;
	if (computer_car >= CARS_MAX) {
		computer_car = CARS_MAX;
	}
	player_car = cars_collected;
	if (player_car >= CARS_MAX) {
		player_car = CARS_MAX;
	}
}

void dragrace_deff (void) {
	dragracedbprintf ("dragrace_deff: start\n");

	dragracedbprintf ("dragraces won: %d\n", total_dragraces_won);
	// TODO wait for cars to return if racetrack is working while resetting ball search timer

	dragrace_select_cars();
	dragrace_start_anim();
	racetrack_race();

	dragracedbprintf ("dragraces won (before): %d\n", total_dragraces_won);

	player_car_position = 0;
	computer_car_position = 0;
	button_expectation = EXPECT_LEFT;

	while (1) {
#ifdef CONFIG_DEBUG_DRAGRACE
		dbprintf ("dragrace status:\n");
		dbprintf ("counter: %d\n", dragrace_counter);
		dbprintf ("computer_car_position: %d\n", computer_car_position);
		dbprintf ("player_car_position: %d\n", player_car_position);
		dbprintf ("button_expectation: %d\n", button_expectation);
#endif

		if (player_car_position == 100) {
			break;
		}
		if (computer_car_position == 100) {
			break;
		}
		dragrace_counter++;

		// FIXME calling this causes a random crash!
		//ball_search_timer_reset();

		dragrace_advance_car(&computer_car_position, computer_car, player_car, 1 + (random_scaled(5) < 2)); // (0 to 4) is less than 2?  == 2 in 5 chance

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
		total_dragraces_won++;
		dragrace_winner_anim();
	} else {
		dragrace_loser_anim();
	}

	lamp_tristate_off(LM_LEFT_TREE_GREEN);
	lamp_tristate_off(LM_RIGHT_TREE_GREEN);
	lamp_tristate_off(LM_LEFT_TREE_RED);
	lamp_tristate_off(LM_RIGHT_TREE_RED);


	dragracedbprintf ("dragraces won (after): %d\n", total_dragraces_won);

	dragracedbprintf ("dragrace_deff: exit\n");
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
	if (button_expectation != EXPECT_LEFT) {
		return;
	}
	button_expectation = EXPECT_RIGHT;
	dragrace_advance_player_car();
}

CALLSET_ENTRY (dragrace, sw_right_button) {
	if (!global_flag_test (GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return;
	}
	if (button_expectation != EXPECT_RIGHT) {
		return;
	}
	button_expectation = EXPECT_LEFT;
	dragrace_advance_player_car();
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
	global_flag_off(GLOBAL_FLAG_DIVERTER_ENABLED);
}

void award_lite_dragrace( void ) {
	// TODO sound effect and deff
	dragrace_enable();
}

void dragrace_enable( void ) {
	global_flag_on(GLOBAL_FLAG_DRAGRACE_ENABLED);
}

CALLSET_ENTRY(dragrace, dev_route_66_popper_enter) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}
	// kill the shot/arrow lights
	dragrace_disable();

	dragrace_start(GID_DRAGRACE);
}

CALLSET_ENTRY (dragrace, device_update) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}

	if (multi_ball_play()) {
		global_flag_off(GLOBAL_FLAG_DIVERTER_ENABLED);
	} else {
		global_flag_on(GLOBAL_FLAG_DIVERTER_ENABLED);
	}
}

CALLSET_ENTRY (dragrace, lamp_update) {
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		return;
	}

	if (multi_ball_play()) {
		lamp_tristate_off(LM_RACE_TODAY);
		lamp_tristate_off(LM_ROUTE_66_ARROW);
	} else {
		lamp_tristate_flash(LM_RACE_TODAY);
		lamp_tristate_flash(LM_ROUTE_66_ARROW);
	}
}

CALLSET_ENTRY(dragrace, vmode_dragrace_won) {
	if (!was_dragrace_started_by(GID_DRAGRACE)) {
		return;
	}

	score(SC_25M);

	// TODO award bonus for dragraces won (per ball?)
}

CALLSET_ENTRY(dragrace, vmode_dragrace_lost) {
}

