/*
 * Copyright 2006, 2007, 2008, 2009, 2010 by Brian Dominy <brian@oddchange.com>
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
#include <status.h>

__local__ U8 mball_locks_lit;
__local__ U8 mball_locks_made;
__local__ U8 mballs_played;

/* Current jackpot, in 10M's */
U8 jackpot_level;
/* Used to pass the jackpot level to deff */
U8 jackpot_level_stored;
/* Used to restart if multiball ends without picking up a jackpot */
bool mball_jackpot_uncollected;
bool mball_restart_collected;
U8 mball_restart_timer;
U8 last_number_called;

void mball_restart_mode_init (void);
void mball_restart_mode_exit (void);

struct timed_mode_ops mball_restart_mode = {
	DEFAULT_MODE,
	.init = mball_restart_mode_init,
	.exit = mball_restart_mode_exit,
	.gid = GID_MBALL_RESTART_MODE,
	.music = MUS_FASTLOCK_COUNTDOWN,
	.deff_running = DEFF_MBALL_RESTART,
	.prio = PRI_MULTIBALL,
	.init_timer = 15,
	.timer = &mball_restart_timer,
	.grace_timer = 3,
	.pause = system_timer_pause,
};

extern U8 unlit_shot_count;
extern U8 live_balls;
extern U8 gumball_enable_count;
extern U8 autofire_request_count;
extern bool fastlock_running (void);
extern U8 lucky_bounces;
extern __machine__ void mpf_countdown_task (void);

static void mball_restart_countdown_task (void)
{
	do {
		if (last_number_called != mball_restart_timer)
		{
			switch (mball_restart_timer)
			{
				case 5:
					sound_send (SND_FIVE);
					break;
				case 4:
					sound_send (SND_FOUR);
					break;
				case 3:
					sound_send (SND_THREE);
					break;
				case 2:
					sound_send (SND_TWO);
					break;
				case 1:
					sound_send (SND_ONE);
					break;
			}
			last_number_called = mball_restart_timer;
			task_sleep (TIME_900MS);
		}
		task_sleep (TIME_100MS);
	}while (mball_restart_timer <= 5 && mball_restart_timer != 0
			&& !system_timer_pause ());
	task_exit ();
}


/* Rules to say whether we can start multiball */
bool multiball_ready (void)
{
	/* Don't allow during certain conditions */
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING) 
		|| global_flag_test (GLOBAL_FLAG_SSSMB_RUNNING)
		|| global_flag_test (GLOBAL_FLAG_CHAOSMB_RUNNING)
		|| multi_ball_play ()
		|| global_flag_test (GLOBAL_FLAG_POWERBALL_IN_PLAY))
		return FALSE;
	/* Require one locked ball first multiball, 2 locks after */
	else if ((mball_locks_made > 0) && (mballs_played == 0))
		return TRUE;
	else if ((mball_locks_made > 1) && (mballs_played > 0))
		return TRUE;
	else
		return FALSE;

}

void mball_restart_deff (void)
{
	for (;;)
	{
		dmd_alloc_low_clean ();
		font_render_string_center (&font_var5, 64, 16, "SHOOT LOCK TO RESTART");
		font_render_string_center (&font_fixed6, 64, 4, "MULTIBALL");
		sprintf ("%d", mball_restart_timer);
		font_render_string_center (&font_fixed6, 64, 25, sprintf_buffer);
		dmd_show_low ();
		task_sleep (TIME_200MS);
	}
}

void mball_restart_mode_init (void)
{
}

void mball_restart_mode_expire (void)
{
}

void mball_restart_mode_exit (void)
{
	task_kill_gid (GID_MBALL_RESTART_COUNTDOWN_TASK);
}

CALLSET_ENTRY (mball, display_update)
{
	timed_mode_display_update (&mball_restart_mode);
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING))
		deff_start_bg (DEFF_MB_RUNNING, 0);
}

