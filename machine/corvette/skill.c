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

#include <freewpc.h>
#include <corvette/racetrack.h>
/*
 * TODO implement race-for-pinks skill-shot mode
 */

//
// DRAGRACE SKILLSHOT
//
// The aim is to hit the right ramp (while diverter is open), hold the ball in the route 66 popper
// then start a video mode where the player bashes alternate flipper buttons to get the blue car to outrun the red
// car, ala track-and-field.
// Each drag-race one should increase the difficulty of the next one
// The more cars you have collected, the easier it should be to win a drag race (as you'd be using your best car right!)
// If the player wins a drag race award them something suitable (for now, just points).

U8 dragrace_counter_max;
U8 dragrace_counter;

U8 player_car_position;
U8 computer_car_position;

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
	sample_start (SND_RACE_STARTER_01, SL_1S);
	task_sleep_sec(1);

	// set
	lamp_tristate_on(LM_TREE_BOTTOM_YELLOW);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "SET");
	dmd_show_low ();
	sample_start (SND_RACE_STARTER_01, SL_1S);
	task_sleep_sec(1);

	// go
	lamp_tristate_flash(LM_LEFT_TREE_GREEN);
	lamp_tristate_flash(LM_RIGHT_TREE_GREEN);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_supercar9, 64, 16, "GO");
	dmd_show_low ();
	sample_start (SND_RACE_STARTER_02, SL_1S);
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
	font_render_string_center (&font_supercar9, 64, 16, "LOOSER");
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

	// start with a 10 second race and shorten by one second for each dragrace won.
	dragrace_counter_max = (DRAGRACE_TICKS_PER_SECOND * 10) - (dragraces_won * DRAGRACE_TICKS_PER_SECOND);
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
		score(SC_50M);
		// TODO award car or something
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
	dragrace_stop();
	task_exit();
}

void dragrace_start( void ) {
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

void skillshot_dragrace_disable( void ) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return;
	}

	lamp_tristate_off(LM_RACE_TODAY);
	lamp_tristate_off(LM_ROUTE_66_ARROW);

	task_kill_gid(GID_SKILLSHOT_DRAGRACE_TIMER);
	global_flag_off(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED);
	flag_off(FLAG_DIVERTER_OPENED);
}

void skillshot_dragrace_timer( void ) {
	task_sleep_sec(10);
	skillshot_dragrace_disable();
	task_exit();
}

void skillshot_dragrace_enable( void ) {
	// kill any existing task, just to make sure
	task_kill_gid(GID_SKILLSHOT_DRAGRACE_TIMER);
	task_create_gid1(GID_SKILLSHOT_DRAGRACE_TIMER, skillshot_dragrace_timer);
	global_flag_on(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED);
	flag_on(FLAG_DIVERTER_OPENED);
}

CALLSET_ENTRY(skillshot_dragrace, dev_route_66_popper_enter) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return;
	}
	// kill the skillshot timer and shot/arrow lights
	skillshot_dragrace_disable();

	dragrace_start();
}

CALLSET_BOOL_ENTRY(skillshot_dragrace, dev_route_66_popper_kick_request) {
	// XXX should work without this check
	/*
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return TRUE;
	}
	*/

	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_IN_PROGRESS)) {
		return TRUE;
	}

	 // hold the kickout for a bit.
	return FALSE;
}

CALLSET_ENTRY (skillshot_dragrace, lamp_update) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return;
	}

	lamp_tristate_flash(LM_RACE_TODAY);
	lamp_tristate_flash(LM_ROUTE_66_ARROW);
}


//
// SKIDPAD SKILLSHOT
//

void skillshot_skidpad_disable( void ) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_SKIDPAD_ENABLED)) {
		return;
	}

	lamp_tristate_off (LM_SKID_PAD_ARROW);
	lamp_tristate_off (LM_SKID_PAD_JACKPOT);

	task_kill_gid (GID_SKILLSHOT_SKIDPAD_TIMER);
	global_flag_off(GLOBAL_FLAG_SKILLSHOT_SKIDPAD_ENABLED);
}

