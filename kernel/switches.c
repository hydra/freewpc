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

/**
 * \file
 * \brief Handle all switch inputs
 */

#include <freewpc.h>
#include <diag.h>
#include <search.h>

/*
 * A pending switch is one which has transitioned and is eligible
 * for servicing, but is undergoing additional debounce logic.
 * This struct tracks the switch number, its state, and the
 * debounce timer.
 *
 * There are a finite number of these objects; when the hardware
 * detects a transition, it will either allocate a new slot or
 * re-use an existing slot for the same switch that did not
 * complete its debounce cycle.
 */
typedef struct
{
	/* The switch number that is pending */
	U8 id;

	/* The amount of time left before the transition completes. */
	S8 timer;
} pending_switch_t;


/** The raw input values of the switch.  These values are
 * updated every 2ms, and are only used as inputs into the
 * debounce procedure.  Higher layer software never looks at
 * these values. */
__fastram__ switch_bits_t sw_raw;

/** Nonzero for each switch whose last reading differs from the
most recent logical (debounced) reading */
__fastram__ switch_bits_t sw_edge;

/** Nonzero for each switch that has been stable at the IRQ level.
 * This means two consecutive readings, 2ms apart, returned the
 * same value, which differed from the last stable reading. */
__fastram__ switch_bits_t sw_stable;

/** Nonzero for each stable switch (according to sw_stable)
 * that transitioned back to its previous reading.  This means
 * the switch did not remain stable long enough for periodic
 * switch scanning to recognize it. */
__fastram__ switch_bits_t sw_unstable;

/** The current logical (i.e. debounced) readings for each
 * switch.  These are still based on voltage levels and do not
 * consider inverting necessary for processing optos. */
__fastram__ switch_bits_t sw_logical;

/** Nonzero for each switch that is in the switch queue.
 * This is not strictly needed, but it provides a fast way to
 * see if a switch is already in the debounce queue. */
switch_bits_t sw_queued;

/* A list of pending switches which have not fully debounced yet */
pending_switch_t switch_queue[MAX_QUEUED_SWITCHES];

/* A pointer to the first free entry in the switch queue.
 * When this is equal to 'switch_queue', it means the entire
 * queue is empty. */
__fastram__ pending_switch_t *switch_queue_top;

/** The last time that switch scanning occurred.  It is used
 * to determine by how much to decrement debounce timers. */
U16 switch_last_service_time;

/** The switch number of the last switch to be scheduled. */
U8 sw_last_scheduled;

/** Nonzero if a switch short was detected and switches need to be
 * ignored for some time.  The value indicates the number of
 * seconds to ignore switches. */
U8 sw_short_timer;


#ifdef CONFIG_COMBOS

#define COMBO_START           (1 << 0)
#define COMBO_END             (1 << 1)
#define IGNORE_TIME           (1 << 2)
#define IGNORE_SWITCHES_UNTIL (1 << 3)
#define NO_FLAGS              0


#define CF_NEVER               0
#define CF_ALWAYS              (1 << 0)
#define CF_SINGLE_BALL_ONLY    (1 << 1)
#define CF_MULTI_BALL_ONLY     (1 << 3)

// NOTE due to reverse processing of combo switches, the IGNORE_SWITCHES_UNTIL flag and
// the time value must be on the line before you would expect normally it.

extern void callset_left_orbit_combo_shot (void);
extern void callset_right_orbit_combo_shot (void);
extern void callset_rl_orbit_rl_ramp_combo_shot (void);
extern void callset_ordered_bank_1_combo_shot (void);

combo_definition_t left_orbit_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "LO",
	.fn = callset_left_orbit_combo_shot,
	.switch_count = 2,
	.switch_list = {
		{SW_LEFT_OUTER_LOOP,  TIME_66MS, COMBO_START},
		{SW_RIGHT_OUTER_LOOP, 0,         COMBO_END}
	}
};

combo_definition_t right_orbit_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "RO",
	.fn = callset_right_orbit_combo_shot,
	.switch_count = 2,
	.switch_list = {
		{SW_RIGHT_OUTER_LOOP, TIME_66MS, COMBO_START},
		{SW_LEFT_OUTER_LOOP,  0,         COMBO_END}
	}
};

combo_definition_t ordered_bank_1_combo = {
	.flags = CF_ALWAYS,
	.name = "OBC1",
	.fn = callset_ordered_bank_1_combo_shot,
	.switch_count = 3,
	.switch_list = {

		// this says, match SW_LEFT_STANDUP_3
		// and if SW_LEFT_STANDUP_2 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// and if SW_LEFT_STANDUP_1 was hit within 5 seconds before that (during which time other switches could have been pressed)
		// then the combo is matched

		{SW_LEFT_STANDUP_1, TIME_5S, COMBO_START|IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_STANDUP_2, TIME_5S, IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_STANDUP_3, 0,       COMBO_END}
	}
};


