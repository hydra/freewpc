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
#include <eb.h>
/** Index of the panel which is currently slow flashing (next to
 * be awarded) or fast flashing (running) */
__local__ U8 door_index;
/* Temporary value so deff shows the right award */
U8 door_index_awarded;
/** Number of door panels that have been started */
__local__ U8 door_panels_started;
extern __local__ U8 extra_ball_enable_count;
extern U8 unlit_shot_count;
//bool door_awarded_from_slot;
U8 door_active_lamp;

/** Total number of door panels, not counting the handle */
#define NUM_DOOR_PANELS 14

#define BTTZ_DOOR_INDEX NUM_DOOR_PANELS

/** Names of all the door panels, in order */
const char *door_panel_names[] = {
	"TOWNSQUARE MADNESS",
	"LIGHT EXTRA BALL",
	"SUPER SLOT",
	"CLOCK MILLIONS",
	"THE SPIRAL",
	"BATTLE THE POWER",
	"10 MILLION",
	"GREED",
	"THE CAMERA",
	"HITCHHIKER",
	"CLOCK CHAOS",
	"SUPER SKILL MB",
	"FAST LOCK",
	"LIGHT GUMBALL",
	"BACK TO THE ZONE",
};

const char *door_award_goals[] = {
	"JET BUMPERS",
	"LOCK LANE",
	"SLOT MACHINE",
	"CLOCK TARGET",
	"THE LOOPS",
	"RIGHT RAMP",
	"ANYTHING",
	"YELLOW TARGETS",
	"THE CAMERA",
	"BELOW THE CAMERA",
	"CLOCK TARGET",
	"LEFT RAMP",
	"LOCK LANE",
	"RIGHT LOOP",
	"EVERYTHING",
};

static void door_start_event (U8 id)
{
	switch (id)
	{
		case 0: callset_invoke (door_start_tsm); break;
		case 1: callset_invoke (door_start_eb); break;
		case 2: callset_invoke (door_start_sslot); break;
		case 3: callset_invoke (door_start_clock_millions); break;
		case 4: callset_invoke (door_start_spiral); break;
		case 5: callset_invoke (door_start_battle_power); break;
		case 6: callset_invoke (door_start_10M); break;
		case 7: callset_invoke (door_start_greed); break;
		case 8: callset_invoke (door_start_camera); break;
		case 9: callset_invoke (door_start_hitchhiker); break;
		case 10: callset_invoke (door_start_clock_chaos); break;
		case 11: callset_invoke (door_start_super_skill); break;
		case 12: callset_invoke (door_start_fast_lock); break;
		case 13: callset_invoke (door_start_light_gumball); break;
		case 14: callset_invoke (door_start_bttz); break;
	}
}


extern inline lampnum_t door_get_lamp (U8 id)
{
	return lamplist_index (LAMPLIST_DOOR_PANELS_AND_HANDLE, id);
}


extern inline U8 door_get_flashing_lamp (void)
{
	return door_get_lamp (door_index);
}

static inline bool can_award_door_panel (void)
{
	/* Panels not awarded during any multiball */
	if (multi_ball_play () || global_flag_test (GLOBAL_FLAG_BTTZ_RUNNING))
		return FALSE;
	else
		return TRUE;
}

static inline void door_set_flashing (U8 id)
{
	lamp_flash_off (door_get_flashing_lamp ());
	door_index = id;
	lamp_flash_on (door_get_flashing_lamp ());
}


static inline void door_advance_flashing (void)
{
	U8 new_door_index;

	if (door_panels_started < NUM_DOOR_PANELS)
	{
		new_door_index = door_index;
		do {
			new_door_index++;
			if (new_door_index >= NUM_DOOR_PANELS)
				new_door_index = 0;
		} while (lamp_test (door_get_lamp (new_door_index)));
	}
	else
		/* Light the door handle */
		new_door_index = BTTZ_DOOR_INDEX;

	door_set_flashing (new_door_index);
}

void door_award_rotate (void)
{
	task_sleep_sec (2);
	while (in_live_game)
	{
		door_advance_flashing ();
		task_sleep_sec (2);
	}
	task_exit ();
}

void slot_animation_sound_task (void)
{
	sound_send (SND_SLOT_PULL);
	task_sleep (TIME_800MS);
	sound_send (SND_SLOT_REEL);
	task_sleep (TIME_800MS);
	sound_send (SND_SLOT_REEL);
	task_sleep (TIME_800MS);
	sound_send (SND_SLOT_REEL);
	task_exit ();
}

