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
#include <loop_gate.h>
#include <diverter.h>
#include <zr1_up_rev_gate.h>

CALLSET_ENTRY (simple, device_update) {
	if (global_flag_test (GLOBAL_FLAG_LOOP_GATE_OPENED)) {
		loop_gate_start ();
	} else {
		loop_gate_stop ();
	}

	if (global_flag_test (GLOBAL_FLAG_ZR1_UP_REV_GATE_OPENED)) {
		zr1_up_rev_gate_start ();
	} else {
		zr1_up_rev_gate_stop ();
	}

	if (global_flag_test (GLOBAL_FLAG_DIVERTER_OPENED)) {
		diverter_start();
	} else {
		diverter_stop();
	}
}

CALLSET_ENTRY (simple, start_ball) {
	global_flag_on (GLOBAL_FLAG_LOOP_GATE_OPENED);
	global_flag_off (GLOBAL_FLAG_DIVERTER_OPENED);
}

CALLSET_ENTRY (simple, end_ball, tilt) {
	global_flag_off (GLOBAL_FLAG_ZR1_UP_REV_GATE_OPENED);
	global_flag_off (GLOBAL_FLAG_LOOP_GATE_OPENED);
	global_flag_off (GLOBAL_FLAG_DIVERTER_OPENED);
}

CALLSET_ENTRY (simple, extra_ball_award) {
	music_effect_start (SND_DITTY_06, SL_4S);
}
