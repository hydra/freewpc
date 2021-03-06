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

#ifndef _SYS_DEVICE_H
#define _SYS_DEVICE_H

/** The maximum number of switches that a ball device can have */
#ifndef MAX_SWITCHES_PER_DEVICE
#define MAX_SWITCHES_PER_DEVICE 6
#endif

/** The maximum number of ball devices that are supported */
#ifndef MAX_DEVICES
#define MAX_DEVICES 8
#endif

/** The GIDs for the device update tasks.
 * Every device has its own GID for the function that runs to service it.
 * This task is in charge of processing switch closures AND servicing
 * kick requests.
 */
#define DEVICE_GID_BASE		GID_DEVICE0_ACTIVE
#define DEVICE_GID1			GID_DEVICE1_ACTIVE
#define DEVICE_GID2			GID_DEVICE2_ACTIVE
#define DEVICE_GID3			GID_DEVICE3_ACTIVE
#define DEVICE_GID4			GID_DEVICE4_ACTIVE
#define DEVICE_GID5			GID_DEVICE5_ACTIVE
#define DEVICE_GID6			GID_DEVICE6_ACTIVE
#define DEVICE_GID7			GID_DEVICE7_ACTIVE


/** Translate a device number (devno), which is zero-based, into
 * its corresponding GID. */
#define DEVICE_GID(devno)	(GID_DEVICE0_ACTIVE + (devno))


struct device;

/** The device operations structure.  Each device can define callbacks
 * for various types of events/operations that are device-specific.
 * You can leave a field as NULL and it will not be called.
 * These are game-specific functions, as all of the basic stuff is done
 * by the common code here. */
typedef struct device_ops
{
	/** Called whenever a ball enters the device */
	void (*enter) (void);

	/** Called before the game attempts to kick a ball, to allow modules
	to deny the attempt, if it should not be tried at all for some reason.
	In that case, the device code will back-off for a few seconds and then
	request again.  Returns TRUE if the request is allowed, or FALSE if
	it is denied. */
	bool (*kick_request) (void);

	/** Called just before actually kick a ball from the device.  The kick
	is imminent and cannot be stopped at this point; this hook allows
	modules to perform a lamp effect or sound call in sync with the kick. */
	void (*kick_attempt) (void);

	/** Called when a kick is successful.
	 * If a delay is needed in between kicks, enforce that by
	 * putting a delay in this routine. */
	void (*kick_success) (void);

	/** Called when a kick is not successful */
	void (*kick_failure) (void);

	/** Called when the device becomes empty */
	void (*empty) (void);

	/** Called when a ball disappears from the device without a kick */
	void (*surprise_release) (void);
} device_ops_t;


/** Macro used by the device module to invoke device events.
 * The events are automatically generated by genmachine and gencallset.
 * These hooks are only called during an actual game; the system
 * is fully in charge during test/attract mode.
 */
#define device_call_op(dev, op) \
do { \
	if (in_game) \
	{ \
		far_indirect_call_handler (dev->props->ops->op, EVENT_PAGE); \
	}  \
} while (0)

#define device_call_boolean_op(dev, op) \
	far_indirect_call_value_handler (dev->props->ops->op, EVENT_PAGE)

/** The device properties structure is a read-only descriptor that
 * contains various compile-time attributes. */
typedef struct device_properties
{
	/** Name of the device */
	const char *name;

	/** Pointer to the operations structure */
	device_ops_t *ops;

	/** The solenoid used to kick balls from it */
	solnum_t sol;

	/** The number of switches in the device for counting balls */
	U8 sw_count;

	/** The initial number of switches that ought to be closed;
	 * i.e. the number of balls that the device is allowed to hold
	 * at initialization time.  Any extra balls found will be kicked. */
	U8 init_max_count;

	/** The switch numbers for the switches; switch 0 always refers to
	 * the first switch that would close when a ball enters the device,
	 * switch MAX-1 is the last switch to open when a ball leaves it.  */
	switchnum_t sw[MAX_SWITCHES_PER_DEVICE];

	/** The time to wait for a device to settle before evaluating switches */
	U8 settle_delay; // See TIME_* defines
} device_properties_t;


/**
 * Device states.  Each device is governed by a state
 * machine; these values dictate the various states that
 * a device can be in.
 */

/** The device is idle (initial state).  No kicks are in progress. */
#define DEV_STATE_IDLE			0