void door_award_deff (void)
{
	U8 index = door_index_awarded;
	U16 fno;
	
	switch (door_index_awarded)
	{
		case 0:
			sound_send (SND_THERE_IS_MADNESS);
			break;
		case 1:
			sound_send (SND_GET_THE_EXTRA_BALL);	
			break;

		case 2:
			task_create_anon (slot_animation_sound_task);
			break;

		case 3:
			sound_send (SND_TIME_IS_A_ONEWAY_STREET);
			break;

		case 4:
			sound_send (SND_SPIRAL_AWAITS_YOU);
			break;
		case 5:
			sound_send (SND_ARE_YOU_READY_TO_BATTLE);
			break;
		
		case 6:
			sound_send (SND_TEN_MILLION_POINTS);
			break;

		case 7:
			sound_send (SND_FEEL_LUCKY);
			break;

		case 8:
			sound_send (SND_MOST_UNUSUAL_CAMERA);
			break;

		case 9:
			sound_send (SND_NOTE_THE_HITCHHIKER);
			break;

		case 10:
			sound_send (SND_QUIT_PLAYING_WITH_THE_CLOCK);
			break;

		case 11:
			sound_send (SND_THIS_SHOT_REQUIRES_SKILL_DUP);
			break;

		case 12:
			sound_send (SND_NOT_AN_ORDINARY_DAY);
			break;

		case 13:
			sound_send (SND_THIS_IS_NO_ORDINARY_GUMBALL);
			break;

		case 14:
			sound_send (SND_YOU_HAVE_COME_TO_THE_END);
			break;
	}
	sound_send (SND_NEXT_CAMERA_AWARD_SHOWN);
	/* Play once normally */
	for (fno = IMG_DOOR_START; fno <= IMG_DOOR_END; fno += 2)
	{
		dmd_alloc_pair ();
		frame_draw (fno);
		/* Flip it, as text is drawn to the low page */
		dmd_flip_low_high ();	
		//draw_door_award_text ();
		switch (door_index_awarded)
		{
			case 0:
				font_render_string_left (&font_mono5, 3, 3, "TOWNSQUARE");
				font_render_string_left (&font_mono5, 3, 16, "MADNESS");
				break;
			case 1:
				font_render_string_left (&font_mono5, 3, 3, "EXTRA BALL");
				font_render_string_left (&font_mono5, 3, 16, "LIT");
				break;
	
			case 2:
				font_render_string_left (&font_mono5, 3, 3, "SUPER");
				font_render_string_left (&font_mono5, 3, 16, "SLOT MACHINE");
				break;
	
			case 3:
				font_render_string_left (&font_mono5, 3, 3, "CLOCK");
				font_render_string_left (&font_mono5, 3, 16, "MILLIONS");
				break;
	
			case 4:
				font_render_string_left (&font_mono5, 3, 3, "SPIRAL");
				font_render_string_left (&font_mono5, 3, 16, "ROUND");
				break;
			case 5:
				font_render_string_left (&font_mono5, 3, 3, "BATTLE THE");
				font_render_string_left (&font_mono5, 3, 16, "POWER");
				break;
			
			case 6:
				font_render_string_left (&font_mono5, 3, 3, "10 MILLION");
				font_render_string_left (&font_mono5, 3, 16, "POINTS");
				break;
	
			case 7:
				font_render_string_left (&font_mono5, 3, 3, "GREED");
				font_render_string_left (&font_mono5, 3, 16, "ROUND");
				break;
	
			case 8:
				font_render_string_left (&font_mono5, 3, 3, "CAMERA");
				font_render_string_left (&font_mono5, 3, 16, "LIT");
				break;
	
			case 9:
				font_render_string_left (&font_mono5, 3, 3, "HITCHHIKER");
				font_render_string_left (&font_mono5, 3, 16, "ROUND");
				break;
	
			case 10:
				font_render_string_left (&font_mono5, 3, 3, "CLOCK CHAOS");
				font_render_string_left (&font_mono5, 3, 16, "MULTIBALL");
				break;
	
			case 11:
				font_render_string_left (&font_mono5, 3, 3, "SUPER SKILL");
				font_render_string_left (&font_mono5, 3, 16, "MULTIBALL");
				break;
	
			case 12:
				font_render_string_left (&font_mono5, 3, 3, "FASTLOCK");
				font_render_string_left (&font_mono5, 3, 16, "ROUND");
				break;
	
			case 13:
				font_render_string_left (&font_mono5, 3, 3, "GUMBALL");
				font_render_string_left (&font_mono5, 3, 16, "LIT");
				break;
	
			case 14:
				font_render_string_left (&font_mono5, 3, 3, "BACK TO");
				font_render_string_left (&font_mono5, 3, 16, "THE ZONE");
				break;
	
		}		
		/* Flip it again so text is now on high page */
		dmd_flip_low_high ();	
		dmd_show2 ();
		task_sleep (TIME_66MS);
	}
	task_sleep_sec (1);	
	/* Play backwards */
	sound_send (SND_SPIRAL_EB_LIT);
	for (fno = IMG_DOOR_END; fno >= IMG_DOOR_START; fno -= 2)
	{
		dmd_alloc_pair ();
		/* Draw the frame, leave it blank at the end */
		if (fno == IMG_DOOR_START)
		{
			dmd_clean_page_low ();
			dmd_clean_page_high ();
		}
		else
			frame_draw (fno);
		
		/* Flip it, as text is drawn to the low page */
		dmd_flip_low_high ();	
		font_render_string_center (&font_fixed6, 48, 9, "SHOOT");
		font_render_string_center (&font_var5, 48, 22, door_award_goals[index]);
		/* Flip it again so text is now on high page */
		dmd_flip_low_high ();	
		dmd_show2 ();
		task_sleep (TIME_66MS);
	}
	task_sleep_sec (2);
	deff_exit ();
}

