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
 * TODO add a idle task that checks the step markers of all combos and lights appropriate arrows
 */
#include <freewpc.h>
#include <combo_definitions.h>

#ifdef CONFIG_COMBOS

//
// combos.c game code follows
//

void combo_name_deff (void)
{
#if defined(CONFIG_DEBUG_COMBOS)
	sprintf("%s", last_matched_combo->name);
#else
	sprintf("Combo: %d", last_matched_combo_id);
#endif
	flash_and_exit_deff (20, TIME_100MS);
}

const combo_def_t *find_combo_with_step_at(const combo_step_t *step_to_match) {

	U8 combo_id;
	const combo_def_t *combo;
	const combo_step_t *next_step;
	U8 wildcard_marker;

	//dbprintf("looking for step: %ld\n", step_to_match);
	for (combo_id = 0; combo_id < COMBO_COUNT; combo_id++) {
		//dbprintf("combo: %d, sm: %d\n", combo_id, current_step_markers[combo_id]);
		if (current_step_markers[combo_id] == 0) {
			// skip the first step otherwise lamps would be flashing all the time...
			continue;
		}

		combo = machine_combos[combo_id];

		if (!(
				(combo->flags & CF_ALWAYS) ||
				((combo->flags & CF_SINGLE_BALL_ONLY) && single_ball_play()) ||
				((combo->flags & CF_MULTI_BALL_ONLY) && !single_ball_play())
		)) {
			return NULL;
		}

		next_step = combo->step_list[current_step_markers[combo_id]];
		if (next_step == step_to_match) {
			//dbprintf("matched (1)\n");
			return combo;
		}

		// find the previous wildcard step (including the current step)
		wildcard_marker = current_step_markers[combo_id] + 1;
		do {
			if (combo->step_list[wildcard_marker - 1]->switches == 0) {
				break;
			}
			wildcard_marker--;
		} while (wildcard_marker != 0);
		//dbprintf("wcm: %d\n", wildcard_marker);
		// see the next step following the wildcard is the step we're after
		if (wildcard_marker > 0 && combo->step_list[wildcard_marker] == step_to_match) {
			//dbprintf("matched (2)\n");
			return combo;
		}
	}
	return NULL;
}

extern const combo_step_t cstp_left_outer_loop_entry;
extern const combo_step_t cstp_right_outer_loop_entry;
extern const combo_step_t cstp_route_66_entry;
extern const combo_step_t cstp_skid_pad_entry;
extern const combo_step_t cstp_zr1_entry;
extern const combo_step_t cstp_inner_loop_entry;

// FIXME these lamp updates override other lamp updates, such as the lamp arrows for multiball jackpots, locks and dragrace
CALLSET_ENTRY (combo, lamp_update) {
	lamp_flash_if(LM_RIGHT_OUTER_LOOP_ARROW, find_combo_with_step_at(&cstp_right_outer_loop_entry));
	lamp_flash_if(LM_LEFT_OUTER_LOOP_ARROW, find_combo_with_step_at(&cstp_left_outer_loop_entry));
	lamp_flash_if(LM_ROUTE_66_ARROW, find_combo_with_step_at(&cstp_route_66_entry));
	lamp_flash_if(LM_SKID_PAD_ARROW, find_combo_with_step_at(&cstp_skid_pad_entry));
	lamp_flash_if(LM_ZR1_RAMP_ARROW, find_combo_with_step_at(&cstp_zr1_entry));
	lamp_flash_if(LM_INNER_LOOP_ARROW, find_combo_with_step_at(&cstp_inner_loop_entry));
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

CALLSET_ENTRY (combo, rl_skidpad_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_25M); // XXX
}

CALLSET_ENTRY (combo, route66_ramp_to_zr1_ramp_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_15M); // XXX
}

CALLSET_ENTRY (combo, zr1_ramp_to_route66_ramp_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_15M); // XXX
}

CALLSET_ENTRY (combo, zr1_ramp_to_skidpad_ramp_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_20M); // XXX
}

CALLSET_ENTRY (combo, route66_ramp_to_zr1_ramp_to_skidpad_ramp_combo_shot)
{
	// TODO sounds, deff, scoring
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX
	score (SC_40M); // XXX
}

#else
//FIXME genmachine expects these to be defined because it doesn't know about #ifdef CONFIG_COMBOS
//__far__(C_STRING(MACHINE_PAGE)) void machine_combos_init_complete(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_name_deff(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lamp_update(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_lr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lr_rl_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_lr_lr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_rl_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rr_ll_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_ll_rr_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_il_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_rl_skidpad_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_route66_ramp_to_zr1_ramp_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_zr1_ramp_to_route66_ramp_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_zr1_ramp_to_skidpad_ramp_combo_shot(void) {}
__far__(C_STRING(MACHINE_PAGE)) void combo_route66_ramp_to_zr1_ramp_to_skidpad_ramp_combo_shot(void) {}
#endif
