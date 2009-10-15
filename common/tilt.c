/*
 * Copyright 2006, 2007, 2008, 2009 by Brian Dominy <brian@oddchange.com>
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
 * \file
 * \brief Common tilt functions
 *
 * This module handles all of the tilt switches, start game-specific
 * effects when they occur, and updates the core state machines
 * appropriately (in the case of a slam tilt, this will abort the
 * game).  It also provides the display and lamp effects that go along
 * with them.
 */

#include <freewpc.h>
#include <coin.h>

/** The number of tilt warnings that have been issued on this ball. */
U8 tilt_warnings;


/** Lamp effect function for a leff that turns all lights off.
 * Used by the system-defined tilt function. */
void no_lights_leff (void)
{
	for (;;)
		task_sleep_sec (5);
}


void tilt_warning_leff (void)
{
	task_sleep (TIME_500MS);
	leff_exit ();
}


/** The tilt display effect runs until explicitly cancelled. */
void tilt_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_cu17, 64, 13, "TILT");
	dmd_show_low ();
	for (;;)
		task_sleep_sec (10);
}


void tilt_warning_deff (void)
{
	dmd_alloc_pair_clean ();
	if (tilt_warnings % 2)
	{
		font_render_string_center (&font_fixed10, 64, 16, "DANGER");
	}
	else
	{
		font_render_string_center (&font_fixed10, 64, 7, "DANGER");
		font_render_string_center (&font_fixed10, 64, 23, "DANGER");
	}
	deff_swap_low_high (24, TIME_66MS);
	deff_exit ();
}


void slam_tilt_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_fixed10, 64, 13, "SLAM TILT");
	dmd_show_low ();
	task_sleep_sec (3);
	deff_exit ();
}


CALLSET_ENTRY (tilt, sw_tilt)
{
	extern U8 in_tilt;

	/* Ignore tilt switch activity while already in tilt state.
	 * But restart the timer that tells us that the tilt is still
	 * moving, so we can delay endball. */
	if (in_tilt)
	{
		free_timer_restart (TIM_IGNORE_TILT, TIME_2S);
		return;
	}

	/* IDEA : Disable tilt while a ball search is in progress? */

	else if (++tilt_warnings == system_config.tilt_warnings)
	{
		/* Warnings exceeded... tilt the current ball */
		sound_reset ();
		triac_disable (TRIAC_GI_MASK);
		deff_start (DEFF_TILT);
		leff_start (LEFF_TILT);
		free_timer_restart (TIM_IGNORE_TILT, TIME_2S);
		in_tilt = TRUE;
		task_remove_duration (TASK_DURATION_LIVE);
		task_duration_expire (TASK_DURATION_LIVE);
		flipper_disable ();
		set_valid_playfield ();
		audit_increment (&system_audits.tilts);
		audit_increment (&system_audits.plumb_bob_tilts);
		callset_invoke (tilt);
	}
	else
	{
		/* Give a warning this time */
		deff_start (DEFF_TILT_WARNING);
		leff_start (LEFF_TILT_WARNING);
		callset_invoke (tilt_warning);
	}
}


CALLSET_ENTRY (tilt, sw_slam_tilt)
{
	/* Ignore slam tilt switch entirely while coin door is open,
	and configured for tournament mode.  This is to avoid inadvertent slam tilts
	while dealing with problems. */
	if (system_config.tournament_mode && !switch_poll_logical (SW_COIN_DOOR_CLOSED))
		return;

	/* Ignore right after a coin door open/close */
	if (nonball_event_did_follow (sw_coin_door_closed, sw_slam_tilt))
		return;

	/* Kill the current game */
	stop_game ();

	/* Disable coins briefly, the whole point of the slam tilt */
	event_can_follow (sw_slam_tilt, sw_coin, TIME_5S);

	/* Start the slam tilt effect */
	callset_invoke (slam_tilt);
	deff_start (DEFF_SLAM_TILT);
	lamp_all_off ();

	/* Audit the event */
	audit_increment (&system_audits.slam_tilts);

	/* When slamtilt penalty adjustment is enabled, remove a credit. */
	if (price_config.slamtilt_penalty)
		remove_credit ();

	while (deff_get_active () == DEFF_SLAM_TILT)
		task_sleep (TIME_66MS);

	/* TODO: wait for slam switch to become stable, to avoid
	 * endless restarts */
	/* TODO : change to a warm reboot here */
	amode_start ();
}


CALLSET_ENTRY (tilt, start_ball)
{
	tilt_warnings = 0;
}