CALLSET_ENTRY (mball, music_refresh)
{
	timed_mode_music_refresh (&mball_restart_mode);
	if (!in_game)
		return;
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING))
		music_request (MUS_MULTIBALL, PRI_GAME_MODE1 + 12);
	if (mball_restart_timer <= 5 
		&& !task_find_gid (GID_MBALL_RESTART_COUNTDOWN_TASK)
		&& !system_timer_pause ())
	{
		task_recreate_gid (GID_MBALL_RESTART_COUNTDOWN_TASK, mball_restart_countdown_task);
	}
}

void lock_lit_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_fixed10, 64, 16, "LOCK IS LIT");
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

void mb_lit_deff (void)
{
	dmd_alloc_low_clean ();
	sprintf ("BALL %d LOCKED", mball_locks_made);
	font_render_string_center (&font_fixed6, 64, 7, sprintf_buffer);
	if (multiball_ready ())
	{
		font_render_string_center (&font_mono5, 64, 20, "SHOOT LEFT RAMP");
		font_render_string_center (&font_mono5, 64, 26, "FOR MULTIBALL");
	}
	dmd_show_low ();
	task_sleep_sec (3);
	deff_exit ();
}

void mb_start_deff (void)
{
	sound_send (SND_DONT_TOUCH_THE_DOOR_AD_INF);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_fixed10, 64, 16, "MULTIBALL");
	dmd_show_low ();
	flash_and_exit_deff (50, TIME_100MS);
}

void jackpot_relit_deff (void)
{
	sound_send (0xFD);
	dmd_alloc_low_clean ();
	font_render_string_center (&font_fixed10, 64, 16, "JACKPOT RELIT");
	dmd_show_low ();
	flash_and_exit_deff (50, TIME_100MS);
}

void mb_jackpot_collected_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_term6, 64, 10, "MB JACKPOT");
	printf_millions (jackpot_level_stored * 10);
	font_render_string_center (&font_fixed6, 64, 21, sprintf_buffer);
	dmd_show_low ();
	sound_send (SND_SKILL_SHOT_CRASH_3);
	task_sleep_sec (1);
	deff_exit ();
}

void mb_running_deff (void)
{
	for (;;)
	{
		score_update_start ();
		dmd_alloc_pair ();
		dmd_clean_page_low ();
		sprintf_current_score ();
		font_render_string_center (&font_fixed6, 64, 16, sprintf_buffer);
		if (global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT))
		{
			sprintf("SHOOT PIANO FOR %dM", (jackpot_level * 10));
			font_render_string_center (&font_var5, 64, 27, sprintf_buffer);
		}
		else
		{
			font_render_string_center (&font_var5, 64, 27, "SHOOT LOCK TO RELIGHT");
		}
		dmd_copy_low_to_high ();
		font_render_string_center (&font_fixed6, 64, 4, "MULTIBALL");
		dmd_show_low ();
		do
		{
			task_sleep (TIME_133MS);
			dmd_show_other ();
			task_sleep (TIME_133MS);
			dmd_show_other ();
		} while (!score_update_required ());
	}
}

/* Check to see if we can light the lock/lock a ball */
bool can_lock_ball (void)
{	
	if ( mball_locks_lit > 0 
		&& mball_locks_made < 2
		&& !global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING) 
		&& !global_flag_test (GLOBAL_FLAG_BTTZ_RUNNING) 
		&& !global_flag_test (GLOBAL_FLAG_SSSMB_RUNNING) 
		&& !global_flag_test (GLOBAL_FLAG_CHAOSMB_RUNNING)
		&& !multi_ball_play ()
		&& !pb_in_lock ())
		return TRUE;
	else
		return FALSE;
}

bool can_light_lock (void)
{
	if (can_lock_ball ())
		return TRUE;
	else if (fastlock_running ())
		return TRUE;
	else if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING) && !global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT))
		return TRUE;
	else if (timed_mode_running_p (&mball_restart_mode))
		return TRUE;
	else
		return FALSE;
}

