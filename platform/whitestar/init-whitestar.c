/*
 * Copyright 2010 by Brian Dominy <brian@oddchange.com>
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

#ifdef CONFIG_BARE
__fastram__ U16 sys_time;
U16 sched_test_count;
void fatal (U8 err_code) {}

void freewpc_init (void)
{
	VOIDCALL (platform_init);
	task_init ();
	for (;;)
	{
		pinio_write_lamp_strobe (0x1);
		pinio_write_lamp_data (0xFF);
	}
}

__local__ U8 tmp_local;
__permanent__ U8 tmp_permanent;
__nvram__ U8 tmp_nvram;
void dbprintf1 (void) {}
char sprintf_buffer[PRINTF_BUFFER_SIZE];
void do_sprintf_hex_byte () {}
void log_event1 (U16 module_event, U8 arg) {}
#endif

__fastram__ U8 ws_page_led_io;


/**
 * Initialize the Whitestar platform.
 */
void platform_init (void)
{
	/* Reset the sound board... the earlier the better */
	pinio_reset_sound ();

#ifdef __m6809__
	/* Install the null pointer catcher, by programming
	 * some SWI instructions at zero. */
	*(U8 *)0 = 0x3F;
	*(U8 *)1 = 0x3F;
#endif /* __m6809__ */

	/* Initialize the ROM page register
	 * page of ROM adjacent to the system area is mapped.
	 * This is the default location for machine-specific files. */
	pinio_set_bank (PINIO_BANK_ROM, MACHINE_PAGE);
}


void ws_lamp_test (void)
{
	for (;;)
	{
		lamp_all_on ();
		task_sleep (TIME_100MS);
		lamp_all_off ();
		task_sleep (TIME_100MS);
	}
}

CALLSET_ENTRY (whitestar, amode_start)
{
	task_create_anon (ws_lamp_test);
}

