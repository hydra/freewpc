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

/**
 * FIXME if there are less than PITSTOP_AWARD_ITEMS items after disabling unavilable awards a crash occurs (due to infinite loop)
 */
#include <freewpc.h>
#include <eb.h>

enum possible_pitstop_award {
	PITSTOP_AWARD_MIN = 0,
	AWARD_CAR = PITSTOP_AWARD_MIN,
	AWARD_SCORE_5M,
	AWARD_SCORE_50M,
	AWARD_BONUS_MULTIPLIER,
	AWARD_LITE_LOCK,
	AWARD_DRAGRACE,
	AWARD_EXTRA_BALL,
	// TODO add more things!
	PITSTOP_AWARD_MAX = AWARD_EXTRA_BALL,
	DISABLED_AWARD
};

#define PITSTOP_AWARD_COUNT (PITSTOP_AWARD_MAX + 1)

/** one title per award */
char *pitstop_award_options_titles[PITSTOP_AWARD_COUNT] = {
	"COLLECT CAR",
	"5 MILLION",
	"50 MILLION",
	"BONUS MULTIPLIER",
	"LITE LOCK",
	"DRAG RACE",
	"EXTRA BALL"
};

/** the amount of items to choose from */
#define PITSTOP_AWARD_ITEMS 4

bool allowable_pitstop_awards[PITSTOP_AWARD_COUNT];

enum possible_pitstop_award selected_pitstop_awards[PITSTOP_AWARD_ITEMS];

U8 pitstop_award;
U8 find_pitstop_award_index;

bool have_already_selected_possible_award(U8 possible_award) {
#ifdef DEBUG_PITSTOP_AWARD
	dbprintf("looking for %d\n", possible_award);
#endif
	for (find_pitstop_award_index = 0; find_pitstop_award_index < PITSTOP_AWARD_ITEMS; find_pitstop_award_index++) {
#ifdef DEBUG_PITSTOP_AWARD
		dbprintf("comparing with %d\n", selected_pitstop_awards[find_pitstop_award_index]);
#endif
		if (selected_pitstop_awards[find_pitstop_award_index] == possible_award) {
#ifdef DEBUG_PITSTOP_AWARD
			dbprintf("matched\n");
#endif
			return TRUE;
		}
	}
#ifdef DEBUG_PITSTOP_AWARD
	dbprintf("unmatched\n");
#endif
	return FALSE;
}

void determine_allowable_pitstop_awards(void) {
	memset(allowable_pitstop_awards, 1, sizeof (allowable_pitstop_awards));
	memset(selected_pitstop_awards, DISABLED_AWARD, sizeof(selected_pitstop_awards));

	if (in_live_game) {
		if (have_collected_all_cars()) {
			allowable_pitstop_awards[AWARD_CAR] = FALSE;
		}

		// TODO if bonus multiplier is already 8x disallow it
		// TODO if lock is already lit, disallow it.

		if (global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
			allowable_pitstop_awards[AWARD_DRAGRACE] = FALSE;
		}

		if (!can_award_extra_ball()) {
			allowable_pitstop_awards[AWARD_EXTRA_BALL] = FALSE;
		}

		if (!zr1_mb_can_award_lite_lock()) {
			allowable_pitstop_awards[AWARD_LITE_LOCK] = FALSE;
		}

		// TODO if we've still got more than PITSTOP_AWARD_ITEMS allowable then disable the last awarded thing
	}

#ifdef DEBUG_PITSTOP_AWARD
	for (pitstop_award = 0; pitstop_award < PITSTOP_AWARD_COUNT; pitstop_award++) {
		dbprintf("award: %s (%d) is %s\n",
			pitstop_award_options_titles[pitstop_award],
			pitstop_award,
			allowable_pitstop_awards[pitstop_award] ? "ALLOWED" : "DISABLED"
		);
	}
#endif

	// select 4 random items from the list
	U8 pitstop_awards_selected;

#ifdef DEBUG_PITSTOP_AWARD
	dbprintf("choosing %d random items\n", PITSTOP_AWARD_ITEMS);
#endif

	for (pitstop_awards_selected = 0; pitstop_awards_selected < PITSTOP_AWARD_ITEMS; pitstop_awards_selected++) {
		do {
			pitstop_award = random_scaled(PITSTOP_AWARD_COUNT);
#ifdef DEBUG_PITSTOP_AWARD
			dbprintf("random (1): %d\n", pitstop_award);
#endif
		} while (allowable_pitstop_awards[pitstop_award] == FALSE || have_already_selected_possible_award(pitstop_award));
		selected_pitstop_awards[pitstop_awards_selected] = pitstop_award;
#ifdef DEBUG_PITSTOP_AWARD
		dbprintf("selected award: %s (%d)\n", pitstop_award_options_titles[pitstop_award], pitstop_award);
#endif
	}
}

U8 pitstop_award_index;