combo_definition_t right_left_orbit_and_right_left_ramp_combo = {
	.flags = CF_SINGLE_BALL_ONLY,
	.name = "RLO RLR",
	.fn = callset_rl_orbit_rl_ramp_combo_shot,
	.switch_count = 8,
	.switch_list = {
		{SW_RIGHT_OUTER_LOOP,  TIME_66MS, COMBO_START},
		{SW_LEFT_OUTER_LOOP,   TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_LEFT_OUTER_LOOP,   TIME_66MS, NO_FLAGS},
		{SW_RIGHT_OUTER_LOOP,  TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_ROUTE_66_ENTRY,    TIME_1S,   NO_FLAGS},
		{SW_ROUTE_66_EXIT,     TIME_5S,   IGNORE_SWITCHES_UNTIL},
		{SW_ZR1_BOTTOM_ENTRY,  TIME_1S,   NO_FLAGS},
		{SW_ZR1_TOP_ENTRY,     0,         COMBO_END}
	}
};


combo_definition_t *combo_list[] = {
	&left_orbit_combo,
	&right_orbit_combo,
	&ordered_bank_1_combo,
	&right_left_orbit_and_right_left_ramp_combo
};

#define MAX_RECENT_SWITCHES 32

typedef struct recent_switch_s {
	U8 switch_id;
	U16 hit_time;
} recent_switch_t;


recent_switch_t recent_switches[MAX_RECENT_SWITCHES];
U8 next_recent_switch; // a tail pointer which points to the next slot to be used which is also the oldest recent-switch.

CALLSET_ENTRY (combo_handler, init_complete, start_ball)
{
	next_recent_switch = 0;
	memset(recent_switches, 0, sizeof(recent_switches));
}

#ifdef CONFIG_DEBUG_COMBOS
void dump_recent_switches( void ) {
	U8 dump_count = 0;
	U8 index = next_recent_switch;
	recent_switch_t *recent_switch;
	while (dump_count < MAX_RECENT_SWITCHES) {
		if (index == 0) {
			index = MAX_RECENT_SWITCHES;
		}
		index--;
		recent_switch = &recent_switches[index];
		dbprintf("dc: %d, i: %d, s: %d, t: %ld\n", dump_count, index, recent_switch->switch_id, recent_switch->hit_time);
		task_runs_long();

		dump_count++;
	}
}
#endif

combo_definition_t *last_matched_combo;

#define MATCHED_NONE   0
#define MATCHED_SWITCH (1 << 0)
#define MATCHED_TIME   (1 << 1)

