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

#ifdef SKILL_ENABLED
#include <freewpc.h>
#include <corvette/racetrack.h>
#include <corvette/vmode_dragrace.h>
/*
 * TODO implement race-for-pinks skill-shot mode
 */

//
// DRAGRACE SKILLSHOT
//
// The aim is to hit the right ramp (while diverter is open), hold the ball in the route 66 popper
// then start a drag-race video mode


CALLSET_ENTRY(skillshot_dragrace, vmode_dragrace_won) {
	if (!was_dragrace_started_by(GID_SKILLSHOT_DRAGRACE)) {
		return;
	}
	score(SC_50M);
}

void skillshot_dragrace_disable( void ) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return;
	}

	task_kill_gid(GID_SKILLSHOT_DRAGRACE_TIMER);
	global_flag_off(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED);
	dragrace_disable();
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
	dragrace_enable();
}

CALLSET_ENTRY(skillshot_dragrace, dev_route_66_popper_enter) {
	if (!global_flag_test(GLOBAL_FLAG_SKILLSHOT_DRAGRACE_ENABLED)) {
		return;
	}
	// kill the skillshot timer
	skillshot_dragrace_disable();

	dragrace_start(GID_SKILLSHOT_DRAGRACE);
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

	global_flag_on (GLOBAL_FLAG_LOOP_GATE_OPENED);

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
	global_flag_off (GLOBAL_FLAG_LOOP_GATE_OPENED);

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

#endif
