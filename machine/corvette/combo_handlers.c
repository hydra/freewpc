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
// combos.c game code follows
//

void combo_name_deff (void)
{
#if defined(CONFIG_DEBUG_COMBOS)
	sprintf("%s", last_matched_combo->name);
#else
	sprintf("Combo: %ld", last_matched_combo);
#endif
	flash_and_exit_deff (20, TIME_100MS);
}

CALLSET_ENTRY (combo, rl_lr_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_25M); // XXX
}

CALLSET_ENTRY (combo, lr_rl_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_25M); // XXX
}

CALLSET_ENTRY (combo, lr_lr_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_10M); // XXX
}

CALLSET_ENTRY (combo, rl_rl_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_10M); // XXX
}

CALLSET_ENTRY (combo, rr_ll_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_50M); // XXX
}

CALLSET_ENTRY (combo, ll_rr_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_50M); // XXX
}

CALLSET_ENTRY (combo, rl_il_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_25M); // XXX
}

#else
//FIXME genmachine expects these to be defined because it doesn't know about #ifdef CONFIG_COMBOS
//__far__(C_STRING(MACHINE_PAGE)) void machine_combos_init_complete(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_name_deff(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_lr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lr_rl_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lr_lr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_rl_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rr_ll_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_ll_rr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_il_combo_shot(void) {}
#endif
