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

typedef struct combo_switch_entry_s {
	U8 switch_id;
	U16 max_time_difference;
	U8 flags;
} combo_switch_entry_t;

typedef struct combo_definition_s {
	char *name; // XXX
	U8 flags;
	/** A function to call when the switch pattern produces an event.
	 * All functions are assumed to be callsets, and located in the
	 * callset page of the ROM. */
	combo_handler_t fn; // invoke with callset_pointer_invoke(fn);

	U8 switch_count;
	/** A list of switches and flags that define the sequence.
	 */
	combo_switch_entry_t switch_list[];
} combo_definition_t;

extern void process_combos(void);
#endif

#endif /* _SYS_COMBOS_H */