CALLSET_ENTRY (mball, lamp_update)
{
	/* Light the lock if it can be collected */
	if (can_light_lock ())
		lamp_tristate_flash (LM_LOCK_ARROW);
	else	
		lamp_tristate_off (LM_LOCK_ARROW);

	/* Flash the appropiate lamp when multiball is ready */
	if (multiball_ready ())
		lamp_tristate_flash (LM_MULTIBALL);
	else if (!global_flag_test (GLOBAL_FLAG_CHAOSMB_RUNNING))
		lamp_tristate_off (LM_MULTIBALL);
	
	if (multi_ball_play ())
	{
		/* Turn off during multiball */
		lamp_tristate_off (LM_LOCK1);
		lamp_tristate_off (LM_LOCK2);
		return;
	}
	/* Turn on and flash door lock lamps during game situations */
	if (mball_locks_made == 0 && mball_locks_lit == 0)
	{
		lamp_tristate_off (LM_LOCK1);
		lamp_tristate_off (LM_LOCK2);
	}
	else if (mball_locks_made == 0 && mball_locks_lit == 1)
	{
		lamp_tristate_flash (LM_LOCK1);
		lamp_tristate_off (LM_LOCK2);
	}
	else if (mball_locks_made == 0 && mball_locks_lit > 1)
	{
		lamp_tristate_flash (LM_LOCK1);
		lamp_tristate_flash (LM_LOCK2);
	}
	else if (mball_locks_made == 1 && mball_locks_lit == 0)
	{
		lamp_tristate_on (LM_LOCK1);
		lamp_tristate_off (LM_LOCK2);
	}
	else if (mball_locks_made == 1 && mball_locks_lit >= 1)
	{
		lamp_tristate_on (LM_LOCK1);
		lamp_tristate_flash (LM_LOCK2);
	}
	else if (mball_locks_made >= 2)
	{
		lamp_tristate_on (LM_LOCK1);
		lamp_tristate_on (LM_LOCK2);
	}
	
	/* Flash the Piano Jackpot lamp when MB Jackpot is lit */
	if (global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT)&& multi_ball_play ())
		lamp_tristate_flash (LM_PIANO_JACKPOT);
	else
	{
		lamp_tristate_off (LM_PIANO_JACKPOT);
	}
}

void mball_light_lock (void)
{
	if (mball_locks_lit  != 2 && mball_locks_made != 2)
	{
		sound_send (SND_GUMBALL_COMBO);
		deff_start (DEFF_LOCK_LIT);
	}
	bounded_increment (mball_locks_lit, 2);
}

/* Check to see if GUMBAL has been completed and light lock */
void mball_check_light_lock (void)
{
	if (lamp_test (LM_GUM) && lamp_test (LM_BALL))
	{
		mball_light_lock ();
		gumball_enable_count++;
	}
}

 /* How we want the balls released and in what order */
CALLSET_ENTRY (multiball, mball_start_3_ball)
{
	/* Don't start if another multiball is running */
	if (multi_ball_play () || live_balls == 3)
		return;
	/* Check lock and empty accordingly */
	switch (device_recount (device_entry (DEVNO_LOCK)))
	{	
		/* No balls in lock, fire 2 from trough */
		case 0:
			autofire_add_ball ();	
			autofire_add_ball ();	
			break;
		/* 1 ball in lock, fire 2 from trough 
		 *  1 ball may already be in autofire */
		case 1:
			autofire_add_ball ();	
		 	task_sleep_sec (4);
			task_sleep (TIME_500MS);
			device_unlock_ball (device_entry (DEVNO_LOCK));
			break;
		/* 2 balls in lock, fire 1 from trough */
		case 2:
			device_unlock_ball (device_entry (DEVNO_LOCK));
			task_sleep_sec (1);	
			device_unlock_ball (device_entry (DEVNO_LOCK));
			break;
	}
	/* This should add in an extra ball if the above wasn't enough */
	set_ball_count (3);
	
}

CALLSET_ENTRY (multiball, mball_start_2_ball)
{
	if (multi_ball_play () || live_balls > 1)
		return;
	/* Check lock and empty accordingly */
	switch (device_recount (device_entry (DEVNO_LOCK)))
	{	
		/* No balls in lock, fire 1 from trough */
		case 0:
			autofire_add_ball ();	
			break;
		/* 1/2 balls in lock, drop 1 */ 
		case 1:
		case 2:
			device_unlock_ball (device_entry (DEVNO_LOCK));
			break;
	}
	set_ball_count (2);
}

