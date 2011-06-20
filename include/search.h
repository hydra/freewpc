/*
 * Copyright 2006, 2007, 2008, 2009 by Brian Dominy <brian@oddchange.com>
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

#ifndef _SEARCH_H
#define _SEARCH_H

extern U8 ball_search_count;

__common__ void ball_search_timer_reset (void);
__common__ bool ball_search_timed_out (void);
__common__ void ball_search_timeout_set (U8 secs);
__common__ void ball_search_monitor_start (void);
__common__ void ball_search_monitor_stop (void);
__common__ void ball_search_run (void);
__common__ void ball_search_now (void);

#ifdef MACHINE_BALL_SEARCH_TIME
#define BS_TIMEOUT_DEFAULT MACHINE_BALL_SEARCH_TIME
#else
#define BS_TIMEOUT_DEFAULT	15
#endif

#endif /* _SEARCH_H */