void skillshot_skidpad_timer( void ) {
	task_sleep_sec (10);
	skillshot_skidpad_disable();
	task_exit ();
}

void skillshot_skidpad_enable( void ) {
	// kill any existing task, just to make sure
	task_kill_gid (GID_SKILLSHOT_SKIDPAD_TIMER);
	task_create_gid1 (GID_SKILLSHOT_SKIDPAD_TIMER, skillshot_skidpad_timer);
	global_flag_on(GLOBAL_FLAG_SKILLSHOT_SKIDPAD_ENABLED);
}


CALLSET_ENTRY(skillshot_skidpad, skid_pad_shot) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_SKIDPAD_ENABLED)) {
		return;
	}
	skillshot_skidpad_disable();
	score (SC_20M);
}

CALLSET_ENTRY (skillshot_skidpad, lamp_update)
{
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_SKIDPAD_ENABLED)) {
		return;
	}

	lamp_tristate_flash (LM_SKID_PAD_ARROW);
	lamp_tristate_flash (LM_SKID_PAD_JACKPOT);
}
//
// ROLLOVER SKILLSHOT
//

U8 current_rollover; // there are 3

void skillshot_rollover_disable( void ) {
	if (!global_flag_test (GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED)) {
		return;
	}

	global_flag_off ( GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED );

	lamp_tristate_off (LM_LEFT_ROLLOVER);
	lamp_tristate_off (LM_MIDDLE_ROLLOVER);
	lamp_tristate_off (LM_RIGHT_ROLLOVER);

	flag_on (FLAG_LOOP_GATE_OPENED);

	task_kill_gid (GID_SKILLSHOT_ROLLOVER_TIMER);
}

void skillshot_rollover_timer( void ) {
	task_sleep_sec (10);
	skillshot_rollover_disable();
	task_exit ();
}

void skillshot_rollover_enable( void ) {
	current_rollover = SW_MIDDLE_ROLLOVER;
	global_flag_on(GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED);
	flag_off (FLAG_LOOP_GATE_OPENED);

	// kill any existing task, just to make sure
	task_kill_gid (GID_SKILLSHOT_ROLLOVER_TIMER);
	task_create_gid1 (GID_SKILLSHOT_ROLLOVER_TIMER, skillshot_rollover_timer);
}

void award_rollover_skillshot(U8 rollover_switch) {
	if (!global_flag_test (GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED)) {
		return;
	}

	if (rollover_switch == current_rollover) {
		score (SC_5M);
		// TODO display skillshot award.
	}

	skillshot_rollover_disable();
}

CALLSET_ENTRY (skillshot_rollover, lamp_update)
{
	if (!global_flag_test (GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED)) {
		return;
	}

	lamp_flash_if (LM_LEFT_ROLLOVER, current_rollover == SW_LEFT_ROLLOVER);
	lamp_flash_if (LM_MIDDLE_ROLLOVER, current_rollover == SW_MIDDLE_ROLLOVER);
	lamp_flash_if (LM_RIGHT_ROLLOVER, current_rollover == SW_RIGHT_ROLLOVER);
}

CALLSET_ENTRY (skillshot_rollover, sw_left_rollover)
{
	award_rollover_skillshot(SW_LEFT_ROLLOVER);
}

CALLSET_ENTRY (skillshot_rollover, sw_middle_rollover)
{
	award_rollover_skillshot(SW_MIDDLE_ROLLOVER);
}

CALLSET_ENTRY (skillshot_rollover, sw_right_rollover)
{
	award_rollover_skillshot(SW_RIGHT_ROLLOVER);
}

CALLSET_ENTRY (skillshot_rollover, sw_left_button) {
	if (!global_flag_test (GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED)) {
		return;
	}

	switch (current_rollover) {
		case SW_LEFT_ROLLOVER:
			current_rollover = SW_RIGHT_ROLLOVER;
		break;

		case SW_MIDDLE_ROLLOVER:
			current_rollover = SW_LEFT_ROLLOVER;
		break;

		case SW_RIGHT_ROLLOVER:
			current_rollover = SW_MIDDLE_ROLLOVER;
		break;

	}
	callset_invoke( lamp_update );
}