void process_combo(combo_definition_t *combo) {
#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("processing combo: %s\n", combo->name);
#endif

	if (!(
			(combo->flags & CF_ALWAYS) ||
			((combo->flags & CF_SINGLE_BALL_ONLY) && single_ball_play()) ||
			((combo->flags & CF_MULTI_BALL_ONLY) && !single_ball_play())
	)) {
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("skipping, flags: %d, sbp: %d\n", combo->flags, single_ball_play());
#endif
		return;
	}

	// compare the combo switch list, and timings, to the recent switch list.

	U8 recent_index = next_recent_switch;
	// combo are processed in reverse order from last switch to first
	// because there's less matches to attempt that way round
	U8 combo_index = combo->switch_count;
	U8 matched;
	recent_switch_t *recent_switch;
	combo_switch_entry_t *combo_switch;
	U8 last_checked_switch_id = 0;
	U16 previous_recent_time = 0;

	while (combo_index > 0) {
		combo_index--;
		combo_switch = &combo->switch_list[combo_index];


		matched = MATCHED_NONE;
		do {
			task_runs_long();
			if (recent_index == 0) {
				recent_index = MAX_RECENT_SWITCHES;
			}
			recent_index--;

			recent_switch = &recent_switches[recent_index];

#ifdef CONFIG_DEBUG_COMBOS
			dbprintf("ci: %d, cs: %d, ri: %d, rs: %d, t: %ld\n",
				combo_index,
				combo_switch->switch_id,
				recent_index,
				recent_switch->switch_id,
				recent_switch->hit_time
			);
#endif

			if (recent_switch->switch_id == 0) {
#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("ran out of recent switches, bailing\n");
#endif
				break;
			}
/*

recent:
R L L L R    should match
R L L R      should match
R L L R R    should not match
combo:
R L* L R

recent:
1 2 3 3      should not match
1 2 3 2 3    should not match
1 2 3 1 3    should not match
<1 2 3>        should match
1 <1 2 2 3>    should match
1 <1 2 2 3 3>  should not match

1 2 3 2 3

combo:
1* 2* 3


 */
			// FIXME this fix breaks the use of two switches in a row in the combo spec though.
			if (last_checked_switch_id == recent_switch->switch_id && last_checked_switch_id && (combo->switch_list[combo_index+1].flags & IGNORE_SWITCHES_UNTIL)) {
				// Avoid matching multiple-switch presses of the same switch.
				// when the previous combo switch wasn't using IGNORE_SWITCHES_UNTIL
				// Without this check it's possible to repeatedly trigger a combo by hitting the last switch as long as the
				// rest of the switches are in the recent_switches list and the time between switch hits is
			    // still within the allowed time when IGNORE_SWITCHES_UNTIL is used in the combo definition
				// on the second-to-last switch.

#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("same as last checked switch, bailing\n");
#endif

				break;
			}
			last_checked_switch_id = recent_switch->switch_id;

			if (recent_switch->switch_id == combo_switch->switch_id) {
				matched |= MATCHED_SWITCH;
				// don't check time on the first switch as there's no other time to compare it with
				if (combo_index < combo->switch_count - 1) {
#ifdef CONFIG_DEBUG_COMBOS
					dbprintf("diff between previous: %ld\n", previous_recent_time - recent_switch->hit_time);
					// was this switch activated within the timeout period allowed since the previous switch?
					dbprintf("combo time allowed: %ld\n", combo_switch->max_time_difference * IRQS_PER_TICK);
#endif
					if (combo_switch->max_time_difference * IRQS_PER_TICK > previous_recent_time - recent_switch->hit_time) {
#ifdef CONFIG_DEBUG_COMBOS
						dbprintf("time elapsed is OK!\n");
#endif
						matched |= MATCHED_TIME;
					} else {
#ifdef CONFIG_DEBUG_COMBOS
						dbprintf("too much time elapsed\n");
#endif
					}
				} else {
					// if the first switch is the last one in the combo's list of switches, it matches regardless of times
					matched |= MATCHED_TIME;
				}
				previous_recent_time = recent_switch->hit_time;
			} else {
#ifdef CONFIG_DEBUG_COMBOS
				dbprintf("unmatched\n");
				if (combo_switch->flags & IGNORE_SWITCHES_UNTIL) {
					dbprintf("ignoring switch due to flag\n");
				}
#endif

				if (recent_index == next_recent_switch) {
#ifdef CONFIG_DEBUG_COMBOS
					dbprintf("wrapped, bailing!\n");
#endif
					break;
				}
			}
		} while (!(matched & MATCHED_SWITCH) && (combo_switch->flags & IGNORE_SWITCHES_UNTIL));
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("match value: %d\n", matched);
#endif

		if (matched != (MATCHED_SWITCH | MATCHED_TIME)) {
			break;
		}
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("matched switch & time\n");
#endif

		if (combo_index == 0) {
#ifdef CONFIG_DEBUG_COMBOS
			dbprintf("combo matched!\n");
#endif
			last_matched_combo = combo;

			callset_pointer_invoke(combo->fn);
		}
	}
}

void process_combos(void) {
	U8 combo_count = sizeof(combo_list) / sizeof(combo_definition_t *);
#ifdef CONFIG_DEBUG_COMBOS
	dbprintf("processing %d combos\n", combo_count);
#endif
	U8 combo_index = 0;
	while (combo_index < combo_count) {
		process_combo(combo_list[combo_index]);
		combo_index++;
	}
}
#endif


/** Return the switch table entry for a switch */
const switch_info_t *switch_lookup (const switchnum_t sw)
{
	extern const switch_info_t switch_table[];
	return &switch_table[sw];
}


/** Return the lamp associated with a switch */
U8 switch_lookup_lamp (const switchnum_t sw)
{
	U8 lamp;
	const switch_info_t *swinfo = switch_lookup (sw);
	lamp = swinfo->lamp;
	return lamp;
}


/** Detect row / column shorts */
void switch_short_detect (void)
{
	U8 n;

	n = sw_raw[0] & sw_raw[1] & sw_raw[2] & sw_raw[3] &
		sw_raw[4] & sw_raw[5] & sw_raw[6] & sw_raw[7];
	if (n != 0)
	{
		// one or more rows are shorted
		dbprintf ("Row short\n", n);
		sw_short_timer = 3;
	}

	for (n = 0; n < 8; n++)
	{
		if (sw_raw[n] == (U8)~mach_opto_mask[n])
		{
			/* The nth column is shorted. */
			dbprintf ("Column short\n");
			sw_short_timer = 3;
		}
	}
}


