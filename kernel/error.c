/*
 * Copyright 2006, 2007, 2008, 2009, 2010 by Brian Dominy <brian@oddchange.com>
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

/** Indicates the last nonfatal error taken */
U8 last_nonfatal_error_code;

/** Indicate the task that was running when the last nonfatal happened */
task_gid_t last_nonfatal_error_gid;

__permanent__ bool new_fatal_error;

/**
 * The lockup check routine examines 'task_dispatching_ok', which
 * should normally be true as normal task scheduling occurs.
 * If this value stays false, something is very wrong.
 *
 * This check occurs every 128 IRQs.  No task should run for
 * that long without giving up control.  If the flag stays false
 * between 2 consecutive calls, we invoke a fatal error and reset.
 * This implies the same task was running for between 125 and 250ms.
 *
 * NOTE: if a task _really_ does take that long to execute before
 * switching out, it should set "task_dispatching_ok = TRUE"
 * periodically, to avoid a time out here.  This should rarely be
 * used.  (Even better, it should "task_yield" and let other tasks
 * run for a while.)
 */
void lockup_check_rtt (void)
{
#ifndef CONFIG_NATIVE
	if (sys_init_complete && !task_dispatching_ok)
	{
		audit_increment (&system_audits.exec_lockups);
		fatal (ERR_TASK_LOCKUP);
	}
	else
	{
		task_dispatching_ok = FALSE;
	}
#endif
}


/**
 * Entry point for errors that are nonrecoverable.
 * error_code is one of the values in include/system/errno.h.
 */
__noreturn__
void fatal (errcode_t error_code)
{
	new_fatal_error = TRUE;
#ifdef __m6809__
	set_stack_pointer (6133);
#endif

	/* Don't allow any more interrupts, since they might be the
	source of the error.  Since FIRQ is disabled, we can only
	do mono display at this point.   Also note that idle
	cannot run anymore, because task scheduling can't proceed
	without the system clock moving. */
	periodic_ok = 0;
	disable_interrupts ();

	/* Reset hardware outputs */
#ifdef CONFIG_GI
	pinio_write_gi (0);
#endif

	/* TODO - this whole function needs porting to Whitestar */
#ifdef CONFIG_PLATFORM_WPC
	if (WPC_HAS_CAP (WPC_CAP_FLIPTRONIC))
		wpc_write_flippers (0);
#endif
	pinio_write_ticket (0);
	pinio_write_solenoid_set (0, 0);
	pinio_write_solenoid_set (1, 0);
	pinio_write_solenoid_set (2, 0);
	pinio_write_solenoid_set (3, 0);
#ifdef MACHINE_SOL_EXTBOARD1
	pinio_write_solenoid_set (5, 0);
#endif

	/* Audit the error. */
	audit_increment (&system_audits.fatal_errors);
	audit_assign (&system_audits.lockup1_addr, error_code);
	audit_assign (&system_audits.lockup1_pid_lef, task_getgid ());
	log_event (SEV_ERROR, MOD_SYSTEM, EV_SYSTEM_FATAL, error_code);

	/* Dump all of the task information to the debugger port. */
#ifdef DEBUGGER
	dbprintf ("Fatal error %d\n", error_code);
	db_dump_all ();
#endif

	/* In simulation, exit whenever a fatal occurs. */
#ifdef CONFIG_NATIVE
	sim_exit (error_code);
#endif

	/* Defining STOP_ON_ERROR is helpful during debugging a problem.
	Having the machine reset makes it hard to debug after the fact. */
#ifdef STOP_ON_ERROR
	while (1);
#endif

	/* Restart the system */
	warm_reboot ();
}


/**
 * Entry point for errors that are recoverable.
 * error_code is one of the values in include/system/errno.h.
 * This function simply logs the error, but the system continues on.
 */
void nonfatal (errcode_t error_code)
{
	audit_increment (&system_audits.non_fatal_errors);
#ifdef DEBUGGER
	last_nonfatal_error_code = error_code;
	last_nonfatal_error_gid = task_getgid ();
	if (!in_test)
		deff_start (DEFF_NONFATAL_ERROR);
	dbprintf ("Nonfatal error %d\n", error_code);
#endif
	log_event (SEV_ERROR, MOD_SYSTEM, EV_SYSTEM_NONFATAL, error_code);
}


#ifdef PARANOID
void process_check_rtt (void)
{
#ifdef __m6809__
	if (task_getpid () >= (task_t *)0x2000)
		fatal (ERR_TASK_LOCKUP);

	if (task_getgid () >= 0x80)
		fatal (ERR_TASK_LOCKUP);
#endif
}
#endif


CALLSET_ENTRY (error, init_complete)
{
#ifdef CONFIG_BPT
	if (new_fatal_error)
	{
		bpt_stop ();
		new_fatal_error = FALSE;
	}
#endif
}

