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

#include <freewpc.h>

extern U8 sw_last_scheduled;
typedef void (*combo_handler_t) (void);

#define COMBO_START           (1 << 0)
#define COMBO_END             (1 << 1)
#define IGNORE_TIME           (1 << 2)
#define IGNORE_SWITCHES_UNTIL (1 << 3)
#define NO_FLAGS              0


typedef struct combo_switch_entry_s {
	U8 flags;
	U8 switch_id;
	U16 max_time_difference;
} combo_switch_entry_t;

typedef struct combo_definition_s {
	char *name; // XXX
	/** A function to call when the switch pattern produces an event.
	 * All functions are assumed to be callsets, and located in the
	 * callset page of the ROM. */
	combo_handler_t fn; // invoke with callset_pointer_invoke(fn);

	U8 switch_count;
	/** A list of switches and flags that define the sequence.
	 */
	combo_switch_entry_t switch_list[];
} combo_definition_t;


// NOTE due to reverse processing of combo switches, the IGNORE_SWITCHES_UNTIL flag and
// the time value must be on the line before you would expect normally it.


combo_definition_t left_orbit_combo = {
	.name = "LO",
	.fn = null_function,
	.switch_count = 2,
	.switch_list = {
		{COMBO_START, SW_LEFT_OUTER_LOOP,  TIME_66MS},
		{COMBO_END,   SW_RIGHT_OUTER_LOOP, 0}
	}
};

combo_definition_t right_orbit_combo = {
	.name = "RO",
	.fn = null_function,
	.switch_count = 2,
	.switch_list = {
		{COMBO_START, SW_RIGHT_OUTER_LOOP, TIME_66MS},
		{COMBO_END,   SW_LEFT_OUTER_LOOP,  0}
	}
};

combo_definition_t ordered_bank_combo1 = {
	.name = "OBC1",
	.fn = null_function,
	.switch_count = 3,
	.switch_list = {

		// this says, match SW_LEFT_STANDUP_3
		// and if SW_LEFT_STANDUP_2 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// and if SW_LEFT_STANDUP_1 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// then the combo is matched

		{COMBO_START|IGNORE_SWITCHES_UNTIL,	SW_LEFT_STANDUP_1, TIME_5S},
		{IGNORE_SWITCHES_UNTIL,             SW_LEFT_STANDUP_2, TIME_5S},
		{COMBO_END, 						SW_LEFT_STANDUP_3, 0}
	}
};


combo_definition_t right_left_orbit_and_right_left_ramp_combo = {
	.name = "RLO RLR",
	.fn = null_function,
	.switch_count = 8,
	.switch_list = {
		{COMBO_START,			SW_RIGHT_OUTER_LOOP,  TIME_66MS},
		{IGNORE_SWITCHES_UNTIL, SW_LEFT_OUTER_LOOP,   TIME_5S},
		{NO_FLAGS,				SW_LEFT_OUTER_LOOP,   TIME_66MS},
		{IGNORE_SWITCHES_UNTIL, SW_RIGHT_OUTER_LOOP,  TIME_5S},
		{NO_FLAGS,				SW_ROUTE_66_ENTRY,    TIME_1S},
		{IGNORE_SWITCHES_UNTIL, SW_ROUTE_66_EXIT,     TIME_5S},
		{NO_FLAGS,				SW_ZR1_BOTTOM_ENTRY,  TIME_1S},
		{COMBO_END,             SW_ZR1_TOP_ENTRY,     0}
	}
};


combo_definition_t *combo_list[] = {
	&left_orbit_combo,
	&right_orbit_combo,
	&ordered_bank_combo1,
	&right_left_orbit_and_right_left_ramp_combo
};

#define MAX_RECENT_SWITCHES 32

typedef struct recent_switch_s {
	U8 switch_id;
	U16 hit_time;
} recent_switch_t;


recent_switch_t recent_switches[MAX_RECENT_SWITCHES];
U8 next_recent_switch; // a tail pointer which points to the next slot to be used which is also the oldest recent-switch.

CALLSET_ENTRY (combo_handler, init_complete, start_ball)
{
	next_recent_switch = 0;
	memset(recent_switches, 0, sizeof(recent_switches));
}