/* Before any switch data can be accessed on a WPC-S
 * or WPC95 machine, we need to poll the PIC and see
 * if the unlock code must be sent to it.   On pre-
 * security games, this function is a no-op. */
void pic_rtt (void)
{
#if (MACHINE_PIC == 1)
	U8 unlocked;

	/* Until initialization is complete, the PIC is not ready
	to receive unlock commands. */
	if (sys_init_complete == 0)
		return;

	/* Read the status to see if the matrix is still unlocked. */
	wpc_write_pic (WPC_PIC_COUNTER);
	null_function ();
	null_function ();
	null_function ();
	unlocked = wpc_read_pic ();
	if (!unlocked)
	{
		/* We need to unlock it again. */
		extern U8 pic_unlock_code[3];
		extern bool pic_unlock_ready;

		if (!pic_unlock_ready)
			return;

		/* The unlock sequence is four bytes long, but we can't
		write everything without some delay between bytes.
		The 'null_function' calls are there just to delay for
		a few tens of cycles. */
		wpc_write_pic (WPC_PIC_UNLOCK);
		null_function ();
		null_function ();
		wpc_write_pic (pic_unlock_code[0]);
		null_function ();
		null_function ();
		wpc_write_pic (pic_unlock_code[1]);
		null_function ();
		null_function ();
		wpc_write_pic (pic_unlock_code[2]);
		null_function ();
		null_function ();
	}
#endif /* MACHINE_PIC */
}


#if defined(CONFIG_PLATFORM_WPC) && defined(__m6809__)
extern inline void switch_rowpoll (const U8 col)
{
	/* Load the raw switch value from the hardware. */
	if (col == 0)
	{
		/* Column 0 = Dedicated Switches */
		__asm__ volatile ("ldb\t" C_STRING (WPC_SW_CABINET_INPUT));
	}
	else if (col <= 8)
	{
		/* Columns 1-8 = Switch Matrix
		Load method is different on PIC vs. non-PIC games. */
#if (MACHINE_PIC == 1)
		__asm__ volatile ("ldb\t" C_STRING (WPCS_PIC_READ));
#else
		__asm__ volatile ("ldb\t" C_STRING (WPC_SW_ROW_INPUT));
#endif
	}
	else if (col == 9)
	{
		/* Column 9 = Flipper Inputs
		Load method is different on WPC-95 vs. older Fliptronic games */
#if (MACHINE_WPC95 == 1)
		__asm__ volatile ("ldb\t" C_STRING (WPC95_FLIPPER_SWITCH_INPUT));
#else
		__asm__ volatile ("ldb\t" C_STRING (WPC_FLIPTRONIC_PORT_A));
#endif
	}

	/* Save the raw switch */
	__asm__ volatile ("stb\t%0" :: "m" (sw_raw[col]) );

	/* Set up the column strobe for the next read (on the next
	 * iteration).  Don't do this if the next read will be the dedicated
	 * switches.
	 *
	 * PIC vs. non-PIC games set up the column strobe differently. */
	if (col < 8)
	{
#if (MACHINE_PIC == 1)
		__asm__ volatile (
			"lda\t%0\n"
			"\tsta\t" C_STRING (WPCS_PIC_WRITE) :: "n" (WPC_PIC_COLUMN (col)) );
#else
		__asm__ volatile (
			"lda\t%0\n"
			"\tsta\t" C_STRING (WPC_SW_COL_STROBE) :: "n" (1 << col) );
#endif
	}

	/* Update stable/unstable states.  This is an optimized
	version of the C code below, which works around some GCC
	problems that are unlikely to ever be fixed.  I took the
	gcc output and optimized it by hand. */
	__asm__ volatile (
			"eorb	%0\n"
			"\ttfr	b,a\n"
			"\tandb	%1\n"
			"\tsta	%1\n"
			"\torb	%2\n"
			"\tstb	%2\n"
			"\tcoma\n"
			"\tanda	%2\n"
			"\tora	%3\n"
			"\tsta	%3" ::
				"m" (sw_logical[col]), "m" (sw_edge[col]),
				"m" (sw_stable[col]), "m" (sw_unstable[col]) );
}

#else /* !__m6809__ */

/** Poll a single switch column.
 * Column 0 corresponds to the cabinet switches.
 * Columns 1-8 refer to the playfield columns.
 * It is assumed that columns are polled in order, as
 * during the read of column N, the strobe is set so that
 * the next read will come from column N+1.
 */
