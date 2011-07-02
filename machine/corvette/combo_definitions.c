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

/*
 * TODO Ideally this file should be autogenerated via genmachine.
 *
 * The combo definitions reference game-specific callset_*() methods and switch defines
 *
 * There MUST be a call to combo_reset_current_step_markers() after machine_combos_count has been set via an 'init' callset handler.
 */

#ifdef CONFIG_COMBOS

const combo_step_t cstp_left_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, 0 }
	}
};

const combo_step_t cstp_left_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,    TIME_3S },
		{ SW_MIDDLE_ROLLOVER,  TIME_3S },
		{ SW_RIGHT_ROLLOVER,   TIME_3S }
	}
};


const combo_step_t cstp_right_outer_loop_entry = {
	.flags = CSTP_NO_FLAGS,
	.switches = 1,
	.switch_list = {
		{ SW_RIGHT_OUTER_LOOP, 0 }
	}
};

const combo_step_t cstp_right_outer_loop_exit = {
	.flags = CSTP_NO_FLAGS,
	.switches = 4,
	.switch_list = {
		{ SW_LEFT_OUTER_LOOP, TIME_1S },
		{ SW_LEFT_ROLLOVER,   TIME_3S },
		{ SW_MIDDLE_ROLLOVER, TIME_3S },
		{ SW_RIGHT_ROLLOVER,  TIME_3S }
	}
};

const combo_step_t cstp_wildcard_5sec = {
	.flags = CSTP_WILDCARD,
	.switches = 0,
	.time_allowed = TIME_5S
};

extern void callset_lr_rl_combo_shot(void);
const combo_def_t lr_rl_combo = {
	COMBO_NAME("LR RL")
	.fn = callset_lr_rl_combo_shot,
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
const combo_def_t rl_lr_combo = {
	COMBO_NAME("RL LR")
	.fn = callset_rl_lr_combo_shot,
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

extern void callset_rl_rl_combo_shot(void);
const combo_def_t rl_rl_combo = {
	COMBO_NAME("RL RL")
	.fn = callset_rl_rl_combo_shot,
	.steps = 5,
	.step_list = {
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_exit
	}
};

extern void callset_lr_lr_combo_shot(void);
const combo_def_t lr_lr_combo = {
	COMBO_NAME("LR LR")
	.fn = callset_lr_lr_combo_shot,
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit,
		&cstp_wildcard_5sec,
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_exit
	}
};

extern void callset_ll_rr_combo_shot(void);
const combo_def_t ll_rr_combo = {
	COMBO_NAME("LL RR")
	.fn = callset_ll_rr_combo_shot,
	.steps = 5,
	.step_list = {
		&cstp_left_outer_loop_entry,
		&cstp_left_outer_loop_entry,
		&cstp_wildcard_5sec,
		&cstp_right_outer_loop_entry,
		&cstp_right_outer_loop_entry
	}
};

extern void callset_rr_ll_combo_shot(void);
const combo_def_t rr_ll_combo = {
	COMBO_NAME("RR LL")
	.fn = callset_rr_ll_combo_shot,
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

/*
#define LR_RL_COMBO_ID 0
#define RL_LR_COMBO_ID 1
#define RL_RL_COMBO_ID 2
#define LR_LR_COMBO_ID 3
#define LL_RR_COMBO_ID 4
#define RR_LL_COMBO_ID 5
*/
const combo_def_t *machine_combos[COMBO_COUNT] = {
	&lr_rl_combo,
	&rl_lr_combo,
	&rl_rl_combo,
	&lr_lr_combo,
	&ll_rr_combo,
	&rr_ll_combo
};

U8 current_step_markers[COMBO_COUNT]; // 1-based index
U16 step_time_list[COMBO_COUNT]; // the system timer at the time the last-match step that counted
U16 step_time_allowed_list[COMBO_COUNT]; // the time that was allowed by the last-hit switch or step that counted
U16 wildcard_time_list[COMBO_COUNT]; // the system timer at the time the last wildcard step that counted

CALLSET_ENTRY(machine_combos, init, start_ball) {
	machine_combos_count = COMBO_COUNT;
	combo_reset_current_step_markers();
}
#else
//FIXME genmachine expects these to be defined because it doesn't know about #ifdef CONFIG_COMBOS
__far__(C_STRING(MACHINE_PAGE)) void machine_combos_init_complete(void) {}
#endif