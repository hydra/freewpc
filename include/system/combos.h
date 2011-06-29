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

#define CSTP_NO_FLAGS 0
#define CSTP_WILDCARD (1<<0)

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
	char *name;
	/** A function to call when the switch pattern produces an event.
	 * All functions are assumed to be callsets, and located in the
	 * callset page of the ROM. */
	combo_handler_t fn; // invoked with callset_pointer_invoke(fn);

	U8 steps;
	combo_step_t *step_list[];
} combo_def_t;


#ifdef CONFIG_UNITTEST
#define UNITTEST_COMBO_ID 0xFF
#endif

#if defined(CONFIG_DEBUG_COMBOS) || defined(CONFIG_UNITTEST)
void dump_combo_step(const combo_step_t *combo_step);
void dump_combo(const combo_def_t *combo);
#endif

extern U8 current_step_markers[];
extern U16 step_time_list[];
extern U16 step_time_allowed_list[];
extern U8 machine_combos_count;
extern combo_def_t *machine_combos[];
extern const combo_def_t *last_matched_combo;

extern void combo_process_switch(void);

#endif

#endif /* _SYS_COMBOS_H */