extern inline void switch_rowpoll (const U8 col)
{
	U8 edge;

	/*
	 * Read the raw switch.
	 */
	if (col == 0)
		sw_raw[col] = pinio_read_dedicated_switches ();
	else if (col <= 8)
		sw_raw[col] = pinio_read_switch_rows ();
	else if (col == 9)
		sw_raw[col] = wpc_read_flippers ();

	/* Set up the column strobe for the next read (on the next
	 * iteration) */
	if (col < 8)
		pinio_write_switch_column (col);

	/* Update stable/unstable states. */
	edge = sw_raw[col] ^ sw_logical[col];
	sw_stable[col] |= edge & sw_edge[col];
	sw_unstable[col] |= ~edge & sw_stable[col];
	sw_edge[col] = edge;
}

#endif


/** Return TRUE if the given switch is CLOSED.
 * This scans the logical values calculated at periodic service time. */
bool switch_poll (const switchnum_t sw)
{
	return bitarray_test (sw_logical, sw);
}


/** Return TRUE if the given switch is an opto.  Optos
are defined in the opto mask array, which is auto generated from
the machine description. */
bool switch_is_opto (const switchnum_t sw)
{
	return bitarray_test (mach_opto_mask, sw);
}


/** Return TRUE if the given switch is ACTIVE.  This takes into
 * account whether or not the switch is an opto. */
bool switch_poll_logical (const switchnum_t sw)
{
	return switch_poll (sw) ^ switch_is_opto (sw);
}


void switch_rtt_0 (void)
{
	switch_rowpoll (0);
	if (switch_scanning_ok ())
	{
		switch_rowpoll (1);
		switch_rowpoll (2);
		switch_rowpoll (3);
		switch_rowpoll (4);
	}
}


void switch_rtt_1 (void)
{
	if (switch_scanning_ok ())
	{
		switch_rowpoll (5);
		switch_rowpoll (6);
		switch_rowpoll (7);
		switch_rowpoll (8);
	}

#if (MACHINE_FLIPTRONIC == 1)
	/* Poll the Fliptronic flipper switches */
	switch_rowpoll (9);
#endif
}


typedef struct
{
	const switch_info_t *swinfo;
	leff_data_t leffdata;
} lamp_pulse_data_t;


/** Task that performs a switch lamp pulse.
 * Some switches are inherently tied to a lamp.  When the switch
 * triggers, the lamp can be automatically flickered.  This is
 * implemented as a pseudo-lamp effect, so the true state of the
 * lamp is not disturbed. */
void switch_lamp_pulse (void)
{
	lamp_pulse_data_t * const cdata = task_current_class_data (lamp_pulse_data_t);
	/* Although not a true leff, this fools the lamp draw to doing
	 * the right thing. */
	cdata->leffdata.flags = L_SHARED;

	/* If the lamp is already allocated by another lamp effect,
	then don't bother trying to do the pulse. */
	if (lamp_leff2_test_and_allocate (cdata->swinfo->lamp))
	{
		/* Change the state of the lamp */
		if (lamp_test (cdata->swinfo->lamp))
			leff_off (cdata->swinfo->lamp);
		else
			leff_on (cdata->swinfo->lamp);
		task_sleep (TIME_200MS);

		/* Change it back */
		leff_toggle (cdata->swinfo->lamp);
		task_sleep (TIME_200MS);

		/* Free the lamp */
		lamp_leff2_free (cdata->swinfo->lamp);
	}
	task_exit ();
}


/*
 * The entry point for processing a switch transition.  It performs
 * some of the common switch handling logic before calling all
 * event handlers.  Then it also performs some common post-processing.
 * This function runs in a separate task context for each switch that
 * needs to be processed.
 */