void eliminate_pitstop_award(void) {
	do {
		pitstop_award = random_scaled(PITSTOP_AWARD_ITEMS);
#ifdef DEBUG_PITSTOP_AWARD
		dbprintf("random (2): %d\n", pitstop_award);
#endif
	} while (selected_pitstop_awards[pitstop_award] == DISABLED_AWARD);

#ifdef DEBUG_PITSTOP_AWARD
	dbprintf("eliminated award: %s (%d)\n", pitstop_award_options_titles[pitstop_award], pitstop_award);
#endif
	selected_pitstop_awards[pitstop_award] = DISABLED_AWARD;
}


void pitstop_award_draw(void) {
	dmd_alloc_low_clean ();

	// draw list of non-elimited awards
	for (pitstop_award_index = 0; pitstop_award_index < PITSTOP_AWARD_ITEMS; pitstop_award_index++) {
		pitstop_award = selected_pitstop_awards[pitstop_award_index];
		if (pitstop_award != DISABLED_AWARD) {
#ifdef DEBUG_PITSTOP_AWARD
			dbprintf("drawing award: %d, %d, %s\n", pitstop_award_index, pitstop_award, pitstop_award_options_titles[pitstop_award]);
#endif
			font_render_string_center (&font_var5, 64, 5 + (6 * pitstop_award_index), pitstop_award_options_titles[pitstop_award]);
		}
#ifdef DEBUG_PITSTOP_AWARD
		else {
			dbprintf("skipping award: %d, %d, %s\n", pitstop_award_index, pitstop_award, pitstop_award_options_titles[pitstop_award]);
		}
#endif
	}
	dmd_show_low();
}

void pitstop_award_deff(void) {

	// must happen in the deff so it doesn't crash in test mode due to uninitialised list of items
	determine_allowable_pitstop_awards();

	U8 pitstop_award_items_remaining;
	for (pitstop_award_items_remaining = PITSTOP_AWARD_ITEMS; pitstop_award_items_remaining >= 1 ; pitstop_award_items_remaining--) {
		dbprintf("remaining items: %d\n", pitstop_award_items_remaining);
		pitstop_award_draw();
		task_sleep_sec(1);

		if (pitstop_award_items_remaining >= 3) {
			sample_start (SND_GUITAR_03, SL_100MS);
			eliminate_pitstop_award();
		} else if (pitstop_award_items_remaining == 2) {
			sample_start (SND_GUITAR_02, SL_100MS);
			eliminate_pitstop_award();
		} else if (pitstop_award_items_remaining == 1) {
			sample_start (SND_GUITAR_01, SL_100MS);
		}
		dmd_invert_page (dmd_low_buffer);
		dmd_show_low();
		task_sleep(TIME_100MS);
	}

	// find the one remaining award

	pitstop_award = DISABLED_AWARD;
	for (pitstop_award_index = 0; pitstop_award_index < PITSTOP_AWARD_ITEMS; pitstop_award_index++) {
		if (selected_pitstop_awards[pitstop_award_index] != DISABLED_AWARD) {
			pitstop_award = selected_pitstop_awards[pitstop_award_index];
		}
	}
	dbprintf("%s\n", pitstop_award_options_titles[pitstop_award]);

	// flash the remaining award

	dmd_alloc_pair ();
	dmd_clean_page_low ();
	font_render_string_center (&font_var5, 64, 16, pitstop_award_options_titles[pitstop_award]);
	dmd_show_low ();
	dmd_copy_low_to_high ();
	dmd_invert_page (dmd_low_buffer);
	deff_swap_low_high (10, TIME_100MS);

	deff_exit (); // ball will be ejected from popper soon

}

void pitstop_award_task(void) {

	deff_start_sync(DEFF_PITSTOP_AWARD);

	switch (pitstop_award) {
		case AWARD_SCORE_50M:
			score(SC_50M);
		break;
		case AWARD_SCORE_5M:
			score(SC_5M);
		break;
		case AWARD_CAR:
			award_car();
		break;
		case AWARD_EXTRA_BALL:
			increment_extra_balls();
		break;
		case AWARD_LITE_LOCK:
			zr1_mb_award_lite_lock();
		break;
		case AWARD_DRAGRACE:
			dragrace_enable();
		default:
		// TODO handle remaining awards
		break;
	}
	task_exit();
}

CALLSET_ENTRY(pitstop_award, dev_pitstop_popper_enter) {
	if (multi_ball_play()) {
		return;
	}

	task_kill_gid(GID_PITSTOP_AWARD_TASK);
	task_create_gid1(GID_PITSTOP_AWARD_TASK, pitstop_award_task);

}

CALLSET_BOOL_ENTRY(pitstop_award, dev_pitstop_popper_kick_request) {

	if (!task_find_gid(GID_PITSTOP_AWARD_TASK)) {
		return TRUE;
	}

	 // hold the kickout for a bit if the deff if still running
	return FALSE;
}
