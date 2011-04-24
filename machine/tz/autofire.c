/*
 * Copyright 2006-2010 by Brian Dominy <brian@oddchange.com>
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
#include <shooter_div.h>

/** A count of the number of pending autolaunches. */
U8 autofire_request_count;

/** Delay time in seconds before opening the shooter divertor */
U8 shooter_div_delay_time;

/** Delay time in seconds before closing the shooter divertor */
U8 shooter_div_open_time;

/* Flag to say whether the autofire is busy firing a ball */
bool autofire_busy;
extern bool hold_balls_in_autofire;

void sw_autofire (void)
{
	/* TODO : balls landing here when not expected
	still need to be ejected.  See amode_start below.  This
	would also need to happen if the autofire monitor
	failed to kick properly; some retry logic is needed. */
	sol_stop (SOL_RAMP_DIVERTOR);
	shooter_div_stop ();
	score (SC_100);
}

CALLSET_ENTRY (autofire, sw_autofire1)
{
	sw_autofire ();
}


CALLSET_ENTRY (autofire, sw_autofire2)
{
	sw_autofire ();
}

/* Function to wait 6 seconds or until the autofire is detected as loaded */
static inline void autofire_ball_catch_wait (void)
{
	U8 timeout = 60; /* 6 seconds */
	while (!switch_poll_logical (SW_AUTOFIRE2) 
		&& --timeout != 0)
	{
		task_sleep (TIME_100MS);
	}
}
/** A task that manages the autolaunching of balls.
Upon entry, the autofire divertor solenoid is already pulsing
and a ball is being kicked from the trough. */
void autofire_monitor (void)
{
	/* Open the divertor to catch the ball.  Because it may be
	coming from either the trough or a ramp divert, the
	timings are variable. */
	if (shooter_div_delay_time)
		task_sleep_sec (shooter_div_delay_time);
	
	autofire_busy = TRUE;
	//if (autofire_full ()
	//	don't open to catch 
	shooter_div_start ();
	/* TODO - If the autofire switch trips during the 'open
	time', we can abort this delay early and go ahead and
	close the divertor.  This is safe because only one
	ball can appear here at a time. */
	//task_sleep_sec (shooter_div_open_time);
	autofire_ball_catch_wait ();	
	shooter_div_stop ();
	
	/* Wait a little longer for the ball to settle 
	 * and the divertor to close */
	task_sleep (TIME_500MS);
	
	/* If Right inlane -> Left ramp combo, start tnf mode */
	if (event_did_follow (left_ramp_exit, tnf) && single_ball_play ())
	{
		callset_invoke (tnf_start);
	}
	
	/* Wait until allowed to kickout */
	while (kickout_locks > 0)
		task_sleep (TIME_100MS);

	/* Open diverter again */
	shooter_div_start ();
	/* Wait for the diverter to fully open before firing */
	task_sleep_sec (1);	
	
	if (in_live_game && single_ball_play ())
	{
		sound_send (SND_EXPLOSION_1);
		leff_start (LEFF_STROBE_UP);
	}
	/* Say that the ball is heading into the right loop */
	timer_restart_free (GID_BALL_LAUNCH, TIME_3S);
	event_can_follow (autolaunch, right_loop, TIME_4S);
	/* Clear the magnet so we can fire a ball */
	magnet_disable_catch (MAG_RIGHT);
	/* Launch the ball */
	sol_request (SOL_AUTOFIRE);
	/* Wait for the ball to clear the divertor 
	 * before closing*/
	task_sleep (TIME_700MS);
	shooter_div_stop ();
	autofire_busy = FALSE;
	task_exit ();
}	

/** Called just before the trough kicks a ball when it ought to
go to the autofire lane rather than the manual plunger. */
void autofire_open_for_trough (void)
{
	/* Do not proceed if another ball is in the process of
	being autofired. */
	while (task_find_gid (GID_AUTOFIRE_HANDLER))
	{
		dbprintf ("Autofire waiting for previous.\n");
		/* TODO : these sleeps should be done in the spawned task,
		not in the interface */
		task_sleep_sec (1);
	}
	dbprintf ("Shooter divertor open to catch\n");
	shooter_div_delay_time = 0;
	shooter_div_open_time = 2;
	task_create_gid_while (GID_AUTOFIRE_HANDLER, autofire_monitor, TASK_DURATION_INF);
	task_sleep (TIME_500MS);
}


/** Request that a new ball be autolaunched into play. */
void autofire_add_ball (void)
{
	autofire_request_count++;
	
	if (!in_game || switch_poll_logical (SW_FAR_LEFT_TROUGH))
	{
		/* For special situations.  If not in game, the
		kick_attempt hook won't be called to open for trough.
		If far left trough is set, the kick will appear failed
		because the trough count won't change (even though a
		ball was successfully kicked).  In these cases, do it
		manually.  However, you get no retry capability here. */
		autofire_open_for_trough ();
		/* Wait for divertor to open */
		task_sleep_sec (2);		
		sol_request (SOL_BALL_SERVE);
	}
	else
	{
		/* The normal way to kick a ball from the trough.
		 * dev_trough_kick_attempt will be called next */
		device_request_kick (device_entry (DEVNO_TROUGH));
	}
}



/** Signals that a ball is headed to the autofire lane from the ramp
divertor and that it needs to be caught in the autoplunger, rather than
falling into the manual plunger lane */
void autofire_catch (void)
{
	shooter_div_delay_time = 0;
	shooter_div_open_time = 4;
	task_create_gid1_while (GID_AUTOFIRE_HANDLER, autofire_monitor, TASK_DURATION_INF);
}


CALLSET_ENTRY (autofire, dev_trough_kick_attempt)
{
	/* Decide whether or not to open the autofire divertor before
	kicking the ball.  If so, then also wait until the autofire
	lane is clear before allowing the ball to go. */

	while (task_find_gid (GID_MUTE_AND_PAUSE))
		task_sleep (TIME_100MS);
	
	dbprintf ("need to autofire? live=%d, request=%d ...",
		live_balls, autofire_request_count);
	if (live_balls || autofire_request_count)
	{
		dbprintf ("yes.\n");
		bounded_decrement (autofire_request_count, 0);

		/* Need to open the divertor */
		autofire_open_for_trough ();
		/* Wait for the divertor to open */	
		task_sleep_sec (1);
	}
	else
	{
		dbprintf ("no.\n");
	}
	/* Autofire solenoid will now be pulsed */
}

CALLSET_ENTRY (autofire, clear_autofire)
{
	/* Used to empty autofire if found full
	 * during attract mode */
	shooter_div_start ();
	task_sleep_sec (2);
	sol_request (SOL_AUTOFIRE);
	task_sleep_sec (1);
	shooter_div_stop ();
}

CALLSET_ENTRY (autofire, ball_search)
{
	/* The shooter divertor/autofire are both kicked here
	since there is a dependency between the two.  The main
	ball search routine is told not to kick either one of them. */
	if (switch_poll_logical (SW_AUTOFIRE2) || switch_poll_logical (SW_AUTOFIRE1))
	{
		callset_invoke (clear_autofire);
	}
	else if (feature_config.fire_when_detected_empty == YES)
	{
		callset_invoke (clear_autofire);
	}
}

CALLSET_ENTRY (autofire, start_ball)
{
	autofire_request_count = 0;
	autofire_busy = FALSE;
}

CALLSET_ENTRY (autofire, init)
{
	autofire_request_count = 0;
	autofire_busy = FALSE;
}