void switch_sched_task (void)
{
	const U8 sw = (U8)task_get_arg ();
	const switch_info_t * const swinfo = switch_lookup (sw);

	/* Ignore any switch that doesn't have a processing function.
	   This shouldn't ever happen if things are working correctly, but it
		was observed on PIC games when the PIC code is broken and reporting
		bad switch returns.  genmachine ensures that all defined switches
		have a non-null value (null_function if nothing needs to be done) */
	if (swinfo->fn == 0)
		goto cleanup;

	/* For test mode : this lets it see what was the last switch
	 * to be scheduled.  Used by the Switch Edges test. */
	sw_last_scheduled = sw;

	log_event (SEV_INFO, MOD_SWITCH, EV_SW_SCHEDULE, sw);

	/* Don't service switches marked SW_IN_GAME at all, if we're
	 * not presently in a game */
	if ((swinfo->flags & SW_IN_GAME) && !in_game)
		goto cleanup;

	/* Don't service switches not marked SW_IN_TEST, unless we're
	 * actually in test mode */
	if (!(swinfo->flags & SW_IN_TEST) && in_test)
		goto cleanup;

	/* If the switch has an associated lamp, then flicker the lamp when
	 * the switch triggers. */
	if ((swinfo->lamp != 0) && in_live_game)
	{
		task_pid_t tp = task_create_gid (GID_SWITCH_LAMP_PULSE,
			switch_lamp_pulse);

		lamp_pulse_data_t *cdata = task_init_class_data (tp, lamp_pulse_data_t);
		cdata->swinfo = swinfo;
	}

	/* If we're in a live game and the switch declares a standard
	 * sound, then make it happen. */
	if ((swinfo->sound != 0) && in_live_game)
		sound_send (swinfo->sound);

#ifdef DEBUGGER
	if (swinfo->fn != null_function && sw < 72)
	{
		dbprintf ("SW: ");
		sprintf_far_string (names_of_switches + sw);
		dbprintf1 ();
#ifdef DEBUG_SWITCH_NUMBER
		dbprintf (" (%d) ", sw);
#endif
		dbprintf ("\n");
	}
#endif

	/* Call the processing function.  All functions are in the EVENT_PAGE. */
	callset_pointer_invoke (swinfo->fn);

	/* If a switch is marked SW_PLAYFIELD and we're in a game,
	 * then call the global playfield switch handler and mark
	 * the ball 'in play'.  Don't do the last bit if the switch
	 * specifically doesn't want this to happen.  Also, kick
	 * the ball search timer so that it doesn't expire.
	 */
	if ((swinfo->flags & SW_PLAYFIELD) && in_game)
	{
		callset_invoke (any_pf_switch);

		// TODO maybe use CONFIG_COMBOS | CONFIG_RECENT_SWITCHES ?
#ifdef CONFIG_COMBOS
		U16 now = get_sys_time();
#ifdef CONFIG_DEBUG_COMBOS
		dbprintf("now: %ld, sw: %d\n", now, sw_last_scheduled);
#endif
		recent_switch_t *switch_hit = &recent_switches[next_recent_switch];
		switch_hit->hit_time = now;
		switch_hit->switch_id = sw_last_scheduled;

		next_recent_switch++;
		if (unlikely( next_recent_switch >= MAX_RECENT_SWITCHES)) {
			next_recent_switch = 0;
		}

#ifdef CONFIG_DEBUG_RECENT_SWITCHES
		dump_recent_switches();
#endif

		process_combos();
#endif


		/* If valid playfield not asserted yet, then see if this
		 * switch validates it.  Most switches do this right
		 * away; for other switches, like special solenoids,
		 * queue the transition and maybe validate if some number
		 * of different switches have triggered.  Device counting
		 * switches are ignored here, but an 'enter' event will
		 * set it. */
		if (!valid_playfield)
		{
			if (swinfo->flags & SW_NOVALID)
			{
				if (!SW_HAS_DEVICE (swinfo))
					try_validate_playfield (sw);
			}
			else
				set_valid_playfield ();
		}
		ball_search_timer_reset ();
	}

cleanup:
	/* If the switch is part of a device, then let the device
	 * subsystem process the event */
	if (SW_HAS_DEVICE (swinfo))
		device_sw_handler (SW_GET_DEVICE (swinfo));

	task_exit ();
}


/**
 * Process a switch that has transitioned states.  All debouncing
 * is fully completed prior to this call.
 *
 * The transition is first latched, so that polling the switch level
 * will return the new state.  At the same time, IRQ-level scanning
 * is restarted, so that further transitions can be detected.
 *
 * Second, if the transition requires scheduling, a task is started
 * to handle it.  Eligibility depends on whether or not the switch
 * is declared as an edge switch (meaning it is scheduled on both
 * types of transitions) and whether or not it is an opto (i.e.
 * do we schedule open-to-closed or closed-to-open?)
 *
 * The switch is also known not to be in the debounce queue prior to this
 * function being called.
 */