CALLSET_ENTRY (mball, mball_restart_stop)
{
	if (timed_mode_running_p (&mball_restart_mode))
		timed_mode_end (&mball_restart_mode);
}

CALLSET_ENTRY (mball, mball_start)
{
	if (!global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING))
	{
		magnet_reset ();
		callset_invoke (mball_restart_stop);
		unlit_shot_count = 0;
		global_flag_on (GLOBAL_FLAG_MULTIBALL_RUNNING);
		global_flag_on (GLOBAL_FLAG_MB_JACKPOT_LIT);
		kickout_lock (KLOCK_DEFF);
		deff_start (DEFF_MB_START);
		leff_start (LEFF_MB_RUNNING);
		effect_update_request ();
		/* Set the jackpot higher if two balls were locked */
		if (mball_locks_made > 1)
			jackpot_level = 2;
		else
			jackpot_level = 1;
		mball_locks_lit = 0;
		mball_locks_made = 0;
		mball_jackpot_uncollected = TRUE;
		mballs_played++;
		lamp_off (LM_GUM);
		lamp_off (LM_BALL);
	}
}

CALLSET_ENTRY (mball, mball_stop)
{
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING))
	{
		global_flag_off (GLOBAL_FLAG_MULTIBALL_RUNNING);
		global_flag_off (GLOBAL_FLAG_SUPER_MB_RUNNING);
		global_flag_off (GLOBAL_FLAG_MB_JACKPOT_LIT);
		deff_stop (DEFF_MB_START);
		deff_stop (DEFF_MB_RUNNING);
		deff_stop (DEFF_JACKPOT_RELIT);
		leff_stop (LEFF_MB_RUNNING);
		lamp_off (LM_GUM);
		lamp_off (LM_BALL);
		effect_update_request ();
		/* If a jackpot wasn't collected, offer a restart */
		if (mball_jackpot_uncollected && !mball_restart_collected)
			timed_mode_begin (&mball_restart_mode);
	}
}

/* Called from leftramp.c */
void mball_left_ramp_exit (void)
{
	if (multiball_ready ())
	{
		leff_start (LEFF_STROBE_DOWN);
		leff_start (LEFF_FLASH_GI);
		lamp_tristate_off (LM_MULTIBALL);
		callset_invoke (mball_start);
		callset_invoke (mball_start_3_ball);
	}
	else if (!lamp_test (LM_GUM) && !multi_ball_play ())
	{
		lamp_on (LM_GUM);
		mball_check_light_lock ();
	}
	event_can_follow (sw_left_ramp_exit, sw_shooter, TIME_3S);
}

/* TODO - on a missed left ramp exit switch, use the
 * autoplunger switch to start multiball instead */

CALLSET_ENTRY (mball, sw_right_ramp)
{
	if (multi_ball_play ());
	else if (!lamp_test (LM_BALL))
	{
		lamp_on (LM_BALL);
		mball_check_light_lock ();
	}
}

/* If for some reason the ball ends up in the shooter lane, start multiball */
CALLSET_ENTRY (mball, sw_shooter)
{
	if (event_did_follow (sw_left_ramp_exit, sw_shooter) && multiball_ready ())
	{
		callset_invoke (mball_start_3_ball);
		callset_invoke (mball_start);
	}
}

CALLSET_ENTRY (mball, sw_piano)
{
	if (global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT))
	{
		magnet_disable_catch (MAG_LEFT);
		global_flag_off (GLOBAL_FLAG_MB_JACKPOT_LIT);
		/* Add anoither 10M to the jackpot if three balls are out */
		if (live_balls == 3)
			bounded_increment (jackpot_level, 5);
		jackpot_level_stored = jackpot_level;
		deff_start (DEFF_JACKPOT);
		deff_start (DEFF_MB_JACKPOT_COLLECTED);
		mball_jackpot_uncollected = FALSE;
		/* Score it */
		score_multiple (SC_10M, jackpot_level);
		/* Increase the jackpot level */
		bounded_increment (jackpot_level, 5);
		timer_restart_free (GID_MB_JACKPOT_COLLECTED, TIME_3S);
	}
}

