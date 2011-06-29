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

#ifdef CONFIG_COMBOS

#define COMBO_START           (1 << 0)
#define COMBO_END             (1 << 1)
#define IGNORE_TIME           (1 << 2)
#define IGNORE_SWITCHES_UNTIL (1 << 3)
#define NO_FLAGS              0


#define CF_NEVER               0
#define CF_ALWAYS              (1 << 0)
#define CF_SINGLE_BALL_ONLY    (1 << 1)
#define CF_MULTI_BALL_ONLY     (1 << 3)

// NOTE due to reverse processing of combo switches, the IGNORE_SWITCHES_UNTIL flag and
// the time value must be on the line before you would expect normally it.

extern void callset_left_orbit_combo_shot (void);
extern void callset_right_orbit_combo_shot (void);
extern void callset_rl_orbit_rl_ramp_combo_shot (void);
extern void callset_ordered_bank_1_combo_shot (void);

combo_definition_t left_orbit_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "LO",
	.fn = callset_left_orbit_combo_shot,
	.switch_count = 2,
	.switch_list = {
		{SW_LEFT_OUTER_LOOP,  TIME_66MS, COMBO_START},
		{SW_RIGHT_OUTER_LOOP, 0,         COMBO_END}
	}
};

combo_definition_t right_orbit_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "RO",
	.fn = callset_right_orbit_combo_shot,
	.switch_count = 2,
	.switch_list = {
		{SW_RIGHT_OUTER_LOOP, TIME_66MS, COMBO_START},
		{SW_LEFT_OUTER_LOOP,  0,         COMBO_END}
	}
};

combo_definition_t ordered_bank_1_combo = {
	.flags = CF_ALWAYS,
	.name = "OBC1",
	.fn = callset_ordered_bank_1_combo_shot,
	.switch_count = 3,
	.switch_list = {

		// this says, match SW_LEFT_STANDUP_3
		// and if SW_LEFT_STANDUP_2 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// and if SW_LEFT_STANDUP_1 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// then the combo is matched

		{SW_LEFT_STANDUP_1, TIME_5S, COMBO_START|IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_STANDUP_2, TIME_5S, IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_STANDUP_3, 0,       COMBO_END}
	}
};


combo_definition_t right_left_orbit_and_right_left_ramp_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "RLO RLR",
	.fn = callset_rl_orbit_rl_ramp_combo_shot,
	.switch_count = 8,
	.switch_list = {
		{SW_RIGHT_OUTER_LOOP,  TIME_66MS, COMBO_START},
		{SW_LEFT_OUTER_LOOP,   TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_OUTER_LOOP,   TIME_66MS, NO_FLAGS},
		{SW_RIGHT_OUTER_LOOP,  TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_ROUTE_66_ENTRY,    TIME_1S,   NO_FLAGS},
		{SW_ROUTE_66_EXIT,     TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_ZR1_BOTTOM_ENTRY,  TIME_1S,   NO_FLAGS},
		{SW_ZR1_TOP_ENTRY,     0,         COMBO_END}
	}
};


combo_definition_t *combo_list[] = {
	&left_orbit_combo,
	&right_orbit_combo,
	&ordered_bank_1_combo,
	&right_left_orbit_and_right_left_ramp_combo
};

combo_definition_t *last_matched_combo;

#define MATCHED_NONE   0
#define MATCHED_SWITCH (1 << 0)
#define MATCHED_TIME   (1 << 1)

