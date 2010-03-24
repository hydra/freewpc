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

U8 in_test;
U8 p2k_write_cache[0x20];

void writeb (IOPTR addr, U8 val)
{
	dbprintf ("IO write(%X,%02X)\n", addr, val);
}

U8 readb (IOPTR addr)
{
	dbprintf ("IO read(%X)\n", addr);
	return 0;
}

void linux_shutdown (U8 error_code)
{
	exit (error_code);
}

void platform_init (void)
{
}

void linux_init (void)
{
}

int main (int argc, char *argv[])
{
	in_test = FALSE;
	freewpc_init ();
	return 0;
}

