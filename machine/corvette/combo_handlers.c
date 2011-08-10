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
 * TODO record the timestamp of the last combo made
 * if another combo is made within 5 seconds increase a counter (otherwise reset counter)
 * when counter is 2,3,4,5 etc then trigger appropriate lamp effect.
 */
#include <freewpc.h>
#include <combo_definitions.h>

U8 chained_combos;
U8 combo_chain_ticks_remaining;

CALLSET_ENTRY(combo, start_ball) {
	chained_combos = 0;
	combo_chain_ticks_remaining = 0;
}

/**
 * Award the player 10M, 20M...40M for 2 to 5 chained shots, respectively.
 * Award the player 50M for more than 5 chained shots
 *
 * NOTE it's also possible to exclude shots from this by checking the value of last_matched_combo_id
 *
 */
CALLSET_ENTRY(combo, any_combo_shot) {
	deff_start(DEFF_COMBO_NAME); // XXX
	sound_start (ST_SAMPLE, SND_EXPLOSION_01, SL_2S, PRI_GAME_QUICK2); // XXX

	if (!(last_matched_combo->flags & CF_USE_FOR_SHOT_CHAINING)) {
		return;
	}

	chained_combos++;
	combo_chain_ticks_remaining = 5; // allow 5 seconds for another shot to be made
	if (chained_combos > 5) {
		leff_start(LEFF_FIVE_COMBOS_SHOT);
		score (SC_50M);
	} else if (chained_combos == 5) {
		leff_start(LEFF_FIVE_COMBOS_SHOT);
		score (SC_40M);
	} else if (chained_combos == 4) {
		leff_start(LEFF_FOUR_COMBOS_SHOT);
		score (SC_30M);
	} else if (chained_combos == 3) {
		leff_start(LEFF_THREE_COMBOS_SHOT);
		score (SC_20M);
	} else if (chained_combos == 2) {
		leff_start(LEFF_TWO_COMBOS_SHOT);
		score (SC_10M);
	}
}


CALLSET_ENTRY(combo, idle_every_second) {
	if (unlikely(combo_chain_ticks_remaining > 0)) {
		combo_chain_ticks_remaining--;
		if (unlikely(combo_chain_ticks_remaining == 0)) {
			chained_combos = 0;
		}
	}
}


//
// combos.c game code follows
//

void combo_name_deff (void)
{
	//dmd_sched_transition (&trans_scroll_left);

	U8 count = 16;
	while (--count > 0) {
#if defined(CONFIG_NAMED_COMBOS)
		sprintf("%s", last_matched_combo->name);
#else
		sprintf("CID:%d CH:%d", last_matched_combo_id, chained_combos);
#endif
		dmd_alloc_pair_clean();
		font_render_string_center (&font_fixed10, 64, 8, sprintf_buffer);
		dmd_copy_low_to_high();
		if (count & 1 && chained_combos > 1) {
			sprintf("%d WAY COMBO", chained_combos);
			font_render_string_center (&font_fixed10, 64, 24, sprintf_buffer);
		}
		if (chained_combos >= 3) {
			dmd_copy_low_to_high();
		}
		dmd_show2 ();
		task_sleep (TIME_100MS);
	}

	deff_exit ();
}

extern const combo_step_t cstp_left_outer_loop_entry;
extern const combo_step_t cstp_right_outer_loop_entry;
extern const combo_step_t cstp_route_66_entry;
extern const combo_step_t cstp_skid_pad_entry;
extern const combo_step_t cstp_zr1_entry;
extern const combo_step_t cstp_inner_loop_entry;

CALLSET_ENTRY (combo, lamp_update) {
	// NOTE ensure these lamp updates do not override other lamp updates, such as the lamp arrows for multiball jackpots, locks and dragrace
	lamp_flash_if(LM_RIGHT_OUTER_LOOP_ARROW, find_combo_with_step_at(&cstp_right_outer_loop_entry));
	lamp_flash_if(LM_LEFT_OUTER_LOOP_ARROW, find_combo_with_step_at(&cstp_left_outer_loop_entry));
	if (!global_flag_test(GLOBAL_FLAG_DRAGRACE_ENABLED)) {
		lamp_flash_if(LM_ROUTE_66_ARROW, find_combo_with_step_at(&cstp_route_66_entry));
	}
	lamp_flash_if(LM_SKID_PAD_ARROW, find_combo_with_step_at(&cstp_skid_pad_entry));
	lamp_flash_if(LM_ZR1_RAMP_ARROW, find_combo_with_step_at(&cstp_zr1_entry));
	lamp_flash_if(LM_INNER_LOOP_ARROW, find_combo_with_step_at(&cstp_inner_loop_entry));

}

CALLSET_ENTRY (combo, left_orbit_combo_shot) {
	// TODO sounds, deff
	score (SC_100K);
}

CALLSET_ENTRY (combo, right_orbit_combo_shot) {
	// TODO sounds, deff
	score (SC_100K);
}

CALLSET_ENTRY (combo, zr1_ramp_combo_shot) {
	// TODO sounds, deff
	score (SC_150K);
}

CALLSET_ENTRY (combo, route66_ramp_combo_shot) {
	// TODO sounds, deff
	score (SC_150K);
}

CALLSET_ENTRY (combo, skidpad_ramp_combo_shot) {
	// TODO sounds, deff
	score (SC_200K);
}

CALLSET_ENTRY (combo, inner_loop_combo_shot) {
	// TODO sounds, deff
	score (SC_200K);
}

CALLSET_ENTRY (combo, rl_lr_combo_shot)
{
	// TODO sounds, deff
	score (SC_2M);
}

CALLSET_ENTRY (combo, lr_rl_combo_shot)
{
	// TODO sounds, deff
	score (SC_2M);
}

CALLSET_ENTRY (combo, lr_lr_combo_shot)
{
	// TODO sounds, deff
	score (SC_1M);
}

CALLSET_ENTRY (combo, rl_rl_combo_shot)
{
	// TODO sounds, deff
	score (SC_1M);
}

CALLSET_ENTRY (combo, rr_ll_combo_shot, ll_rr_combo_shot)
{
	// TODO sounds, deff
	score (SC_50M);
}

CALLSET_ENTRY (combo, rl_il_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M); // XXX
}

CALLSET_ENTRY (combo, rl_skidpad_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, route66_ramp_to_zr1_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, zr1_ramp_to_route66_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, zr1_ramp_to_skidpad_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, route66_ramp_to_zr1_ramp_to_skidpad_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_10M);
}

CALLSET_ENTRY (combo, inner_loop_to_inner_loop_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, inner_loop_to_skidpad_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_10M); // XXX
}

CALLSET_ENTRY (combo, zr1_ramp_to_inner_loop_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M); // XXX
}

CALLSET_ENTRY (combo, skidpad_ramp_to_zr1_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, left_orbit_to_zr1_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, right_orbit_to_route66_ramp_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, skidpad_ramp_to_left_orbit_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}

CALLSET_ENTRY (combo, route66_ramp_to_left_orbit_combo_shot)
{
	// TODO sounds, deff
	score (SC_5M);
}
