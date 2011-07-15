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

#ifndef _SYS_COMBOS_H
#define _SYS_COMBOS_H

#ifdef CONFIG_COMBOS

typedef void (*combo_handler_t) (void);

/**
 * Combo flags
 */
#define CF_NEVER               0
#define CF_ALWAYS              (1 << 0)
#define CF_SINGLE_BALL_ONLY    (1 << 1)
#define CF_MULTI_BALL_ONLY     (1 << 3)

/**
 * Combo step flags
 */
#define CSTP_NO_FLAGS 0
#define CSTP_WILDCARD (1<<0)

/**
 * Combo IDs
 *
 * Machines should declare defines, one for each combo, end with "_COMBO_ID", e.g. "R66_TO_ZR1_COMBO_ID 0", start numbering at 0.
 */
#define UNKNOWN_COMBO_ID 0xFF

typedef struct combo_switch_s {
	U8 switch_id;
	U16 time_allowed; // takes precedence over the value in combo_step_t that uses this combo_switch_t
} combo_switch_t;

typedef struct combo_step_s {
	U8 flags;
	U8 switches;
	U16 time_allowed;
	combo_switch_t switch_list[];
} combo_step_t;

typedef struct combo_def_s {
#if defined(CONFIG_DEBUG_COMBOS) || defined(CONFIG_UNITTEST)
	const char *name;
#endif
	/** Flags for the combo, see CF_* defines
	 */
	U8 flags;

	/** A function to call when the combo produces an event.
	 * All functions are assumed to be callsets, and located in the
	 * callset page of the ROM. */
	const combo_handler_t fn; // invoked with callset_pointer_invoke(fn);

	const U8 steps;
	const combo_step_t *step_list[];
} combo_def_t;

#define DEFAULT_COMBO \
	.flags = CF_SINGLE_BALL_ONLY, \
	.fn = null_function, \
	.steps = 0


#ifdef CONFIG_UNITTEST
#define UNITTEST_COMBO_ID 0xFF
#endif

#if defined(CONFIG_DEBUG_COMBOS) || defined(CONFIG_UNITTEST)
void dump_combo_step(const combo_step_t *combo_step);
void dump_combo(const combo_def_t *combo);
#define COMBO_NAME(str) .name = str,
#else
#define COMBO_NAME(str)
#endif

extern U8 current_step_markers[];
extern U16 step_time_list[];
extern U16 step_time_allowed_list[];
extern U16 wildcard_time_list[];
extern U16 wildcard_time_allowed_list[];
extern U8 machine_combos_count;
extern const combo_def_t *machine_combos[];
extern const combo_def_t *last_matched_combo;
extern U8 last_matched_combo_id;

extern void combo_process_switch(void);
extern void combo_reset_current_step_markers(void);
extern const combo_def_t *find_combo_with_step_at(const combo_step_t *step_to_match);

#endif

#endif /* _SYS_COMBOS_H */
