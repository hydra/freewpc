/*
 * Copyright 2010 by Dominic Clifton <me@dominicclifton.name>
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

CALLSET_ENTRY (trivial, any_jet)
{
	score (SC_1K);
}

CALLSET_ENTRY (trivial, sw_left_jet)
{
	sample_start (SND_DITTY_01, SL_500MS);
	callset_invoke (any_jet);
}

CALLSET_ENTRY (trivial, sw_upper_jet)
{
	sample_start (SND_DITTY_02, SL_500MS);
	callset_invoke (any_jet);
}

CALLSET_ENTRY (trivial, sw_lower_jet)
{
	sample_start (SND_DITTY_03, SL_500MS);
	callset_invoke (any_jet);
}

CALLSET_ENTRY (trivial, sw_left_return_lane)
{
	score (SC_10K);
	sample_start (SND_TRAFFIC_HORN_01, SL_500MS);
}

CALLSET_ENTRY (trivial, sw_right_return_lane)
{
	score (SC_10K);
	sample_start (SND_BEEP, SL_500MS);
}

void common_outlane (void)
{
	score (SC_5K);
	sample_start (SND_OUT_LANE, SL_500MS);
}

CALLSET_ENTRY (trivial, sw_left_out_lane)
{
	common_outlane ();
}

CALLSET_ENTRY (trivial, sw_right_out_lane)
{
	common_outlane ();
}

CALLSET_ENTRY (trivial, sw_shooter)
{
	if (in_live_game && !switch_poll_logical (SW_SHOOTER))
	{
		sample_start (SND_BURNOUT_01, SL_2S);
		// TODO ? leff_start (LEFF_SHOOTER);
	}
}

CALLSET_ENTRY (trivial, end_ball, start_ball)
{
}

CALLSET_ENTRY (trivial, start_player)
{
}

// FIXME replay code copied from tz but with an adjusted range

static inline U8 decimal_to_bcd_byte (U8 decimal)
{
#ifdef __m6809__
	U8 quot, rem;
	DIV10 (decimal, quot, rem);
	return (quot << 4) + rem;
#else
	return ((decimal / 10) << 4) + (decimal % 10);
#endif
}

/**
 * @See MACHINE_REPLAY_CODE_TO_SCORE
 * @See format.c#replay_score_render()
 */
void replay_code_to_score (score_t s, U8 val)
{
	// ranges from 500M to 1B
	s[0] = decimal_to_bcd_byte (4 + val);
}
