
#include <freewpc.h>
#include <asm-6809.h>

uint8_t errcode;

__fastram__ uint8_t irq_count;
__fastram__ uint8_t tick_count;

uint8_t sys_init_complete;


void do_fatal (uint16_t pc, errcode_t error_code) __noreturn__
{
	U8 *stack = (U8 *)get_stack_pointer () + 16;
	int i;

	dmd_alloc_low_clean ();

	sprintf ("ERRNO %i", error_code);
	font_render_string_center (&font_5x5, 64, 2, sprintf_buffer);

	for (i=0; i < 8; i++)
	{
		sprintf ("%02x", stack[i]);
		font_render_string (&font_5x5, i*16, 10, sprintf_buffer);
	}

	dmd_show_low ();
	task_dump ();
	for (;;);
}


void do_nonfatal (uint16_t pc, errcode_t error_code)
{
}


void irq_init (void)
{
	irq_count = 0;
}


#pragma naked
void do_reset (void) __noreturn__
{
	register uint8_t *ramptr asm ("x");

	extern void lamp_demo ();
	extern void test_start (void);
	extern void test_init (void);

	set_direct_page_pointer (0);

	/* Initialize the stack pointer.  We can now make
	 * function calls!  Note that this stack is only used
	 * for execution that is not task-based.  Once tasks
	 * can be run, each task will use its own stack pointer
	 * separate from this one. */
	set_stack_pointer (STACK_BASE);

	/* Initialize RAM to all zeroes */
	ramptr = (uint8_t *)USER_RAM_SIZE;
	do
	{
		*ramptr-- = 0;
	} while (ramptr != 0);

	/* Set up protected RAM */
	wpc_set_ram_protect (RAM_UNLOCKED);
	wpc_set_ram_protect_size (RAM_LOCK_512);
	wpc_set_ram_protect (RAM_LOCK_512);

	/* Initialize the ROM page register so that the
	 * page of ROM adjacent to the system area is mapped. */
	wpc_set_rom_page (0x3D);

	sys_init_complete = 0;
	
	wpc_led_toggle ();
#ifdef DEBUGGER
	db_init ();
#endif
	sol_init ();
	triac_init ();
	dmd_init ();
	switch_init ();
	flipper_init ();
	sound_init ();
	lamp_init ();

	device_init ();
	trough_init ();

	wpc_led_toggle ();
	irq_init ();
	task_init ();
	deff_init ();
	test_init ();
	score_init ();
	call_hook (init);

	*(uint8_t *)WPC_ZEROCROSS_IRQ_CLEAR = 0x06;

	sys_init_complete++;

	wpc_led_toggle ();

	task_create_gid (GID_DEVICE_PROBE, device_probe);

	amode_start ();
	task_exit ();
}


void lockup_check_rtt (void)
{
	static U8 last_idle_count = 0;

	if (last_idle_count != 0)
	{
		if (last_idle_count == task_idle_count)
		{
			do_reset ();
		}
	}
	last_idle_count = task_idle_count;
}


#pragma interrupt
void do_irq (void)
{
	*(uint8_t *)WPC_ZEROCROSS_IRQ_CLEAR = 0x96;

	/* Execute rtts every 1ms */
#ifdef DEBUGGER
	db_rtt ();
#endif
	irq_count++;
	sol_rtt ();
	flipper_rtt ();
	if (irq_count & 0x1)
		lamp_rtt ();
	else
		switch_rtt ();

	if ((irq_count & 7) == 0)
	{
		/* Execute rtts every 8ms */
		tick_count++;
		triac_rtt ();

		if ((tick_count & 3) == 0) /* 4 x 8ms */
		{
			/* Execute rtts every 32ms */
			wpc_led_toggle ();
			sound_rtt ();
			lamp_flash_rtt ();
			lockup_check_rtt ();
		}
	}
}


#pragma interrupt
void do_firq (void)
{
	if (*(int8_t *)WPC_PERIPHERAL_TIMER_FIRQ_CLEAR < 0)
	{
		/* Timer interrupt */
	}
	else
	{
		/* DMD interrupt */
		dmd_rtt ();
	}

	*(uint8_t *)WPC_PERIPHERAL_TIMER_FIRQ_CLEAR  = 0;
}


#pragma interrupt
void do_nmi (void)
{
	fatal (ERR_NMI);
}


#pragma interrupt
void do_swi (void)
{
	fatal (ERR_SWI);
}


#pragma interrupt
void do_swi2 (void)
{
	fatal (ERR_SWI2);
}


#pragma interrupt
void do_swi3 (void)
{
	fatal (ERR_SWI3);
}