CALLSET_ENTRY (skillshot_rollover, sw_right_button) {
	if (!global_flag_test (GLOBAL_FLAG_SKILLSHOT_ROLLOVER_ENABLED)) {
		return;
	}

	switch (current_rollover) {
		case SW_LEFT_ROLLOVER:
			current_rollover = SW_MIDDLE_ROLLOVER;
		break;

		case SW_MIDDLE_ROLLOVER:
			current_rollover = SW_RIGHT_ROLLOVER;
		break;

		case SW_RIGHT_ROLLOVER:
			current_rollover = SW_LEFT_ROLLOVER;
		break;
	}
	callset_invoke( lamp_update );
}

//
// SKILL MENU
//

U8 skill_menu_draw_count;
enum skill_menu_selections {
	SKILL_MIN = 0,
	SKILL_ROLLOVER = SKILL_MIN,
	SKILL_SKIDPAD,
	SKILL_DRAGRACE,
	SKILL_MAX = SKILL_DRAGRACE
};
enum skill_menu_selections skill_menu_selection;

char *skill_menu_text[4][3] = {
		// main set
		{"SELECT", "SKILL", "SHOT"},
		// set per mode
		{"SHOOT", "FLASHING", "ROLLOVER"},
		{"SHOOT", "SKID", "PAD"},
		{"SHOOT", "RIGHT", "RAMP"},
};

char **current_skill_menu_text;

// 4 = refreshes per second (see TIME_250MS in skill_menu_deff below)
// 5 = change every 5 seconds
#define SKILL_SHOW_SHOT_INFO (4 * 5)

void skill_menu_draw(void) {
	dbprintf ("skill_menu: drawing menu, selection:%d\n", skill_menu_selection);
	dmd_alloc_low_clean ();

	// every 5 seconds toggle left hand side of the menu screen between instructions for the menu and instructions for the shot
	skill_menu_draw_count++;
	// 2 = alternate between 2 things
	if (skill_menu_draw_count % (SKILL_SHOW_SHOT_INFO * 2) > SKILL_SHOW_SHOT_INFO) {
		current_skill_menu_text = (char **)&skill_menu_text[skill_menu_selection + 1];
		font_render_string_center (&font_var5, 64, 29, "LAUNCH BALL TO START");
	} else {
		current_skill_menu_text = skill_menu_text[0];
		font_render_string_center (&font_var5, 64, 29, "USE FLIPPERS TO SELECT");
	}

	dmd_draw_horiz_line ((U16 *)dmd_low_buffer, 25);

	//dmd_draw_border (dmd_low_buffer);
	font_render_string_center (&font_renew8, 32, 3, current_skill_menu_text[0]);
	font_render_string_center (&font_renew8, 32, 2 + 8 + 1, current_skill_menu_text[1]);
	font_render_string_center (&font_renew8, 32, 1 + 8 + 8 + 1 + 1, current_skill_menu_text[2]);

	// 4 = y offset, 5 = height, 3 = space between lines
	// bigger gap between lines, so we can draw box around them
	// TODO draw boxes round each item
	font_render_string_center (&font_var5, 96, 4, "ROLLOVER");
	font_render_string_center (&font_var5, 96, 12, "SKIDPAD");
	font_render_string_center (&font_var5, 96, 4 + 5 + 5 + 3 + 3, "DRAGRACE");

	// TODO flash box around selected item
	switch (skill_menu_selection) {
		case SKILL_ROLLOVER:
			font_render_string_center (&font_var5, 70, 4, ">");
			font_render_string_center (&font_var5, 122, 4, "<");
		break;
		case SKILL_SKIDPAD:
			font_render_string_center (&font_var5, 70, 12, ">");
			font_render_string_center (&font_var5, 122, 12, "<");
		break;
		case SKILL_DRAGRACE:
			font_render_string_center (&font_var5, 70, 4 + 5 + 5 + 3 + 3, ">");
			font_render_string_center (&font_var5, 122, 4 + 5 + 5 + 3 + 3, "<");
		break;
	}


	dmd_show_low ();
}

