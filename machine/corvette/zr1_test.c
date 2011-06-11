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

/* ZR1 Test :
 * - Enable/Disable solenoids.
 * - Calibrate the left right and center positions.
 * - Shake engine.
 */
#ifdef ZR1_TEST_ENABLED
#include <freewpc.h>
#include <window.h>
#include <test.h>
#include <corvette/zr1.h>

// FIXME - font_render_string_right vertically offsets the text by 1 pixel (downwards)
// When this is removed removed all references to FR_WORKAROUND
#define FRSR_WORKAROUND 1

enum {
	FIRST_TEST = 0,
	CALIBRATE = FIRST_TEST,
	SHAKE,
	CENTER,
	FLOAT,
	BALL_SEARCH,
	LAST_TEST = BALL_SEARCH
} zr1_test_command;

char *zr1_test_short_names[] = {
	"CALIBRATE",
	"SHAKE",
	"CENTER",
	"FLOAT",
	"SEARCH"
};

// error messages, see enum mech_zr1_calibration_codes;
char *mech_zr1_calibration_messages[] = {
	"NOT CALIBRATED",
	"ZR1 ERROR 1 CHECK F111",
	"CHECK OPTO ZR1 FULL LEFT",
	"CHECK OPTO ZR1 FULL RIGHT",
	"ZR1 ERROR 2 CHECK ENGINE",
	"CALIBRATED O.K."
};

extern __fastram__ enum mech_zr1_state zr1_state;
extern U8 zr1_calibrated;
extern U8 zr1_calibration_attempted;
extern U8 zr1_pos_center;
extern U8 zr1_pos_full_left_opto_off;
extern U8 zr1_pos_full_right_opto_off;
extern __fastram__ U8 zr1_pos_shake_left;
extern __fastram__ U8 zr1_pos_shake_right;
extern __fastram__ U8 zr1_last_position;
extern __fastram__ U8 zr1_shake_speed;
extern U8 zr1_shake_range;
extern enum mech_zr1_calibration_codes zr1_last_calibration_result_code;

void zr1_test_init (void)
{
	zr1_test_command = CALIBRATE;
}

void zr1_draw_test_title(void) {
	font_render_string_center (&font_mono5, 64, 2, "ZR1 ENGINE TEST");
	dmd_draw_horiz_line ((U16 *)dmd_low_buffer, 5);
}

#define LINE_1_Y 7
#define LINE_2_Y 13
#define LINE_3_Y 19

void zr1_test_draw (void)
{
	dmd_alloc_low_clean ();

	zr1_draw_test_title();

	// 21 characters wide max when using 5 point font.

	// P = Current Position, S = State, PWR = Power, C = Center Position
	// e.g. "P:127 S:1 PWR:1 C:127"
	sprintf ("P:%d S:%d PWR:%d C:%d",
		zr1_last_position,
		zr1_state,
		global_flag_test(GLOBAL_FLAG_ZR1_SOLENOIDS_POWERED) ? 1 : 0,
		zr1_pos_center
	);
	font_render_string_center (&font_var5, 64, LINE_1_Y + 2, sprintf_buffer);

	switch (zr1_test_command) {
		case CALIBRATE:
			if (zr1_state == ZR1_CALIBRATE) {
				font_render_string_center(&font_var5, 64, LINE_3_Y + 2, "CALIBRATING");
				sprintf ("L:%c %s%d",
					(switch_poll_logical (SW_ZR1_FULL_LEFT) ? 'X' : '-'),
					zr1_pos_full_left_opto_off > 100 ? "" : (zr1_pos_full_left_opto_off < 10 ?  "00" : "0"), // FIXME %03d doesn't pad with leading zeros..
					zr1_pos_full_left_opto_off
				);
				font_render_string_left (&font_var5, 0, LINE_3_Y, sprintf_buffer);

				sprintf ("R:%c %s%d",
					(switch_poll_logical (SW_ZR1_FULL_RIGHT) ? 'X' : '-'),
					zr1_pos_full_right_opto_off > 100 ? "" : (zr1_pos_full_right_opto_off < 10 ?  "00" : "0"), // FIXME %03d doesn't pad with leading zeros..
					zr1_pos_full_right_opto_off
				);
				font_render_string_right (&font_var5, 0, LINE_3_Y - FRSR_WORKAROUND, sprintf_buffer);
			} else {
				dbprintf("calibration result: %d\n", zr1_last_calibration_result_code);
				font_render_string_center(&font_var5, 64, LINE_3_Y + 2, mech_zr1_calibration_messages[zr1_last_calibration_result_code]);
			}
		break;

		case SHAKE:
			sprintf ("SPD: %s%d, RNG: %d",
				zr1_shake_speed < 10 ? "0" : "", // FIXME %02d doesn't pad with leading zeros..
				zr1_shake_speed,
				zr1_shake_range
			);
			font_render_string_left (&font_var5, 0, LINE_2_Y, sprintf_buffer);

			sprintf ("L:%c R:%c",
				(switch_poll_logical (SW_ZR1_FULL_LEFT) ? 'X' : '-'),
				(switch_poll_logical (SW_ZR1_FULL_RIGHT) ? 'X' : '-')
			);
			font_render_string_right (&font_var5, 0, LINE_2_Y - FRSR_WORKAROUND, sprintf_buffer);

			if (zr1_state == ZR1_SHAKE) {
				sprintf("LSP:%d", zr1_pos_shake_left);
				font_render_string_left (&font_var5, 0, LINE_3_Y, sprintf_buffer);
				sprintf("RSP:%d", zr1_pos_shake_left);
				font_render_string_right (&font_var5, 0, LINE_3_Y - FRSR_WORKAROUND, sprintf_buffer);

				font_render_string_center(&font_var5, 64, LINE_3_Y + 2, "SHAKING");
			} else {
				if (!zr1_calibrated) {
					font_render_string_center(&font_var5, 64, LINE_3_Y + 2, "NOT CALIBRATED");
				} else {
					font_render_string_center(&font_var5, 64, LINE_3_Y + 2, "NOT SHAKING");
				}
			}

		break;
		default:
			// shut the compiler up
		break;
	}

	dmd_draw_horiz_line ((U16 *)dmd_low_buffer, 25);

	sprintf(zr1_test_short_names[zr1_test_command]);
	font_render_string_left (&font_mono5, 0, 27, sprintf_buffer);



	dmd_show_low ();
}