CALLSET_ENTRY (mball, powerball_jackpot)
{
	deff_start (DEFF_PB_JACKPOT);
	/* -1, as it probably has been incremented already */
	score_multiple (SC_10M, jackpot_level - 1);
	jackpot_level_stored = jackpot_level * 2;
}

CALLSET_ENTRY (mball, any_pf_switch)
{
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING))
	{
		score (SC_20K);
	}
}


CALLSET_ENTRY (mball, single_ball_play)
{
	callset_invoke (mball_stop);
}


CALLSET_ENTRY (mball, dev_lock_enter)
{
	/* Tell fastlock that the lock was entered */
	callset_invoke (fastlock_lock_entered);

	/* Collect multiball jackpot if lit */
	if ((global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING)) && !global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT))
	{
		global_flag_on (GLOBAL_FLAG_MB_JACKPOT_LIT);
		deff_start (DEFF_JACKPOT_RELIT);
	}
	
	/* Check to see if mball_restart is running */
	if (timed_mode_running_p (&mball_restart_mode))
	{
		task_kill_gid (GID_MPF_COUNTDOWN_TASK);
		sound_send (SND_CRASH);
		score (SC_5M);
		timed_mode_end (&mball_restart_mode);
		mball_restart_collected = TRUE;
		if (!multi_ball_play ())
		{
			callset_invoke (mball_start);
			callset_invoke (mball_start_3_ball);
		}
	}
	/* Lock check should pretty much always go last */
	else if (can_lock_ball ())
	{
		device_t *dev = device_entry (DEVNO_LOCK);

		/* Right loop -> Locked ball lucky bounce handler */
		if (event_did_follow (right_loop, locked_ball))
		{
			sound_send (SND_LUCKY);
			score (SC_5M);
			deff_start (DEFF_LUCKY_BOUNCE);
			bounded_increment (lucky_bounces, 99);
		}

		bounded_decrement (mball_locks_lit, 0);
		bounded_increment (mball_locks_made, 2);
		sound_send (SND_FAST_LOCK_STARTED);
		if (mball_locks_lit == 0)
		{
			lamp_off (LM_GUM);
			lamp_off (LM_BALL);
		}
		deff_start (DEFF_MB_LIT);
		unlit_shot_count = 0;

		/* Handle physical device lock.  If the lock is full (3 balls),
		then we can't keep this ball here.  Otherwise, we hold onto it,
		which will force another ball from the trough. */
		if (device_full_p (dev))
		{
			deff_start (DEFF_BALL_FROM_LOCK);
		}
		else
		{
			device_lock_ball (dev);
			enable_skill_shot ();
		}
	}
	else
		/* inform unlit.c that a shot was missed */
		award_unlit_shot (SW_LOCK_LOWER);
}

CALLSET_ENTRY (mball, end_ball)
{
	callset_invoke (mball_stop);
	timed_mode_end (&mball_restart_mode);
}

CALLSET_ENTRY (mball, start_ball)
{
	lamp_tristate_off (LM_MULTIBALL);
	lamp_off (LM_MULTIBALL);
	mball_restart_collected = FALSE;
}

CALLSET_ENTRY (mball, start_player)
{
	lamp_off (LM_GUM);
	lamp_off (LM_BALL);
	mball_locks_lit = 0;
	mball_locks_made = 0;
	mballs_played = 0;
}


CALLSET_ENTRY (mball, status_report)
{
	status_page_init ();
	sprintf ("%d LOCKS LIT", mball_locks_lit);
	font_render_string_center (&font_mono5, 64, 10, sprintf_buffer);
	sprintf ("%d BALLS LOCKED", mball_locks_made);
	font_render_string_center (&font_mono5, 64, 21, sprintf_buffer);
	status_page_complete ();
}

CALLSET_ENTRY (mball, left_ball_grabbed)
{
	if (global_flag_test (GLOBAL_FLAG_MULTIBALL_RUNNING) && global_flag_test (GLOBAL_FLAG_MB_JACKPOT_LIT))
	{
		deff_start (DEFF_SHOOT_JACKPOT);
	}
}

CALLSET_ENTRY (mball, ball_search)
{
}
