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
#include <zr1_low_rev_gate.h>

static void zr1_up_rev_gate_enable_task (void) {
	zr1_up_rev_gate_start ();
	while (global_flag_test (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED)) {
		task_sleep_sec (2);
		zr1_up_rev_gate_restart ();
	}
	zr1_up_rev_gate_stop ();
	task_exit ();
}

void zr1_up_rev_gate_enable (void) {
	task_recreate_gid (GID_ZR1_UP_REV_GATE_OPEN, zr1_up_rev_gate_enable_task);
}

void zr1_up_rev_gate_disable (void) {
	zr1_up_rev_gate_stop ();
	task_kill_gid (GID_ZR1_UP_REV_GATE_OPEN);
}

static void zr1_low_rev_gate_enable_task (void) {
	zr1_low_rev_gate_start ();
	while (global_flag_test (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED)) {
		task_sleep_sec (2);
		zr1_low_rev_gate_restart();
	}
	zr1_low_rev_gate_stop ();
	task_exit ();
}

void zr1_low_rev_gate_enable (void) {
	task_recreate_gid (GID_ZR1_LOW_REV_GATE_OPEN, zr1_low_rev_gate_enable_task);
}

void zr1_low_rev_gate_disable (void) {
	zr1_low_rev_gate_stop ();
	task_kill_gid (GID_ZR1_LOW_REV_GATE_OPEN);
}


static void loop_gate_enable_task (void) {
	loop_gate_start ();
	while (global_flag_test (GLOBAL_FLAG_LOOP_GATE_ENABLED)) {
		task_sleep_sec (2);
		loop_gate_restart();
	}
	loop_gate_stop ();
	task_exit ();
}

void loop_gate_enable (void)
{
	task_recreate_gid (GID_LOOP_GATE_OPEN, loop_gate_enable_task);
}

void loop_gate_disable (void)
{
	loop_gate_stop ();
	task_kill_gid (GID_LOOP_GATE_OPEN);
}

CALLSET_ENTRY (simple, device_update) {
	if (global_flag_test (GLOBAL_FLAG_LOOP_GATE_ENABLED)) {
		if (!task_find_gid(GID_LOOP_GATE_OPEN)) {
			loop_gate_enable ();
		}
	} else {
		loop_gate_disable ();
	}

	if (global_flag_test (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED)) {
		if (!task_find_gid(GID_ZR1_UP_REV_GATE_OPEN)) {
			zr1_up_rev_gate_enable ();
		}
	} else {
		zr1_up_rev_gate_disable ();
	}

	if (global_flag_test (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED)) {
		if (!task_find_gid(GID_ZR1_LOW_REV_GATE_OPEN)) {
			zr1_low_rev_gate_enable ();
		}
	} else {
		zr1_low_rev_gate_disable ();
	}

	if (global_flag_test (GLOBAL_FLAG_DIVERTER_ENABLED)) {
		diverter_start();
	} else {
		diverter_stop();
	}
}

CALLSET_ENTRY (simple, start_ball) {
	global_flag_on (GLOBAL_FLAG_LOOP_GATE_ENABLED);
	global_flag_off (GLOBAL_FLAG_DIVERTER_ENABLED);
}

CALLSET_ENTRY (simple, end_ball, tilt) {
	global_flag_off (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED);
	global_flag_off (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED);
	global_flag_off (GLOBAL_FLAG_LOOP_GATE_ENABLED);
	global_flag_off (GLOBAL_FLAG_DIVERTER_ENABLED);
}


//
// XXX remove this when gate testing is completed
//
#ifdef CONFIG_GATE_TESTS_ENABLED
C_ALLSET_ENTRY(gate_test, sw_left_standup_1) {
	if (global_flag_test (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED)) {
		global_flag_off (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED);
	} else {
		global_flag_on (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED);
	}
}

C_ALLSET_ENTRY(gate_test, sw_left_standup_2) {
	if (global_flag_test (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED)) {
		global_flag_off (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED);
	} else {
		global_flag_on (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED);
	}
}

C_ALLSET_ENTRY(gate_test, sw_left_standup_3) {
	if (global_flag_test (GLOBAL_FLAG_LOOP_GATE_ENABLED)) {
		global_flag_off (GLOBAL_FLAG_LOOP_GATE_ENABLED);
	} else {
		global_flag_on (GLOBAL_FLAG_LOOP_GATE_ENABLED);
	}
}

C_ALLSET_ENTRY(gate_test, lamp_update) {
	lamp_on_if(LM_LEFT_STANDUP_1, global_flag_test (GLOBAL_FLAG_ZR1_LOW_REV_GATE_ENABLED));
	lamp_on_if(LM_LEFT_STANDUP_2, global_flag_test (GLOBAL_FLAG_ZR1_UP_REV_GATE_ENABLED));
	lamp_on_if(LM_LEFT_STANDUP_3, global_flag_test (GLOBAL_FLAG_LOOP_GATE_ENABLED));
}
#endif
