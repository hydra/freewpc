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
#include <test.h>
#include <coin.h>

/**
 * \file
 * \brief Handle the service button switches inside the coin door.
 */

/* TODO : coin door may be closed, in which case button presses
ought to generate a warning message */

CALLSET_ENTRY (service, sw_escape)
{
	if (!in_test)
	{
		add_credit ();
		audit_increment (&system_audits.service_credits);
	}
}

CALLSET_ENTRY (service, sw_down)
{
	if (!in_test)
		button_invoke (SW_DOWN, volume_down, TIME_500MS, TIME_100MS);
	else	
		test_down_button ();
}

CALLSET_ENTRY (service, sw_up)
{
	if (!in_test)
		button_invoke (SW_UP, volume_up, TIME_500MS, TIME_100MS);
	else
		test_up_button ();
}


CALLSET_ENTRY (service, sw_coin_door_closed)
{
	/* Be kind and ignore slam tilt switch briefly after the
	coin door is opened/closed */
	event_can_follow (sw_coin_door_closed, sw_slam_tilt, TIME_5S);

	if (switch_poll_logical (SW_COIN_DOOR_CLOSED))
	{
		dbprintf ("Coin door is closed\n");
	}
	else
	{
		dbprintf ("Coin door is open\n");
	}
}