void skill_menu_deff (void)
{
	dbprintf ("skill_menu_deff, enabled: %d\n", global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED));
	do {
		skill_menu_draw();
		task_sleep(TIME_250MS);
	} while (global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED));
	dbprintf ("skill_menu_deff: exit, selection:%d\n", skill_menu_selection);
	deff_exit ();
}

void skill_menu_start(void) {
	skill_menu_draw_count = 0;
	global_flag_on(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED);
	skill_menu_selection = SKILL_ROLLOVER;
	dbprintf ("skill_menu_start\n");
	deff_start (DEFF_SKILL_MENU);
}

void skill_menu_select(void) {
	global_flag_off(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED);
	dbprintf ("skill_menu_select: selection:%d\n", skill_menu_selection);
	switch(skill_menu_selection) {
		case SKILL_ROLLOVER:
			skillshot_rollover_enable();
		break;
		case SKILL_SKIDPAD:
			skillshot_skidpad_enable();
		break;
		case SKILL_DRAGRACE:
			skillshot_dragrace_enable();
		break;
		default:
			// TODO implement remaining skill-shot modes
		break;
	}
}

/**
 * disables skillshot menu and skillshots,
 * should kill all skillshot timers and also reset any flashing lamps.
 */
void skillshot_disable(void) {
	skillshot_rollover_disable();
	skillshot_skidpad_disable();
	global_flag_off(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED);
}

CALLSET_ENTRY (skill_menu, display_update) {
	// the volume up/down, status reports, etc will cancel the menu, show it again if we should.
	if (global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED) && deff_get_active() != DEFF_SKILL_MENU) {
		deff_start (DEFF_SKILL_MENU);
	}
}

CALLSET_ENTRY (skill_menu, sw_left_button) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED)) {
		return;
	}
	if (skill_menu_selection == SKILL_MIN) {
		skill_menu_selection = SKILL_MAX;
	} else {
		skill_menu_selection--;
	}
	skill_menu_draw_count = SKILL_SHOW_SHOT_INFO;
	dbprintf ("skill_menu: left, selection: %d\n", skill_menu_selection);
}

CALLSET_ENTRY (skill_menu, sw_right_button) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED)) {
		return;
	}
	if (skill_menu_selection == SKILL_MAX) {
		skill_menu_selection = SKILL_MIN;
	} else {
		skill_menu_selection++;
	}
	skill_menu_draw_count = SKILL_SHOW_SHOT_INFO;
	dbprintf ("skill_menu: right, selection: %d\n", skill_menu_selection);
}


CALLSET_ENTRY (skill_menu, sw_shooter) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED)) {
		return;
	}

	if (!switch_poll_logical (SW_SHOOTER)) {
		dbprintf ("skill_menu: sw_shooter\n");
		skill_menu_select();
	}
}

CALLSET_ENTRY (skill_menu, any_pf_switch) {
	dbprintf ("skill_menu: any_pf_switch\n");
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_MENU_ENABLED)) {
		return;
	}
	// wait for the ball to be fired off the plunger switch, which is marked as a playfield switch so
	// that ball-search does not start when a ball is resting on it.
	/*
	if (switch_poll_logical (SW_PLUNGER)) {
		return;
	}
	*/

	skill_menu_select();
}

CALLSET_ENTRY (skill, start_ball, shoot_again) {
	dbprintf ("skill_menu: start_ball/shoot_again\n");
	skill_menu_start();
}

CALLSET_ENTRY (skill, end_ball, stop_game) {
	dbprintf ("skill_menu: end_ball/stop_game\n");
	skillshot_disable();
}
