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

/**
 * \file
 * \brief Entry of player's initials
 * This is used by the high score module, and could be used for
 * other initials entry like loop champions, etc.
 */


#define NUM_INITIALS_ALLOWED 3

/** The array of characters that can be entered.
 * Keep the length of this as a power of 2 (32) so that
 * the circular buffer implementation is simple.
 */
static const unsigned char initial_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ */-+";

#if (MACHINE_DMD == 1)
#define MAX_LETTERS_SHOWN 12
#else
#define MAX_LETTERS_SHOWN 1
#endif
#define MAX_INITIAL_INITIAL (sizeof (initial_chars) - MAX_LETTERS_SHOWN)

#define SELECT_OFFSET 2
#define FONT_WIDTH 8
#define ALPHABET_LEN 32

/* The amount of time left to enter initials */
U8 initials_enter_timer;

/* The index of the next initial to be entered */
U8 initials_index;

/* The index of the currently selected letter/symbol */
U8 initials_selection;

/* The array of initials */
char initials_data[NUM_INITIALS_ALLOWED+1];

/* The callback function to invoke when initials are entered OK */
void (*initials_enter_complete) (void);


/**
 * The display effect for the enter initials screen.
 *
 * It continuously redraws itself whenever the timer, the selected
 * character, or the entered characters change.
 */
void enter_initials_deff (void)
{
	while (in_test || task_find_gid (GID_ENTER_INITIALS))
	{
		if (score_update_required ())
		{
#if (MACHINE_DMD == 1)
			U8 n;
			dmd_alloc_low_clean ();
			font_render_string_left (&font_var5, 0, 1, "ENTER INITIALS");
			font_render_string_left (&font_fixed10, 0, 9, initials_data);

			if (initials_selection < MAX_INITIAL_INITIAL+1)
			{
				sprintf ("%12s", initial_chars + initials_selection);
				font_render_string_left (&font_bitmap8, 0, 23, sprintf_buffer);
			}
			else
			{
				U8 x;

				x = ALPHABET_LEN - initials_selection;
				sprintf ("%*s", x, initial_chars + initials_selection);
				font_render_string_left (&font_bitmap8, 0, 23, sprintf_buffer);

				x = MAX_LETTERS_SHOWN - x;
				sprintf ("%*s", x, initial_chars);
				x = MAX_LETTERS_SHOWN - x;
				font_render_string_left (&font_bitmap8, x * FONT_WIDTH, 23, sprintf_buffer);
			}

			for (n = 22; n <= 30; n++)
			{
				dmd_low_buffer[16UL * n + SELECT_OFFSET - 1] ^= 0x80;
				dmd_low_buffer[16UL * n + SELECT_OFFSET] ^= 0x7F;
			}

			sprintf ("%d", initials_enter_timer);
			font_render_string_right (&font_fixed6, 126, 3, sprintf_buffer);
			dmd_show_low ();
#else
			seg_alloc_clean ();
			seg_write_string (0, 0, "ENTER INITIALS");
			sprintf ("%c", initial_chars[initials_selection]);
			seg_write_string (0, 15, sprintf_buffer);
			seg_write_string (1, 0, initials_data);
			sprintf ("%d", initials_enter_timer);
			seg_write_string (1, 14, sprintf_buffer);
			seg_show ();
#endif
		}
		task_sleep (TIME_66MS);
	}
	task_sleep (TIME_500MS);
	deff_exit ();
}

void initials_stop (void)
{
	task_sleep_sec (1);
	task_kill_gid (GID_ENTER_INITIALS);
	initials_enter_timer = 0;
}


static void initials_running (void)
{
	task_sleep_sec (1);
	initials_enter_timer = 30;
	memset (initials_data, 0, sizeof (initials_data));
	initials_index = 0;
	initials_selection = 0;

#if 1
	initials_enter_complete = null_function;
#endif

	while (initials_enter_timer > 0)
	{
		task_sleep (TIME_1S + TIME_66MS);
		initials_enter_timer--;
		score_update_request ();
	}
	task_exit ();
}


void initials_enter (void)
{
	task_create_gid1 (GID_ENTER_INITIALS, initials_running);
	deff_start_sync (DEFF_ENTER_INITIALS);
}


CALLSET_ENTRY (initials, init)
{
	initials_enter_timer = 0;
}


CALLSET_ENTRY (initials, sw_left_button)
{
	if (initials_enter_timer)
	{
		--initials_selection;
		initials_selection %= ALPHABET_LEN;
		score_update_request ();
	}
}


CALLSET_ENTRY (initials, sw_right_button)
{
	if (initials_enter_timer)
	{
		++initials_selection;
		initials_selection %= ALPHABET_LEN;
		score_update_request ();
	}
}


CALLSET_ENTRY (initials, start_button_handler)
{
	if (initials_enter_timer && initials_index < NUM_INITIALS_ALLOWED)
	{
		initials_data[initials_index] =
			initial_chars[(initials_selection + SELECT_OFFSET) % ALPHABET_LEN];
		score_update_request ();
		if (++initials_index == NUM_INITIALS_ALLOWED)
		{
			(*initials_enter_complete) ();
			initials_stop ();
		}
	}
}

