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

/* CALLSET_SECTION (leffs, __machine2__) */

#include <freewpc.h>

U8 lamplist;

void color_cycle_leff (void)
{
	U8 list;
	for (;;)
	{
		for (list = LAMPLIST_RED_LAMPS; list <= LAMPLIST_BLUE_LAMPS; list++)
		{
			lamplist_apply_nomacro (list, leff_on);
			task_sleep (TIME_100MS);
			lamplist_apply_nomacro (list, leff_off);
		}
	}
}

void build_up_task (void)
{
	lamplist_apply (LAMPLIST_BUILD_UP, leff_toggle);
	task_exit ();
}

void build_up_leff (void)
{
	lamplist_set_apply_delay (TIME_16MS);
	leff_create_peer (build_up_task);
	task_sleep (TIME_500MS);
	leff_create_peer (build_up_task);
	task_sleep (TIME_500MS);
	task_kill_peers ();
	leff_exit ();
}

static void amode_leff1 (void)
{
	register U8 my_lamplist = lamplist;
	lamplist_set_apply_delay (TIME_66MS);
	for (;;)
		lamplist_apply (my_lamplist, leff_toggle);
}

void amode_leff (void)
{
	gi_leff_enable (PINIO_GI_STRINGS);
	for (lamplist = LAMPLIST_TOP_ROLLOVERS; lamplist <= LAMPLIST_EXTRA_BALL_ROLLOVERS; lamplist++)
	{
		leff_create_peer (amode_leff1);
		task_sleep (TIME_66MS);
	}
	task_exit ();
}

void left_ramp_leff (void)
{
	U8 i;
	for (i=0; i < 4; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse (FLASH_ZR1_UNDERSIDE);
		task_sleep (TIME_100MS);
		flasher_pulse (FLASH_ZR1_UNDERSIDE);
		task_sleep (TIME_100MS * 2);
	}
	leff_exit ();
}

void right_ramp_leff (void)
{
	U8 i;
	for (i=0; i < 4; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse (FLASH_RIGHT_RAMPS);
		task_sleep (TIME_100MS);
		flasher_pulse (FLASH_RIGHT_RAMPS);
		task_sleep (TIME_100MS * 2);
	}
	leff_exit ();
}

void orbit_shot_leff (void)
{
	U8 i;
	for (i=0; i < 1; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS * 2);
	}
	leff_exit ();
}


void two_combos_shot_leff (void)
{
	U8 i;
	for (i=0; i < 1; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS * 2);
	}
	leff_exit ();
}

void three_combos_shot_leff (void)
{
	gi_leff_disable (PINIO_GI_STRINGS);
	gi_leff_dim (GI_PLAYFIELD_UL, 3);
	gi_leff_dim (GI_PLAYFIELD_UR, 3);
	gi_leff_enable (GI_PLAYFIELD_LL + GI_PLAYFIELD_LR);

	U8 i;
	for (i=0; i < 2; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS * 2);
	}
	leff_exit ();
	gi_leff_enable (PINIO_GI_STRINGS);
}


void four_combos_shot_leff (void)
{
	gi_leff_disable (PINIO_GI_STRINGS);
	gi_leff_enable (GI_PLAYFIELD_LL + GI_PLAYFIELD_LR);

	U8 i;
	for (i=0; i < 3; i++)
	{
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS * 2);
	}
	gi_leff_enable (PINIO_GI_STRINGS);
	leff_exit ();
}

void five_combos_shot_leff (void)
{
	gi_leff_disable (PINIO_GI_STRINGS);
	U8 i;
	for (i=0; i < 4; i++)
	{

		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS);
		flasher_pulse(FLASH_RIGHT_STANDUP);
		task_sleep (TIME_100MS * 2);
	}
	gi_leff_enable (PINIO_GI_STRINGS);
	leff_exit ();
}

void pitstop_popper_eject_leff (void)
{
	lamplist_apply_leff_alternating (LAMPLIST_PIT_AND_CHALLENGE, 0);

	U8 i;
	for (i=0; i < 16; i++)
	{
		lamplist_apply (LAMPLIST_PIT_AND_CHALLENGE, leff_toggle);
		task_sleep (TIME_100MS);
	}
	leff_exit ();
}
