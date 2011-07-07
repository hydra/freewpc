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

/* ballsave.c */
__common__ void ballsave_disable(void);

/* search.c */
extern __far__(C_STRING(COMMON_PAGE)) void ball_search_monitor_start (void);
extern __far__(C_STRING(COMMON_PAGE)) void ball_search_monitor_stop (void);

/* deffs.c */
__machine__ void flash_and_exit_deff(U8 flash_count, task_ticks_t flash_delay);
__machine__ void printf_millions(U8 n);
__machine__ void printf_thousands(U8 n);
__machine__ void replay_deff(void);
__machine__ void extra_ball_deff(void);
__machine__ void special_deff(void);
__machine__ void jackpot_deff(void);
__machine__ void ballsave_deff(void);
__machine__ void text_color_flash_deff (void);
__machine__ void spell_test_deff (void);
__machine__ void two_color_flash_deff (void);
__machine__ void bg_flash_deff (void);
__machine__ void dmd_flash (task_ticks_t delay);

/* zr1.c */

__machine__ void zr1_shake(void);
__machine__ void zr1_center(void);
__machine__ void zr1_float(void);
__machine__ void zr1_calibrate(void);
__machine__ void zr1_start_ball_search(void);
__machine__ void zr1_set_shake_speed(U8 new_shake_speed);
__machine__ void zr1_set_shake_range(U8 new_shake_range);
__machine__ bool zr1_mb_can_award_lite_lock(void);
__machine__ void zr1_mb_award_lite_lock(void);

/* racetrack.c */
void racetrack_float(void);
void racetrack_calibrate(void);
void racetrack_race(void);
void racetrack_car_return(void);
void racetrack_car_test(void);
U8 racetrack_get_actual_car_position(U8 lane_number);
void racetrack_set_desired_car_position(U8 lane, U8 position_percentage);


/* vmode_dragrace.c */
void dragrace_start(U8 starter_gid);
void dragrace_disable( void );
void dragrace_enable( void );

/* goal_collect_cars.c */
void award_car(void);
bool have_collected_all_cars(void);
