
#include <stdlib.h>
#include <freewpc.h>
#include <simulation.h>
#include "/usr/include/sys/time.h"


/** The frequency of the realtime loop, in millseconds */
#define RT_FREQ 16


unsigned long realtime_counter;

unsigned long realtime_read (void)
{
	return realtime_counter;
}

/** True if the IRQ is enabled */
bool linux_irq_enable;

/** Nonzero if an IRQ is pending */
int linux_irq_pending;

/** True if the FIRQ is enabled */
bool linux_firq_enable;



/**
 * Implement a realtime loop on a non-realtime OS.
 */
void realtime_loop (void)
{
	struct timeval prev_time, curr_time;
	int usecs_elapsed = 0;
#ifdef CONFIG_DEBUG_LATENCY
	int latency;
#endif

	gettimeofday (&prev_time, NULL);
	for (;;)
	{
		/* Sleep before the next iteration of the loop */
		//task_sleep ((RT_FREQ * TIME_16MS) / 16);
		int usecs_asked = 1000 - usecs_elapsed - 100;
		if ((usecs_asked < 0) || (usecs_asked > 1000000))
		{
			dbprintf (SLC_DEBUG, "warning: realtime sleep for %d ticks\n", usecs_asked);
		}
		pth_nap (pth_time (0, usecs_asked));

		/* Now see how long we actually slept.  This takes into account the
		actual sleep time, which is typically longer on a multitasking OS,
		and also the length of time that the previous iteration took. */
		gettimeofday (&curr_time, NULL);
		usecs_elapsed = curr_time.tv_usec - prev_time.tv_usec;
		if (usecs_elapsed < 0)
			usecs_elapsed += 1000000;

#ifdef CONFIG_DEBUG_LATENCY
		latency = usecs_elapsed - usecs_asked;
		if (latency > 500)
			simlog (SLC_DEBUG, "latency %d usec", latency);
#endif
		prev_time = curr_time;

		/* Invoke realtime tick at least once */
		realtime_counter++;
		if (linux_irq_enable)
			tick_driver ();
		callset_invoke (realtime_tick);

		usecs_elapsed -= usecs_asked;

		/* If any remaining millseconds occurred during the wait, handle them */
		while (usecs_elapsed >= 1000)
		{
			realtime_counter++;
			callset_invoke (realtime_tick);
			usecs_elapsed -= 1000;
		}

		/* Whatever was left over will be subtracted from the next delay, so
		that we stay on schedule */
	}
}


CALLSET_ENTRY (realtime, init)
{
	realtime_counter = 0;
	linux_irq_enable = linux_firq_enable = TRUE;
	linux_irq_pending = 0;
	task_create_gid_while (GID_LINUX_REALTIME, realtime_loop, TASK_DURATION_INF);
}