void switch_transitioned (const U8 sw)
{
	/* Latch the transition.  sw_logical is still an open/closed level.
	 * By clearing the stable/unstable bits, IRQ will begin scanning
	 * for new transitions at this point. */
	disable_irq ();
	bit_toggle (sw_logical, sw);
	bit_off (sw_stable, sw);
	bit_off (sw_unstable, sw);
	enable_irq ();

	/* See if the transition requires scheduling.  It does if:
	 a) the switch is bidirectional, or
	 b) the switch is an opto and it is now open, or
	 c) the switch is not an opto and it is now closed.
	*/
	if (bit_test (mach_edge_switches, sw)
		|| (!bit_test (sw_logical, sw) && bit_test (mach_opto_mask, sw))
		|| (bit_test (sw_logical, sw) && !bit_test (mach_opto_mask, sw)))
	{
#ifdef CONFIG_BPT
		/* One extra condition : do not schedule any switches when the
		system is paused */
		if (db_paused != 0)
			return;
#endif

		/* Start a task to process the switch event.
		This task may sleep if necessary, but it should be as fast as possible
		and push long-lived operations into separate background tasks.
		It is possible for more than instance of a task to exist for the same
		switch, if valid debounced transitions occur quickly. */
		task_pid_t tp = task_create_gid (GID_SW_HANDLER, switch_sched_task);
		task_set_arg (tp, sw);
	}
}


/** Add a new entry to the switch queue. */
void switch_queue_add (const switchnum_t sw)
{
	if (switch_queue_top < switch_queue + MAX_QUEUED_SWITCHES)
	{
		dbprintf ("adding %d to queue\n", sw);
		switch_queue_top->id = sw;
		switch_queue_top->timer = switch_lookup(sw)->debounce;
		bit_on (sw_queued, sw);
		switch_queue_top++;
	}
}


/** Remove an entry from the switch queue */
void switch_queue_remove (pending_switch_t *entry)
{
	/* Copy the last entry in the queue into the slot being deleted. */
	entry->id = (switch_queue_top-1)->id;
	entry->timer = (switch_queue_top-1)->timer;
	switch_queue_top--;
	bit_off (sw_queued, entry->id);
}

/** Initialize the switch queue */
void switch_queue_init (void)
{
	switch_queue_top = switch_queue;
	memset (sw_stable, 0, sizeof (sw_stable));
	memset (sw_unstable, 0, sizeof (sw_unstable));
	memset (sw_queued, 0, sizeof (sw_queued));
}


/** Service the switch queue.  This function is called
 * periodically to see if any pending switch transitions have
 * completed their debounce time.  If any switch becomes
 * unstable before the period expires, it will be removed
 * from the queue prior to this function being called.  So
 * all that is needed here is to decrement the timers, and if
 * one reaches zero, consider that switch transitioned.
 */
void switch_service_queue (void)
{
	pending_switch_t *entry;
	U8 elapsed_time;

	/* See how long since the last time we serviced the queue.
	This is in 16ms ticks. */
	elapsed_time = get_elapsed_time (switch_last_service_time);

	entry = switch_queue;
	while (unlikely (entry < switch_queue_top))
	{
		dbprintf ("Service SW%d: ", entry->id);
		entry->timer -= elapsed_time;
		if (entry->timer <= 0)
		{
			/* Debounce interval is complete.  The entry can be removed
			from the queue */
			switch_queue_remove (entry);

			/* See if the switch held its state during the debounce period */
			if (bit_test (sw_unstable, entry->id))
			{
				/* Debouncing failed, so don't process the switch.
				 * Restart IRQ-level scanning. */
				disable_irq ();
				bit_off (sw_stable, entry->id);
				bit_off (sw_unstable, entry->id);
				enable_irq ();
			}
			else
			{
				/* Debouncing succeeded, so process the switch */
				switch_transitioned (entry->id);
			}
		}
		else
		{
			dbprintf ("%d down, %d to go\n", elapsed_time, entry->timer);
		}
		entry++;
	}

	switch_last_service_time = get_sys_time ();
}


#ifdef DEBUGGER
static inline void switch_matrix_dump (const char *name, U8 *matrix)
{
	U8 i;
	dbprintf ("%s:", name);
	for (i=0; i < SWITCH_BITS_SIZE; i++)
		dbprintf ("%02X ", matrix[i]);
	dbprintf ("\n");
}


void switch_queue_dump (void)
{
	pending_switch_t *entry = switch_queue;

	dbprintf ("Switch queue base: %p\n", switch_queue);
	dbprintf ("Switch queue top: %p\n", switch_queue_top);
	while (entry != switch_queue_top)
	{
		dbprintf ("Pending: SW%d  %d\n", entry->id, entry->timer);
		entry++;
	}
	switch_matrix_dump ("Raw     ", sw_raw);
	switch_matrix_dump ("Logical ", sw_logical);
	switch_matrix_dump ("Edge    ", sw_edge);
	task_runs_long ();
	switch_matrix_dump ("Stable  ", sw_stable);
	switch_matrix_dump ("Unstable", sw_unstable);
	switch_matrix_dump ("Queued  ", sw_queued);
}
#endif /* DEBUGGER */


