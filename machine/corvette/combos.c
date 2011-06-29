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

//
// TODO from mach-combos.c
//

static combo_step_t cstp_left_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, 0 }
	}
};

static combo_step_t cstp_left_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,    TIME_3S },
		{ SW_MIDDLE_ROLLOVER,  TIME_3S },
		{ SW_RIGHT_ROLLOVER,   TIME_3S }
	}
};


static combo_step_t cstp_right_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, 0 }
	}
};

static combo_step_t cstp_right_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,   TIME_3S },
		{ SW_MIDDLE_ROLLOVER, TIME_3S },
		{ SW_RIGHT_ROLLOVER,  TIME_3S }
	}
};

static combo_step_t cstp_wildcard_5sec = {
	.flags = CSTP_WILDCARD,
	.switches = 0,
	.time_allowed = TIME_5S
};

extern void callset_lr_rl_combo_shot(void);
static combo_def_t lr_rl_combo = {
	.fn = callset_lr_rl_combo_shot,
	.name = "LR RL",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

extern void callset_rl_lr_combo_shot(void);
static combo_def_t rl_lr_combo = {
	.fn = callset_rl_lr_combo_shot,
	.name = "RL LR",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

static combo_def_t rl_rl_combo = {
	.fn = null_function,
	.name = "RL RL",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

static combo_def_t lr_lr_combo = {
	.fn = null_function,
	.name = "LR LR",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

static combo_def_t ll_rr_combo = {
	.fn = null_function,
	.name = "LL RR",
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_entry,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_entry
	}
};

static combo_def_t rr_ll_combo = {
	.fn = null_function,
	.name = "RR LL",
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_entry,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_entry
	}
};

#define COMBO_COUNT 6

#define LR_RL_COMBO_ID 0
#define RL_LR_COMBO_ID 1
#define RL_RL_COMBO_ID 2
#define LR_LR_COMBO_ID 3
#define LL_RR_COMBO_ID 4
#define RR_LL_COMBO_ID 5

combo_def_t *machine_combos[COMBO_COUNT] = {
	&lr_rl_combo,
	&rl_lr_combo,
	&rl_rl_combo,
	&lr_lr_combo,
	&ll_rr_combo,
	&rr_ll_combo
};

U8 machine_combos_count;
U8 current_step_markers[COMBO_COUNT]; // 1-based index
U16 step_time_list[COMBO_COUNT]; // the system timer at the time the last-match step that counted
U16 step_time_allowed_list[COMBO_COUNT]; // the time that was allowed by the last-hit switch or step that counted

void combo_reset_current_step_markers(void) {
	last_matched_combo = 0;
	memset(current_step_markers, 0x00, sizeof(current_step_markers));
	memset(step_time_list, 0x00, sizeof(step_time_list));
	memset(step_time_allowed_list, 0x00, sizeof(step_time_allowed_list));
}

CALLSET_ENTRY(machine_combos, init_complete, start_ball) {
	machine_combos_count = COMBO_COUNT;
	combo_reset_current_step_markers();
}


//
// combos.c game code follows
//

void combo_name_deff (void)
{
	sprintf("%s", last_matched_combo->name);
	flash_and_exit_deff (20, TIME_100MS);
}

CALLSET_ENTRY (combo, rl_lr_combo_shot)
{
	dbprintf("rl_lr_combo handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_10M); // XXX
}

CALLSET_ENTRY (combo, lr_rl_combo_shot)
{
	dbprintf("lr_rl_combo handler invoked\n");
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_10M); // XXX
}

#else
//FIXME genmachine expects these to be defined because it doesn't know about #ifdef CONFIG_COMBOS
__far__(C_STRING(MACHINE_PAGE)) void machine_combos_init_complete(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_name_deff(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_lr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lr_rl_combo_shot(void) {}
#endif
