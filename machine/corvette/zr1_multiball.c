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
 * @TODO sonny_jim says use kick_request instead of dev_enter.
 *
 * @TODO It's possible to light the lock by doing a right orbit twice very quickly
 *       use event following instead of timed task or just adjust the timer value?
 *
 * @TODO find out why task sleep between ball unlocks doesn't work.
 *       The player needs enough time after the first ball it launched to hit
 *       the inner loop and attempt a shot on the skid pad.
 *       currently the second ball is launched too quickly and gets it the way
 *
 * @TODO use ZR1 flasher so the player knows when and where the balls are going to come from.
 *
 * @TODO reset all other players' locked_ball count to 0 at multiball start
 *       I'ts not fair on other players, but no way round it as balls are ejected from ZR1 lock...
 */

#include <freewpc.h>
#include <corvette/zr1.h>
#include <zr1_low_rev_gate.h>
#include <button_expectation.h>

__local__ U8 lock_count;

score_t zr1_mb_jackpot_value;

void zr1_ball_locked_deff (void)
{
	music_effect_start(SND_AWARD_04, SL_3S);
	dmd_alloc_low_clean ();
	dmd_draw_border (dmd_low_buffer);
	sprintf ("BALL %d LOCKED", lock_count);
	font_render_string_center (&font_fixed6, 64, 16, sprintf_buffer);
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

void zr1_mb_lit_deff (void)
{
	speech_start(SPCH_HEAD_FOR_THE_LT5, SL_3S);
	dmd_alloc_low_clean ();
	dmd_draw_border (dmd_low_buffer);
	font_render_string_center (&font_supercar9, 64, 9, "SHOOT ENGINE");
	font_render_string_center (&font_supercar9, 64, 21, "FOR ZR1 MULTIBALL");
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

void zr1_mb_start_deff (void)
{
	sprintf ("ZR1 MULTIBALL");
	flash_and_exit_deff (30, TIME_100MS);
}

void zr1_mb_running_deff (void)
{
	for (;;)
	{
		score_update_start ();
		dmd_alloc_pair ();
		dmd_clean_page_low ();
		font_render_string_center (&font_mono5, 64, 5, "MULTIBALL");
		sprintf_current_score ();
		font_render_string_center (&font_fixed6, 64, 16, sprintf_buffer);
		dmd_copy_low_to_high ();

		font_render_string_center (&font_var5, 64, 27, "SHOOT JACKPOTS");

		dmd_show_low ();
		do {
			task_sleep (TIME_100MS);
			dmd_show_other ();
			task_sleep (TIME_100MS);
			dmd_show_other ();
		} while (!score_update_required ());
	}
}


void zr1_mb_trq_jp_deff (void)
{
	sprintf ("TORQUE JACKPOT");
	flash_and_exit_deff (20, TIME_100MS);
}

void zr1_mb_hp_jp_deff (void)
{
	sprintf ("H.P. JACKPOT");
	flash_and_exit_deff (20, TIME_100MS);
}

void zr1_mb_reset (void)
{
	lock_count = 0;
	flag_on (FLAG_ZR1_MULTIBALL_LITE_LOCK_LIT);
	flag_off (FLAG_ZR1_MULTIBALL_LOCK_LIT);
	global_flag_off (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING);
	global_flag_off (GLOBAL_FLAG_TORQUE_JACKPOT_LIT);
	global_flag_off (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT);
}

bool zr1_mb_can_award_lite_lock(void)
{
	if (!flag_test (FLAG_ZR1_MULTIBALL_LITE_LOCK_LIT)) {
		return FALSE;
	}
	return TRUE;
}

void zr1_mb_award_lite_lock (void)
{
	if (!zr1_mb_can_award_lite_lock()) {
		return;
	}

	flag_on (FLAG_ZR1_MULTIBALL_LOCK_LIT);
	flag_off (FLAG_ZR1_MULTIBALL_LITE_LOCK_LIT);

	deff_start (DEFF_ZR1_MB_LIT);
}

void zr1_mb_light_horsepower_jackpot (void)
{
	global_flag_on (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT);
}

void zr1_mb_light_torque_jackpot (void)
{
	global_flag_on (GLOBAL_FLAG_TORQUE_JACKPOT_LIT);
}

void zr1_mb_start_task( void )
{
	flag_off (FLAG_ZR1_MULTIBALL_LOCK_LIT);
	global_flag_off (GLOBAL_FLAG_DIVERTER_ENABLED); // FIXME what about other modes that may have set this?
	callset_invoke(device_update);

	deff_start (DEFF_ZR1_MB_START);
	award_kickback();

	global_flag_on (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING);
	zr1_mb_light_torque_jackpot ();
	zr1_mb_light_horsepower_jackpot ();

	// start unlocking balls

	ballsave_disable();

	// FIXME 'Ball Serve' section of manual says preferred way to start MB is to use set_ball_count() but this just causes a crash, we use device_unlock_ball() instead.
	/*set_ball_count(3);*/

	device_unlock_ball (device_entry (DEVNO_ZR1_POPPER));
	device_unlock_ball (device_entry (DEVNO_ZR1_POPPER));
	// third ball kicked out by returning from callset zr1_multiball dev_zr1_popper_enter
	task_exit ();
}

void zr1_mb_start (void)
{
	task_create_gid1 (GID_ZR1_MB_START_TASK, zr1_mb_start_task);
}

static void jackpot_check (void) {
	if (!global_flag_test(GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return;
	}

	if (global_flag_test (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT) || global_flag_test (GLOBAL_FLAG_TORQUE_JACKPOT_LIT)) {
		return; // jackpots still lit - player must collect both to relight jackpots
	}

	zr1_mb_light_torque_jackpot();
	zr1_mb_light_horsepower_jackpot();


	// TODO see if both hit very quickly, if so award super jackpot - maybe use a combo?
}

static void zr1_mb_award_horsepower_jackpot (void)
{
	if (!global_flag_test (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT)) {
		return;
	}
	global_flag_off (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT);

	sound_start (ST_SPEECH, SPCH_HORSEPOWER_JACKPOT, SL_2S, PRI_JACKPOT);
	score (SC_25M);
	/* TODO increase horsepower jackpot value */

	deff_start (DEFF_ZR1_MB_HP_JP);

	jackpot_check();
}


static void zr1_mb_award_torque_jackpot (void)
{
	if (!global_flag_test (GLOBAL_FLAG_TORQUE_JACKPOT_LIT)) {
		return;
	}
	global_flag_off (GLOBAL_FLAG_TORQUE_JACKPOT_LIT);

	sound_start (ST_SPEECH, SPCH_TORQUE_JACKPOT, SL_2S, PRI_JACKPOT);
	score (SC_25M);
	/* TODO increase torque jackpot value */

	deff_start (DEFF_ZR1_MB_TRQ_JP);

	jackpot_check();
}

static void zr1_mb_award_lock ( void )
{
	if (!flag_test (FLAG_ZR1_MULTIBALL_LOCK_LIT)) {
		return;
	}

	lock_count++;

	if (lock_count == 3) {
		zr1_mb_start();
		return;
	}

	sound_start (ST_SAMPLE, SND_DITTY_07, SL_4S, PRI_GAME_QUICK5);
	deff_start (DEFF_ZR1_BALL_LOCKED);

	U8 unlock_from_trough = (device_recount (device_entry (DEVNO_ZR1_POPPER)) >= lock_count);

	if (unlock_from_trough) {
		// locking a ball in the engine causes a new ball to be placed in the trough
		device_lock_ball (device_entry (DEVNO_ZR1_POPPER));
	}
}

CALLSET_ENTRY (zr1_multiball, left_orbit_combo_shot)
{
	if (global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return;
	}

	if (flag_test (FLAG_ZR1_MULTIBALL_LITE_LOCK_LIT)) {
		zr1_mb_award_lite_lock();
	}
}

CALLSET_BOOL_ENTRY (zr1_multiball, dev_zr1_popper_kick_request)
{
	if (global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return TRUE;
	}

	if (!task_find_gid (GID_ZR1_MB_START_TASK)) {
		return TRUE;
	}

	 // hold the kickout for a bit.
	return FALSE;
}

CALLSET_ENTRY (zr1_multiball, dev_zr1_popper_kick_attempt) {

	if (global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING) && live_balls != 3) {
		// need enough time to hit inner orbit then skidpad before ejecting another ball
		task_sleep_sec (4);
	}
	// TODO use custom flasher timings
	flasher_pulse(FLASH_ZR1_RAMP);
	task_sleep(TIME_100MS);
	flasher_pulse(FLASH_ZR1_RAMP);
	task_sleep(TIME_100MS);
	flasher_pulse(FLASH_ZR1_RAMP);
	task_sleep(TIME_100MS);
	flasher_pulse(FLASH_ZR1_RAMP);
	task_sleep(TIME_100MS);
}

CALLSET_ENTRY (zr1_multiball, dev_zr1_popper_kick_success) {
	if (global_flag_test(GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return;
	}

	sound_start (ST_SAMPLE, SND_BEEP_BEEP, SL_2S, PRI_GAME_QUICK5);
}

CALLSET_ENTRY (zr1_multiball, dev_zr1_popper_enter) {
	zr1_mb_award_lock ();
}

CALLSET_ENTRY (zr1_multiball, sw_skid_pad_exit)
{
	if (!global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return;
	}
	zr1_mb_award_torque_jackpot ();
}

CALLSET_ENTRY (zr1_multiball, sw_inner_loop_entry)
{
	if (!global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING)) {
		return;
	}

	zr1_mb_award_horsepower_jackpot ();
}

CALLSET_ENTRY (zr1_multiball, lamp_update)
{
	lamp_flash_if (LM_LITE_LOCK, flag_test (FLAG_ZR1_MULTIBALL_LITE_LOCK_LIT));
	lamp_flash_if (LM_ZR1_RAMP_LOCK, flag_test (FLAG_ZR1_MULTIBALL_LOCK_LIT));
	lamp_flash_if (LM_INNER_LOOP_JACKPOT, global_flag_test (GLOBAL_FLAG_HORSEPOWER_JACKPOT_LIT));
	lamp_flash_if (LM_SKID_PAD_JACKPOT, global_flag_test (GLOBAL_FLAG_TORQUE_JACKPOT_LIT));
}

CALLSET_ENTRY (zr1_multiball, device_update)
{
// XXX remove this when gate testing is completed
#ifdef CONFIG_GATE_TESTS_ENABLED
	return;
#endif
	// TODO other modes will want to change (open) the up rev gate, perform suitable mode check here (e.g. if mode set then bail and let mode control it instead)

	// close the ZR1 upper 'rev' gate when zr1 multiball lock is lit
	if (flag_test (FLAG_ZR1_MULTIBALL_LOCK_LIT)) {
		global_flag_off (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED);
	} else {
		global_flag_on (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED);
	}
}

CALLSET_ENTRY (zr1_multiball, music_refresh)
{
	if (global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING))
	{
		music_request (MUS_MULTIBALL, PRI_GAME_MODE3);
	}
}

CALLSET_ENTRY (zr1_multiball, display_update)
{
	if (global_flag_test (GLOBAL_FLAG_ZR1_MULTIBALL_RUNNING))
		deff_start_bg (DEFF_ZR1_MB_RUNNING, PRI_GAME_MODE3);
}

CALLSET_ENTRY (zr1_multiball, start_player, single_ball_play)
{
	zr1_mb_reset ();
}

CALLSET_ENTRY (zr1_multiball, start_ball) {
	// update the locked_balls count in case another player started a multiball
	U8 zr1_ball_count = device_recount (device_entry (DEVNO_ZR1_POPPER));
	if (lock_count > zr1_ball_count) {
		lock_count = zr1_ball_count;
	}
}


//
// ZR1 Engine ball lock
//

#define ZR1_ENGINE_HOLD_TICKS_PER_SECOND 4
#define ZR1_ENGINE_HOLD_TIME_MAX 15 // (in seconds) any more than this and the zr1 lower rev gate solenoid gets really toasty

U16 zr1_hold_ball_time; // set this when the ball is to about to be held.  See sw_zr1_top_entry handlers.

static void zr1_hold_ball_in_engine_task ( void )
{
	U8 hold_timer;
	global_flag_on (GLOBAL_FLAG_BALL_HELD_IN_ENGINE);
	global_flag_on (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED);

	hold_timer = ZR1_ENGINE_HOLD_TIME_MAX * ZR1_ENGINE_HOLD_TICKS_PER_SECOND;
	while (global_flag_test (GLOBAL_FLAG_BALL_HELD_IN_ENGINE) && hold_timer-- != 0) {
		task_sleep (TIME_250MS);
	}

	global_flag_off (GLOBAL_FLAG_BALL_HELD_IN_ENGINE);
	global_flag_off (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED);

	task_exit ();
}

void zr1_hold_ball_in_engine ( void )
{
	task_recreate_gid (GID_ZR1_HOLD_BALL_IN_ENGINE, zr1_hold_ball_in_engine_task);
}

CALLSET_ENTRY (zr1_hold_ball, sw_zr1_bottom_entry, sw_zr1_top_entry, sw_zr1_exit, dev_zr1_popper_enter)
{
	if (!global_flag_test(GLOBAL_FLAG_BALL_HELD_IN_ENGINE)) {
		return;
	}

	if (sw_last_scheduled_time == zr1_hold_ball_time) {
		// we only just starting holding the ball, don't cancel on this switch transition, wait for the next one.
		return;
	}

	if (!task_find_gid(GID_ZR1_HOLD_BALL_IN_ENGINE)) {
		return;
	}

	sound_start (ST_SAMPLE, SND_SPARK_PLUG_01, SL_1S, PRI_MULTIBALL); // XXX
	global_flag_off(GLOBAL_FLAG_BALL_HELD_IN_ENGINE);
}

//
// ZR1 Rev Mode
//

#define ZR1_REV_MODE_TICKS_PER_SECOND 10
#define ZR1_REV_MODE_TIME 10

#define ZR1_RPM_MIN 0
#define ZR1_RPM_MAX 8
U8 zr1_rpm; // valid range = 0-8. (as there are 9 sound samples, and 9000 rpm is high enough :D
bool zr1_button_expection_matched;

extern __fastram__ U8 zr1_shake_speed; // XXX
void zr1_rev_mode_deff (void)
{
	zr1_rpm = ZR1_RPM_MAX / 2; // start in the middle of the range to keep the ball shaking at medium speed
	zr1_button_expection_matched = FALSE;
	U8 rev_timer = ZR1_REV_MODE_TIME * ZR1_REV_MODE_TICKS_PER_SECOND;
	while ((in_test || global_flag_test (GLOBAL_FLAG_BALL_HELD_IN_ENGINE)) && rev_timer != 0) {

		// ten times a second, this ignores the ZR1_REV_MODE_TICKS_PER_SECOND define
		if ((rev_timer & 1) == 0) {
			flasher_pulse(FLASH_ZR1_UNDERSIDE);
			if (zr1_rpm >= 4) {
				flasher_pulse(FLASH_ZR1_UNDERSIDE);
			}
			if (zr1_rpm >= 6) {
				flasher_pulse(FLASH_ZR1_UNDERSIDE);
			}
		}

		// twice a second
		if (rev_timer % 5 == 0) {
			// update RPM
			if (zr1_button_expection_matched) {
				zr1_button_expection_matched = FALSE;

				if (zr1_rpm < ZR1_RPM_MAX) {
					zr1_rpm++;
				}

				// TODO add to score here

			} else {
				if (zr1_rpm > ZR1_RPM_MIN) {
					zr1_rpm--;
				}
			}
			// play correct engine sample based on RPM
			sound_start (ST_SAMPLE, SND_ENGINE_NOISE_01 + zr1_rpm, SL_500MS, PRI_MULTIBALL);

			// shake engine based on RPM
			zr1_set_shake_speed(ZR1_SHAKE_SPEED_SLOWEST + (zr1_rpm / 2));

		}

		dmd_alloc_low_clean ();
		font_render_string_center (&font_fixed6, 96, 5, "FLIP");
		font_render_string_center (&font_fixed6, 96, 15, "TO");
		font_render_string_center (&font_fixed6, 96, 25, "REV");

		// TODO draw tacho

#ifdef DEBUG_ZR1_REV_MODE
		// Developer code to show variables
		sprintf("%d %d %d", rev_timer, button_expectation, zr1_shake_speed);
		font_render_string_center (&font_var5, 64, 29, sprintf_buffer);
#endif

		font_render_string_right (&font_supercar9, 45, 5, "RPM");
		sprintf("%d000", zr1_rpm + 1); // display 1000 to 9000
		font_render_string_right (&font_fixed10, 52, 14, sprintf_buffer);
		dmd_show_low ();

		task_sleep (TIME_100MS);
		rev_timer--;
	}
	sound_start (ST_SAMPLE, SND_END_RACE_01 + zr1_rpm, SL_500MS, PRI_MULTIBALL);
	deff_exit ();
}

void zr1_rev_mode_exit(void)
{
	task_kill_gid(GID_ZR1_REV_MODE_TASK);
	flipper_enable();
	zr1_center();
	music_enable();
}

void zr1_rev_mode_task(void)
{
	zr1_hold_ball_time = sw_last_scheduled_time;
	button_expectation = EXPECT_LEFT;
	music_disable();
	sound_start (ST_SPEECH, SPCH_FLIP_TO_REV, SL_2S, PRI_MULTIBALL);
	flipper_disable();
	task_kill_gid(GID_ZR1_SHAKE); // make sure nothing else is shaking the engine
	zr1_set_shake_speed(ZR1_SHAKE_SPEED_MEDIUM);
	zr1_shake();
	zr1_hold_ball_in_engine();
	deff_start_sync (DEFF_ZR1_REV_MODE);

	if (global_flag_test (GLOBAL_FLAG_BALL_HELD_IN_ENGINE)) {
		global_flag_off (GLOBAL_FLAG_BALL_HELD_IN_ENGINE);
	}

	zr1_rev_mode_exit();
	task_exit();
}

CALLSET_ENTRY (zr1_rev_mode, sw_zr1_bottom_entry, sw_zr1_exit)
{
	if (!task_find_gid(GID_ZR1_REV_MODE_TASK)) {
		return;
	}
	// ball fell out while in rev mode
	zr1_rev_mode_exit();
}

CALLSET_ENTRY (zr1_rev_mode, sw_zr1_top_entry)
{
	if (!flag_test (FLAG_ZR1_MULTIBALL_LOCK_LIT)) {
		return;
	}

	if (lock_count != 2) {
		sound_start (ST_SPEECH, SPCH_HOUAH, SL_2S, PRI_MULTIBALL);
		return;
	}

	if (task_find_gid(GID_ZR1_REV_MODE_TASK)) {
		// ball fell out while in rev mode
		zr1_rev_mode_exit();
		return;
	}
	// when 2 balls are locked, hold the 3rd ball in the engine for a few seconds
	// sometimes the ball escapes into the popper or back down the ramp, so we must watch for that

	task_recreate_gid(GID_ZR1_REV_MODE_TASK, zr1_rev_mode_task);
}

CALLSET_ENTRY (zr1_rev_mode, sw_left_button) {
	if (!task_find_gid(GID_ZR1_REV_MODE_TASK)) {
		return;
	}
	if (button_expectation != EXPECT_LEFT) {
		return;
	}
	button_expectation = EXPECT_RIGHT;
	zr1_button_expection_matched = TRUE;
}

CALLSET_ENTRY (zr1_rev_mode, sw_right_button) {
	if (!task_find_gid(GID_ZR1_REV_MODE_TASK)) {
		return;
	}
	if (button_expectation != EXPECT_RIGHT) {
		return;
	}
	button_expectation = EXPECT_LEFT;
	zr1_button_expection_matched = TRUE;
}


