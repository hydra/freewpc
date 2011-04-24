/*
 * Copyright 2007, 2008 by Brian Dominy <brian@oddchange.com>
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
#include <simulation.h>
#undef sprintf

/**
 * \file
 * \brief Implements protected memory for native mode.
 * 
 * Protected memory variables can be detected because they reside in a special
 * section of the output file (in much the same way that the 6809 compile does
 * it).  The entire block of RAM can be read from/written to a file to
 * provide persistence.
 */

extern char *__start_protected, *__stop_protected;
extern char *__start_nvram, *__stop_nvram;
extern char *__start_local, *__stop_local;


/** The name of the backing file */
char protected_memory_file[256] = "nvram/default.nv";


/** Load the contents of the protected memory from file to RAM. */
void protected_memory_load (void)
{
	int size = (int)&__stop_nvram - (int)&__start_nvram;
	FILE *fp;

	/* Use a different file for each machine */
	sprintf (protected_memory_file, "nvram/%s.nv", MACHINE_SHORTNAME);

	simlog (SLC_DEBUG, "Loading protected memory from '%s'", protected_memory_file);
	fp = fopen (protected_memory_file, "r");
	if (fp)
	{
		fread (&__start_nvram, 1, size, fp);
		fclose (fp);
	}
	else
	{
		simlog (SLC_DEBUG, "Error loading memory, using defaults\n");
		memset (&__start_nvram, 0, size);
	}
}


/** Save the contents of the protected memory from RAM to a file. */
void protected_memory_save (void)
{
	int size = (int)&__stop_nvram - (int)&__start_nvram;
	FILE *fp;

	simlog (SLC_DEBUG, "Saving 0x%X bytes of protected memory to %s", size, protected_memory_file);
	fp = fopen (protected_memory_file, "w");
	if (fp)
	{
		if (fwrite (&__start_nvram, 1, size, fp) < size)
		{
			simlog (SLC_DEBUG, "Warning: could not save all of memory\n");
			task_sleep_sec (1);
		}
		fclose (fp);
	}
	else
	{
		simlog (SLC_DEBUG, "Warning: could not write to memory file\n");
		task_sleep_sec (1);
	}
}


