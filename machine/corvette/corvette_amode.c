/*
 * Copyright 2011 by Dominic Clifton
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

/* CALLSET_SECTION (corvette_amode, __machine2__) */
#include <freewpc.h>

// TODO have a bunch of corvette car images and each time the amode comes round to display a car use a different one.

/**
 * Scroll two cars, one after the other, from left to right.
 *
 * Note: Car graphics use a white background.
 */
void corvette_amode_scroll_two_cars( void ) {

	dmd_alloc_pair();
	dmd_clean_page_high();
	dmd_clean_page_low();
	dmd_invert_page (dmd_low_buffer);
	dmd_invert_page (dmd_high_buffer);
	dmd_show2();
	task_sleep_sec(1);

	dmd_alloc_pair();
	frame_draw (IMG_CORVETTE_1979);
	dmd_sched_transition (&trans_scroll_right);
	dmd_show2 ();

	dmd_alloc_pair();
	dmd_clean_page_high();
	dmd_clean_page_low();
	dmd_invert_page (dmd_low_buffer);
	dmd_invert_page (dmd_high_buffer);
	dmd_sched_transition (&trans_scroll_right);
	dmd_show2();
	task_sleep_sec(1);

	dmd_alloc_pair();
	frame_draw (IMG_CORVETTE_1986);
	dmd_sched_transition (&trans_scroll_right);
	dmd_show2 ();

	dmd_alloc_pair();
	dmd_clean_page_high();
	dmd_clean_page_low();
	dmd_invert_page (dmd_low_buffer);
	dmd_invert_page (dmd_high_buffer);
	dmd_sched_transition (&trans_scroll_right);
	dmd_show2();
	task_sleep_sec(1);


}

CALLSET_ENTRY (corvette_amode, amode_page)
{
	dbprintf("corvette_amode: amode_page - start\n");

#ifdef CONFIG_UKPINBALLPARTY2011
	dmd_alloc_pair ();
	dmd_clean_page_low ();
	font_render_string_center (&font_fixed6, 64, 7, "UK PINBALL");
	font_render_string_center (&font_fixed6, 64, 20, "PARTY 2011");
	dmd_show_low ();
	task_sleep_sec (2);

	dmd_alloc_pair ();
	dmd_clean_page_low ();
	font_render_string_center (&font_fixed6, 64, 7, "WRITE IDEAS");
	font_render_string_center (&font_fixed6, 64, 20, "IN THE BOOK");
	dmd_show_low ();
	task_sleep_sec (2);
#endif

	corvette_amode_scroll_two_cars();

	dbprintf("corvette_amode: amode_page - exit\n");
}