void process_combo(combo_definition_t *combo) {
#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("processing combo: %s\n", combo->name);
#endif

	if (!(
			(combo->flags & CF_ALWAYS) ||
			((combo->flags & CF_SINGLE_BALL_ONLY) && single_ball_play()) ||
			((combo->flags & CF_MULTI_BALL_ONLY) && !single_ball_play())
	)) {
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("skipping, flags: %d, sbp: %d\n", combo->flags, single_ball_play());
#endif
		return;
	}

	// compare the combo switch list, and timings, to the recent switch list.

	U8 recent_index = next_recent_switch;
	// combo are processed in reverse order from last switch to first
	// because there's less matches to attempt that way round
	U8 combo_index = combo->switch_count;
	U8 matched;
	recent_switch_t *recent_switch;
	combo_switch_entry_t *combo_switch;
	U8 last_checked_switch_id = 0;
	U16 previous_recent_time = 0;

	while (combo_index > 0) {
		combo_index--;
		combo_switch = &combo->switch_list[combo_index];


		matched = MATCHED_NONE;
		do {
			task_runs_long();
			if (recent_index == 0) {
				recent_index = MAX_RECENT_SWITCHES;
			}
			recent_index--;

			recent_switch = &recent_switches[recent_index];

#ifdef CONFIG_DEBUG_COMBOS
			dbprintf("ci: %d, cs: %d, ri: %d, rs: %d, t: %ld\n",
				combo_index,
				combo_switch->switch_id,
				recent_index,
				recent_switch->switch_id,
				recent_switch->hit_time
			);
#endif

			if (recent_switch->switch_id == 0) {
#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("ran out of recent switches, bailing\n");
#endif
				break;
			}
/*

recent:
R L L L R    should match
R L L R      should match
R L L R R    should not match
combo:
R L* L R

recent:
1 2 3 3      should not match
1 2 3 2 3    should not match
1 2 3 1 3    should not match
<1 2 3>        should match
1 <1 2 2 3>    should match
1 <1 2 2 3 3>  should not match

1 2 3 2 3

combo:
1* 2* 3


 */
			// FIXME this fix breaks the use of two switches in a row in the combo spec though.
			if (last_checked_switch_id == recent_switch->switch_id && last_checked_switch_id && (combo->switch_list[combo_index+1].flags & IGNORE_SWITCHES_UNTIL)) {
				// Avoid matching multiple-switch presses of the same switch.
				// when the previous combo switch wasn't using IGNORE_SWITCHES_UNTIL
				// Without this check it's possible to repeatedly trigger a combo by hitting the last switch as long as the
				// rest of the switches are in the recent_switches list and the time between switch hits is
			    // still within the allowed time when IGNORE_SWITCHES_UNTIL is used in the combo definition
				// on the second-to-last switch.

#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("same as last checked switch, bailing\n");
#endif

				break;
			}
			last_checked_switch_id = recent_switch->switch_id;

			if (recent_switch->switch_id == combo_switch->switch_id) {
				matched |= MATCHED_SWITCH;
				// don't check time on the first switch as there's no other time to compare it with
				if (combo_index < combo->switch_count - 1) {
#ifdef CONFIG_DEBUG_COMBOS
					dbprintf("diff between previous: %ld\n", previous_recent_time - recent_switch->hit_time);
					// was this switch activated within the timeout period allowed since the previous switch?
					dbprintf("combo time allowed: %ld\n", combo_switch->max_time_difference * IRQS_PER_TICK);
#endif
					if (combo_switch->max_time_difference * IRQS_PER_TICK > previous_recent_time - recent_switch->hit_time) {
#ifdef CONFIG_DEBUG_COMBOS
						dbprintf("time elapsed is OK!\n");
#endif
						matched |= MATCHED_TIME;
					} else {
#ifdef CONFIG_DEBUG_COMBOS
						dbprintf("too much time elapsed\n");
#endif
					}
				} else {
					// if the first switch is the last one in the combo's list of switches, it matches regardless of times
					matched |= MATCHED_TIME;
				}
				previous_recent_time = recent_switch->hit_time;
			} else {
#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("unmatched\n");
				if (combo_switch->flags & IGNORE_SWITCHES_UNTIL) {
					dbprintf("ignoring switch due to flag\n");
				}
#endif

				if (recent_index == next_recent_switch) {
#ifdef CONFIG_DEBUG_COMBOS
					dbprintf("wrapped, bailing!\n");
#endif
					break;
				}
			}
		} while (!(matched & MATCHED_SWITCH) && (combo_switch->flags & IGNORE_SWITCHES_UNTIL));
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("match value: %d\n", matched);
#endif

		if (matched != (MATCHED_SWITCH | MATCHED_TIME)) {
			break;
		}
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("matched switch & time\n");
#endif

		if (combo_index == 0) {
#ifdef CONFIG_DEBUG_COMBOS
			dbprintf("combo matched!\n");
#endif
			last_matched_combo = combo;

			callset_pointer_invoke(combo->fn);
		}
	}
}

void process_combos(void) {
	U8 combo_count = sizeof(combo_list) / sizeof(combo_definition_t *);
#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("processing %d combos\n", combo_count);
#endif
	U8 combo_index = 0;
	while (combo_index < combo_count) {
		process_combo(combo_list[combo_index]);
		combo_index++;
	}
}
#endif
