/*
 * Copyright 2011 by Dominic Clifton <me@dominicclifton.name>
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

/* CALLSET_SECTION (sounds, __machine2__) */

#include <freewpc.h>

CALLSET_ENTRY (sounds, left_orbit_combo_shot)
{
	sound_start (ST_SAMPLE, SND_TRAFFIC_ZOOM_01, SL_2S, PRI_GAME_QUICK1);
}

CALLSET_ENTRY (sounds, right_orbit_combo_shot)
{
	sound_start (ST_SAMPLE, SND_TRAFFIC_ZOOM_02, SL_2S, PRI_GAME_QUICK1);
}

CALLSET_ENTRY (sounds, zr1_ramp_combo_shot)
{
	sound_start (ST_SAMPLE, SND_ENGINE_REV_04, SL_2S, PRI_GAME_QUICK1);
}

CALLSET_ENTRY (sounds, route66_ramp_combo_shot)
{
	sound_start (ST_SAMPLE, SND_ROUTE_66, SL_2S, PRI_GAME_QUICK1);
}

CALLSET_ENTRY (sounds, skidpad_ramp_combo_shot)
{
	sound_start (ST_SAMPLE, SND_TIRE_SCREECH_03, SL_2S, PRI_GAME_QUICK5);
}

CALLSET_ENTRY (sounds, inner_loop_combo_shot)
{
	sound_start (ST_SAMPLE, SND_ENGINE_REV_05, SL_2S, PRI_GAME_QUICK5);
}

static void extra_ball_award_task(void) {
	sound_start (ST_MUSIC, SND_EXTRA_BALL_AWARD, SL_2S, PRI_EB);
	task_sleep_sec(2);
	sound_start (ST_SPEECH, SPCH_EXTRA_BALL_02, SL_2S, PRI_EB);
	task_exit();
}

CALLSET_ENTRY (sounds, extra_ball_award) {
	task_recreate_gid (GID_EXTRA_BALL_AWARD, extra_ball_award_task);
}
