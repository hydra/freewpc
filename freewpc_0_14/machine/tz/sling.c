/*
 * Copyright 2006, 2007 by Brian Dominy <brian@oddchange.com>
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
#include <rtsol.h>

__fastram__ S8 rtsol_left_sling;
__fastram__ S8 rtsol_right_sling;


void slingshot_rtt (void)
{
	if (in_live_game)
	{
		rt_solenoid_update (&rtsol_left_sling,
			SOL_LEFT_SLING, SW_LEFT_SLING, 8, 8);
	
		rt_solenoid_update (&rtsol_right_sling,
			SOL_RIGHT_SLING, SW_RIGHT_SLING, 8, 8);
	}
}


CALLSET_ENTRY (sling, sw_sling)
{
	score (SC_10);
	sound_send (SND_SLINGSHOT);
}