void door_award_enable (void)
{
	if (can_award_door_panel ())
		task_recreate_gid (GID_DOOR_AWARD_ROTATE, door_award_rotate);
}

CALLSET_ENTRY (door, lamp_update)
{
	if (can_award_door_panel () && flag_test (FLAG_PIANO_DOOR_LIT))
		lamp_on (LM_PIANO_PANEL);
	else
		lamp_off (LM_PIANO_PANEL);
	/* Don't turn the lamp on if sslot is running */
	if (can_award_door_panel () && flag_test (FLAG_SLOT_DOOR_LIT) 
		&& !task_find_gid (GID_SSLOT_AWARD_ROTATE))
		lamp_on (LM_SLOT_MACHINE);
	else
		lamp_off (LM_SLOT_MACHINE);
}

CALLSET_ENTRY (door, award_door_panel)
{
	task_create_anon (award_door_panel_task);
}

CALLSET_ENTRY (door, door_start_10M)
{
	score (SC_10M);
}

CALLSET_ENTRY (door, ball_count_change)
{
	door_lamp_update ();
}

void award_door_panel_task (void)
{
	unlit_shot_count = 0;
	/* Stop the door lamps rotating */
	task_kill_gid (GID_DOOR_AWARD_ROTATE);
	/* Store the current door index */
	door_index_awarded = door_index;
	/* Start the event and show deff */
	deff_start (DEFF_DOOR_AWARD);
	door_start_event (door_index);
	/* Find and turn on the current flashing lamp */
	door_active_lamp = door_get_flashing_lamp ();
	lamp_tristate_on (door_active_lamp);
	leff_start (LEFF_DOOR_STROBE);
	
	score (SC_5M);
	timed_game_extend (10);
	door_panels_started++;
	audit_increment (&feature_audits.door_panels);
	
	switch (door_panels_started)
	{
		case 3:
			audit_increment (&feature_audits._3_panel_games);
			break;
		case 6:
			audit_increment (&feature_audits._6_panel_games);
			break;
		case 9:
			audit_increment (&feature_audits._9_panel_games);
			break;
		case 12:
			audit_increment (&feature_audits._12_panel_games);
			break;
	}

	score (SC_50K);
	/* Restart the door rotation */
	door_award_enable ();

	task_exit ();
}

CALLSET_ENTRY(door, sw_piano)
{
	if (can_award_door_panel () && flag_test (FLAG_PIANO_DOOR_LIT))
	{
		flag_off (FLAG_PIANO_DOOR_LIT);
		flag_on (FLAG_SLOT_DOOR_LIT);
		//callset_invoke (award_door_panel);
		task_create_anon (award_door_panel_task);
	}
	else
	{
		/* Relight the piano if it was unlit when hit */
		if ( feature_config.easy_light_door_panels == YES
			&& door_panels_started < 8)
		{
			flag_on (FLAG_PIANO_DOOR_LIT);
		}
		award_unlit_shot (SW_PIANO);
		score (SC_5130);
		sound_send (SND_ODD_CHANGE_BEGIN);
	}
}

CALLSET_ENTRY (door, shot_slot_machine)
{
	if (can_award_door_panel () && flag_test (FLAG_SLOT_DOOR_LIT))
	{
		flag_off (FLAG_SLOT_DOOR_LIT);
		flag_on (FLAG_PIANO_DOOR_LIT);
		callset_invoke (award_door_panel);
	}
	else
	{
		/* Relight the slot if it was unlit when hit */
		if ( feature_config.easy_light_door_panels == YES
			&& door_panels_started < 8)
		{
			flag_on (FLAG_SLOT_DOOR_LIT);
		}
		award_unlit_shot (SW_SLOT);
		score (SC_5130);
	}
}

CALLSET_ENTRY (door, door_start_eb)
{
	if (can_award_extra_ball ())
		light_easy_extra_ball ();
}

CALLSET_ENTRY(door, start_player)
{
	/* Pick a random door to start on */
	door_index = random_scaled (15);
	door_panels_started = 0;
	flag_on (FLAG_PIANO_DOOR_LIT);
	flag_on (FLAG_SLOT_DOOR_LIT);

	if (system_config.max_ebs == 0)
	{
		lamp_on (LM_PANEL_EB);
		door_panels_started++;
	}
	door_lamp_update ();
}

CALLSET_ENTRY(door, start_ball)
{
	lamplist_apply (LAMPLIST_DOOR_PANELS_AND_HANDLE, lamp_flash_off);
	door_award_enable ();
}

CALLSET_ENTRY(door, machine_paused)
{
	/* Stop the door lamps rotating */
	task_kill_gid (GID_DOOR_AWARD_ROTATE);
}

CALLSET_ENTRY(door, machine_unpaused)
{
	/* Start the door lamps rotating again */
	door_award_enable ();
}