void dump_recent_switches( void ) {
	U8 dump_count = 0;
	U8 index = next_recent_switch;
	recent_switch_t *recent_switch;
	while (dump_count < MAX_RECENT_SWITCHES) {
		if (index == 0) {
			index = MAX_RECENT_SWITCHES;
		}
		index--;
		recent_switch = &recent_switches[index];
		dbprintf("dc: %d, i: %d, s: %d, t: %ld\n", dump_count, index, recent_switch->switch_id, recent_switch->hit_time);
		task_runs_long();


		dump_count++;
	}
}

combo_definition_t *last_matched_combo;

void combo_name_deff (void)
{
	sprintf("%s", last_matched_combo->name);
	flash_and_exit_deff (20, TIME_100MS);
}

void process_combo(combo_definition_t *combo) {
	dbprintf("processing combo: %s\n", combo->name);

	// compare the combo switch list, and timings, to the recent switch list.

	U8 recent_index = next_recent_switch;
	// combo are processed in reverse order from last switch to first
	// because there's less matches to attempt that way round
	U8 combo_index = combo->switch_count;
	U8 matched;
	recent_switch_t *recent_switch;
	combo_switch_entry_t *combo_switch;
	U16 previous_recent_time = 0;

	while (combo_index > 0) {
		combo_index--;
		combo_switch = &combo->switch_list[combo_index];


		matched = FALSE;
		do {
			task_runs_long();
			if (recent_index == 0) {
				recent_index = MAX_RECENT_SWITCHES;
			}
			recent_index--;

			recent_switch = &recent_switches[recent_index];

			dbprintf("ci: %d, cs: %d, ri: %d, rs: %d, t: %ld\n",
				combo_index,
				combo_switch->switch_id,
				recent_index,
				recent_switch->switch_id,
				recent_switch->hit_time
			);

			if (recent_switch->switch_id == 0) {
				dbprintf("ran out of recent switches\n");
				break;
			}
			if (recent_switch->switch_id == combo_switch->switch_id) {
				matched = TRUE;

				// don't check time on the first switch as there's no other time to compare it with
				if (combo_index < combo->switch_count - 1) {
					dbprintf("diff between previous: %ld\n", previous_recent_time - recent_switch->hit_time);
					// was this switch activated within the timeout period allowed since the previous switch?
					dbprintf("combo time allowed: %ld\n", combo_switch->max_time_difference * IRQS_PER_TICK);
					if (combo_switch->max_time_difference * IRQS_PER_TICK > previous_recent_time - recent_switch->hit_time) {
						dbprintf("time elapsed is OK!\n");
					} else {
						dbprintf("too much time elaposed\n");
						// break; // TODO enable this when switch matching is OK.
					}
				}

				previous_recent_time = recent_switch->hit_time;
			} else {
				dbprintf("unmatched\n");
				if (combo_switch->flags & IGNORE_SWITCHES_UNTIL) {
					dbprintf("ignoring switch due to flag\n");
				}

				if (recent_index == next_recent_switch) {
					dbprintf("wrapped, bailing!\n");
					break;
				}
			}
		} while (!matched && (combo_switch->flags & IGNORE_SWITCHES_UNTIL));

		if (!matched) {
			break;
		}
		dbprintf("matched switch\n");

		if (combo_index == 0) {
			sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
			score (SC_1M);
			dbprintf("combo matched!\n");
			last_matched_combo = combo;
			deff_start(DEFF_COMBO_NAME);
		}
	}
}

void process_combos() {
	U8 combo_count = sizeof(combo_list) / sizeof(combo_definition_t *);
	dbprintf("processing %d combos\n", combo_count);
	U8 combo_index = 0;
	while (combo_index < combo_count) {
		process_combo(combo_list[combo_index]);
		combo_index++;
	}
}

CALLSET_ENTRY (combo_handler, any_pf_switch)
{
	U8 sw = sw_last_scheduled;
	U16 now = get_sys_time();
	dbprintf("now: %ld, sw: %d\n", now, sw);

	recent_switch_t *switch_hit = &recent_switches[next_recent_switch];
	switch_hit->hit_time = now;
	switch_hit->switch_id = sw_last_scheduled;

	next_recent_switch++;
	if (unlikely( next_recent_switch >= MAX_RECENT_SWITCHES)) {
		next_recent_switch = 0;
	}

	dump_recent_switches();

	process_combos();
}

CALLSET_ENTRY (combo, single_left_orbit_shot)
{
	dbprintf("single_left_orbit_shot handler invoked\n");
}

