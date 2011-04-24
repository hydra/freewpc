/*
 * Copyright 2010 by Brian Dominy <brian@oddchange.com>
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

//TODO Make this play nice with Skill shot animation
/* CALLSET_SECTION (rocket, __machine2__) */
#include <freewpc.h>

extern bool skill_shot_enabled;

/* Rocket animation contributed by highrise */
void rocket_deff (void)
{
	/* Show loading frames and wait for kick */
	U16 fno;
	for (fno = IMG_ROCKET_LOAD_START; fno <= IMG_ROCKET_LOAD_END; fno += 2)
	{
		dmd_alloc_pair ();
		frame_draw (fno);
		dmd_overlay_onto_color ();
		dmd_show2 ();
		task_sleep (TIME_66MS);
		dmd_map_overlay ();
	}
	task_sleep (TIME_200MS);
	/* Rocket takes 500ms before kick 
	 * load animation takes 400ms */
	/* Launch rocket */
	for (fno = IMG_NEWROCKET_START; fno <= IMG_NEWROCKET_END; fno += 2)
	{
		dmd_alloc_pair_clean ();
		frame_draw (fno);
		dmd_show2 ();
		task_sleep (TIME_33MS);
	}
	deff_exit ();
}

CALLSET_ENTRY (rocket, dev_rocket_enter)
{
	disable_skill_shot ();
	score (SC_10K);
}

static void rocket_kick_sound (void)
{
	sound_send (SND_ROCKET_KICK_DONE);
	flasher_pulse (FLASH_UR_FLIPPER);
	task_exit ();
}

CALLSET_ENTRY (rocket, dev_rocket_kick_attempt)
{
	if (in_live_game)
	{
		/* Wait until the skill shot has finished */
		while (skill_shot_enabled ||
			deff_get_active () == DEFF_SKILL_SHOT_MADE ||
			task_find_gid (GID_SKILL_SWITCH_TRIGGER))
		{
			task_sleep (TIME_500MS);
		}
		if (!multi_ball_play ())
			leff_start (LEFF_ROCKET);
		sound_send (SND_ROCKET_KICK_REVVING);
		deff_start (DEFF_ROCKET);
		task_sleep (TIME_500MS);
		task_create_gid (0, rocket_kick_sound);
	}
}