void zr1_test_thread (void)
{
	for (;;)
	{

		task_sleep (TIME_50MS);

		zr1_test_draw ();
	}
}

U8 new_shake_range;

void zr1_test_start (void)
{
	switch (zr1_test_command) {
		case SHAKE:
			new_shake_range = zr1_shake_range + 1;
			if (new_shake_range > ZR1_SHAKE_RANGE_MAX) {
				new_shake_range = ZR1_SHAKE_RANGE_MIN;
			}
			zr1_set_shake_range(new_shake_range);
		break;
		default:
			// shut the compiler up
		break;
	}
}

void zr1_test_left (void)
{
	switch (zr1_test_command) {
		case SHAKE:
			zr1_set_shake_speed(zr1_shake_speed - 1);
		break;
		default:
		// shut the compiler up
		break;
	}
}

void zr1_test_right (void)
{
	switch (zr1_test_command) {
		case SHAKE:
			zr1_set_shake_speed(zr1_shake_speed + 1);
		break;
		default:
		// shut the compiler up
		break;
	}
}



void zr1_test_up (void)
{
	if (zr1_test_command < LAST_TEST)
		zr1_test_command++;
}


void zr1_test_down (void)
{
	if (zr1_test_command > FIRST_TEST)
		zr1_test_command--;
}

void zr1_test_escape (void) {

	// allow calibrate to continue, but stop everything else
	if (zr1_state != ZR1_CALIBRATE) {
		zr1_float();
	}

	window_pop();
}

void zr1_test_enter (void)
{
	sound_send (SND_TEST_ENTER);
  	switch (zr1_test_command)
  	{
  		case CALIBRATE:
 			dbprintf ("zr1_test_enter: starting 'calibrate'\n");
 			zr1_calibrate();
  		break;

  		case SHAKE:
  			if (zr1_state == ZR1_SHAKE) {
 				dbprintf ("zr1_test_enter: engine already shaking, starting 'center' instead\n");
  				zr1_center();
  				break;
  			}
 			dbprintf ("zr1_test_enter: starting 'shake'\n");
  			zr1_shake();
  		break;

  		case CENTER:
 			dbprintf ("zr1_test_enter: starting 'center'\n");
  			zr1_center();
  		break;

  		case FLOAT:
 			dbprintf ("zr1_test_enter: starting 'float'\n");
  			zr1_float();
  		break;

  		case BALL_SEARCH:
  			if (zr1_state == ZR1_BALL_SEARCH) {
 				dbprintf ("zr1_test_enter: engine already shaking, starting 'center' instead\n");
  				zr1_center();
  				break;
  			}
 			dbprintf ("zr1_test_enter: starting 'float'\n");
  			zr1_start_ball_search();
  		break;

  	}

}


struct window_ops corvette_zr1_test_window = {
	DEFAULT_WINDOW,
	.init = zr1_test_init,
	.draw = zr1_test_draw,
	.up = zr1_test_up,
	.down = zr1_test_down,
	.enter = zr1_test_enter,
	.escape = zr1_test_escape,
	.start = zr1_test_start,
	.left = zr1_test_left,
	.right = zr1_test_right,
	.thread = zr1_test_thread,
};


struct menu corvette_zr1_test_item = {
	.name = "ZR1 ENGINE TEST",
	.flags = M_ITEM,
	.var = { .subwindow = { &corvette_zr1_test_window, NULL } },
};
#endif