/** An 'enter' event has been detected due to switch closure(s). */
#define DEV_STATE_ENTERED		1

/** The device is in the process of kicking a ball out. */
#define DEV_STATE_RELEASING	2


/** The device info structure.  This is a read-write
 * structure that maintains the current status of a device.
 * Included is the state machine state, as well as other
 * things like how many balls are currently in the
 * device. */
typedef struct device
{
	/** Device number assigned to this device.  This is actually
	fixed, but kept here for convenience. */
	U8 devno;

	/** A bitmask unique to this device.  This is equivalent to
	the devno, but in bitmask form. */
	U8 devno_mask;

	/** The size of the device, same as the number of counting switches */
	U8 size;

	/** The number of 'virtual balls' in the device.  These are not seen
	 * by any switches and so are not seen by the normal recount
	 * mechanism.  However, they are 'known' to be in the device due to
	 * other methods.  APIs are available for modifying this. */
	U8 virtual_count;

	/** The current count of balls in the device, as determined by
	the most recent recount. */
	U8 actual_count;

	/** The previous count of balls, before the last recount.  Two
	successive recounts are used to determine what changes occurred. */
	U8 previous_count;

	/** The maximum number of balls that we want held here.
	 * If a ball enters the device and this count is not exceeded, then
	 * the ball is kept here.  Otherwise, the ball needs to be
	 * kicked out.  This variable basically implements 'ball locking'
	 * at the physical level. */
	U8 max_count;

	/** The number of balls that we've determined need to be kicked out */
	U8 kicks_needed;

	/** The number of consecutive kick errors.  This is used for retrying. */
	U8 kick_errors;

	/** The operational state of the device, one of the DEV_STATE_ values */
	U8 state;

	/** Pointer to the read-only device properties */
	device_properties_t *props;
} device_t;

typedef U8 devicenum_t;

#define device_entry(devno)	(&device_table[devno])
#define device_devno(dev)		(dev->devno)

/** True if the given device is empty */
#define device_empty_p(dev)	(dev->actual_count == 0)

/** True if the given device is full */
#define device_full_p(dev)		(dev->actual_count == dev->size)

/** Disable an automatic ball lock on this device */
#define device_disable_lock(dev)	(dev->max_count--)

/** Enable an automatic ball lock on this device */
#define device_enable_lock(dev)	(dev->max_count++)

/** Test if a device is the trough.  For the test fixture ROM, no
ball devices are defined, and this always returns FALSE. */
#ifdef DEVNO_TROUGH
#define trough_dev_p(dev) (device_devno(dev) == DEVNO_TROUGH)
#else
#define trough_dev_p(dev) FALSE
#endif

extern device_t device_table[];
extern U8 counted_balls;
extern U8 missing_balls;
extern U8 live_balls;
extern U8 held_balls;
extern U8 kickout_locks;

__common__ void device_clear (device_t *dev);
__common__ void device_register (devicenum_t devno, device_properties_t *props);
__common__ U8 device_recount (device_t *dev);
__common__ void device_update_globals (void);
__common__ void device_probe (void);
__common__ void device_request_kick (device_t *dev);
__common__ void device_request_empty (device_t *dev);
__common__ void device_sw_handler (U8 devno);
__common__ void device_add_live (void);
__common__ void device_remove_live (void);
__common__ void device_add_virtual (device_t *dev);
__common__ void device_remove_virtual (device_t *dev);
__common__ bool device_check_start_ok (void);
__common__ void device_unlock_ball (device_t *dev);
__common__ void device_lock_ball (device_t *dev);
__common__ U8 device_holdup_count (void);
__common__ void device_init (void);

/** Acquire a kickout lock.  When at least kickout lock is
 * held, then all kicks are postponed. */
#define kickout_lock(by)	do { kickout_locks |= (by); } while (0)

/** Release a kickout lock */
#define kickout_unlock(by)	do { kickout_locks &= ~(by); } while (0)

/** Release a kickout lock */
#define kickout_locked_p()	(kickout_locks != 0)

/** Release a kickout lock */
#define kickout_unlock_all()	do { kickout_locks = 0; } while (0)

#define KLOCK_DEFF 0x1    /* Kickout locked during current deff */
#define KLOCK_USER 0x2    /* Kickout locked by explicit user call */

#define single_ball_play() (live_balls <= 1)
#define multi_ball_play() (live_balls > 1)

#endif /* _SYS_DEVICE_H */