/**
 * Handle a switch that has just become stable; i.e. it changed
 * and held steady for at least 2 consecutive readings.  It is
 * also known not to be in the heavy debounce queue already.
 *
 * If the switch requires a longer debounce period, then it is
 * put into a queue and we wait a little while to see if it
 * remains stable, then it will be scheduled.
 */
static void switch_update_stable (const U8 sw)
{
	/* Queue the switch if it requires further debouncing.
	 * Otherwise, it is eligible for scheduling. */
	if (switch_lookup (sw)->debounce != 0)
		switch_queue_add (sw);
	else
		switch_transitioned (sw);
}


/** Periodic switch processing.  This function is called frequently
 * to scan pending switches and spawn new tasks to handle them.
 */
void switch_periodic (void)
{
	register U16 col = 0;
	extern U8 sys_init_complete;
	U8 rows;

	/* If there are row/column shorts, ignore the switch matrix. */
	if (unlikely (sw_short_timer))
		return;

	/* Prior to system initialization, switches are not serviced.
	Any switch closures during this time continue to be queued up.
	However, at the least, allow the coin door switches to be polled. */
	if (unlikely (sys_init_complete == 0))
	{
		sw_logical[0] = sw_raw[0];
		return;
	}

	/* Check for shorted switch rows/columns. */
	switch_short_detect ();

	/* Service the switch queue.  Note that this is done BEFORE
	 * polling switches; please do not change this!  In case
	 * idle is not invoked fast enough, it is possible that a
	 * switch might have legitimately completed its debounce period,
	 * but we just didn't see it in time before it transitioned
	 * back.  In that case, to be fair, consider the transition
	 * anyway.  The service function does not actually poll the
	 * current switch readings at all, so it is safe to do this
	 * even if there are hardware errors. */
	switch_service_queue ();

	/* Iterate over each switch column to see what needs to be done. */
	task_dispatching_ok = TRUE;
	for (col=0; col < SWITCH_BITS_SIZE; col++)
	{
		/* Each bit in sw_stable indicates a switch
		that just transitioned and may need to be processed */
		if (unlikely (rows = sw_stable[col]))
		{
			U8 sw = col * 8;
			do {
				if ((rows & 1) && !bit_test (sw_queued, sw))
					switch_update_stable (sw);
				rows >>= 1;
				sw++;
			} while (rows);
		}
		task_runs_long ();
	}
}

/** As part of startup diagnostics, check that the 12V
 * switch matrix power is present. */
CALLSET_ENTRY (switch, diagnostic_check)
{
#if (MACHINE_PIC == 1)
	if (!switch_scanning_ok ())
	{
		diag_post_error ("SECURITY PIC\nNOT INITIALIZED\n", SYS_PAGE);
		return;
	}
#endif

#ifdef SW_ALWAYS_CLOSED
	/* Make sure the ALWAYS CLOSED switch is really closed.
	 * If not, there's a serious problem
	 * and we can't do any switch processing. */
	if (unlikely (!rt_switch_poll (SW_ALWAYS_CLOSED)))
		diag_post_error ("12V SWITCH POWER\nIS NOT PRESENT\n", SYS_PAGE);
#endif

#ifdef SW_SLAM_TILT
	if (unlikely (rt_switch_poll (SW_SLAM_TILT)))
		diag_post_error ("SLAM TILT IS\nSTUCK CLOSED\n", SYS_PAGE);
#endif
}


/** Do final initializing once system init is complete. */
CALLSET_ENTRY (switch, init_complete)
{
	/* Initialize the service timer.  This needs to happen
	 * just before interrupts are enabled */
	switch_last_service_time = get_sys_time ();
}


/** Once per second, see if we had disabled switch scanning
 * due to a short, and it should be reenabled now.  This
 * is not accurately timed: it may range from 2.1 to 3s of
 * disabled time. */
CALLSET_ENTRY (switch, idle_every_second)
{
	if (sw_short_timer > 0)
	{
		sw_short_timer--;
	}
}


/** Initialize the switch subsystem */
void switch_init (void)
{
	/* Initialize the short timer so switch scanning is enabled */
	sw_short_timer = 0;

	/* Initialize the switch state buffers.  raw/logical are
	set to 'inactive' for all switches; for optos that means the
	bit is set, not clear.  edge is set to all zeroes, meaning that
	no change was seen since the last reading. */
	memcpy (sw_raw, mach_opto_mask, SWITCH_BITS_SIZE);
	memcpy (sw_logical, mach_opto_mask, SWITCH_BITS_SIZE);
	memset (sw_edge, 0, sizeof (switch_bits_t));

	/* Initialize the switch queue */
	switch_queue_init ();
}
