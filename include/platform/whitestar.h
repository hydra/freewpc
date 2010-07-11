/*
 * Copyright 2006-2010 by Brian Dominy <brian@oddchange.com>
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
 * \brief Definitions/macros specific to the Whitestar hardware
 */

#ifndef _WHITESTAR_H
#define _WHITESTAR_H



/***************************************************************
 * Memory usage
 ***************************************************************/

#ifdef __m6809__

#define ASM_DECL(name) name asm (#name)

#define AREA_DECL(name) extern U8 ASM_DECL (s_ ## name); extern U8 ASM_DECL (l_ ## name);
#define AREA_BASE(name) (&s_ ## name)
#define AREA_SIZE(name) ((U16)(&l_ ## name))

AREA_DECL(direct)
AREA_DECL(ram)
AREA_DECL(local)
AREA_DECL(heap)
AREA_DECL(stack)
AREA_DECL(permanent)
AREA_DECL(nvram)

#else
/* TODO */
#endif /* __m6809__ */


/** The total size of RAM  -- 8K */
#define RAM_SIZE 			0x2000UL

/** The usable, nonprotected area of RAM -- the first 6K */
#define USER_RAM_SIZE	0x1800UL

/** The base address of the stack */
#define STACK_BASE 		(USER_RAM_SIZE - 0x8)
#define STACK_SIZE      0x200UL

/** The layout of the player local area.
 * There are 5 "copies" of the local area: the lowest address is active
 * for the current player up, and the next 4 are save areas to hold
 * values between players in a multi-player game. */
#define LOCAL_BASE		AREA_BASE(local)
#define LOCAL_SIZE		0x40U

#define LOCAL_SAVE_BASE(p)	(LOCAL_BASE + (LOCAL_SIZE * (p)))

/***************************************************************
 * Whitestar memory map
 ***************************************************************/

#define WS_SOLA                 0x2000   /* done */
#define WS_SOLB                 0x2001   /* done */
#define WS_SOLC                 0x2002   /* done */
#define WS_FLASHERS             0x2003   /* done */
#define WS_FLIP0                0x2004
#define WS_FLIP1                0x2005
#define WS_AUX_OUT              0x2006
#define WS_AUX_IN               0x2007
#define WS_LAMP_COLUMN_STROBE   0x2008   /* done */
#define WS_LAMP_ROW_OUTPUT      0x200A   /* done */
#define WS_AUX_CTRL             0x200B
   #define WS_AUX_GI         0x1   /* 0=GI on, 1=GI off */
	#define WS_AUX_LEFT_POST_SAVE 0x2
	#define WS_AUX_BSTB       0x8
	#define WS_AUX_CSTB       0x10
	#define WS_AUX_DSTB       0x20
	#define WS_AUX_ESTB       0x40
	#define WS_AUX_ASTB       0x80
#define WS_SW_DEDICATED         0x3000  /* done */
   #define WS_DED_LEFT       0x1
	#define WS_DED_LEFT_EOS   0x2
	#define WS_DED_RIGHT      0x4
	#define WS_DED_RIGHT_EOS  0x8
	#define WS_VOLUME_RED     0x20
	#define WS_SERVICE_GREEN  0x40
	#define WS_TEST_BLACK     0x80
#define WS_SW_DIP               0x3100
#define WS_PAGE_LED             0x3200  /* done */
   #define WS_PAGE_MASK      0x3F
   #define WS_LED_MASK       0x80
#define WS_SW_COLUMN_STROBE     0x3300  /* done */
#define WS_SW_ROW_INPUT         0x3400  /* done */
#define WS_PLASMA_STROBE        0x3500
#define WS_PLASMA_DATA          0x3600
#define WS_PLASMA_RESET         0x3601
#define WS_PLASMA_STATUS        0x3700
	#define WS_PLASMA_BUSY      0x80
	#define WS_PLASMA_TX_READY  0x10  /* 1=ok to send */
#define WS_SOUND_OUT            0x3800
   #define WS_SOUND_BUSY       0x1

extern U8 ws_page_led_io;
extern U8 ws_aux_ctrl_io;


extern inline void ws_aux_strobe (const U8 bits)
{
	register U8 reg = ws_aux_ctrl_io;
	reg &= ~bits;
	writeb (WS_AUX_CTRL, reg);
	reg |= bits;
	writeb (WS_AUX_CTRL, reg);
}


/********************************************/
/* LED                                      */
/********************************************/

/** Toggle the diagnostic LED. */
extern inline void pinio_active_led_toggle (void)
{
	ws_page_led_io ^= WS_LED_MASK;
	writeb (WS_PAGE_LED, ws_page_led_io);
}


/********************************************/
/* Printer / Parallel Port                  */
/********************************************/

#undef HAVE_PARALLEL_PORT

extern inline void pinio_parport_write (U8 data)
{
}

/********************************************/
/* NVRAM Protection Circuit                 */
/********************************************/

#define pinio_nvram_unlock()
#define pinio_nvram_lock()

/********************************************/
/* Bank Switching                           */
/********************************************/

#define PINIO_BANK_ROM 0

extern inline void pinio_set_bank (U8 bankno, U8 val)
{
	switch (bankno)
	{
		case PINIO_BANK_ROM:
			ws_page_led_io &= ~WS_PAGE_MASK;
			ws_page_led_io |= val;
			writeb (WS_PAGE_LED, ws_page_led_io);
			break;
		default:
			break;
	}
}

extern inline U8 pinio_get_bank (U8 bankno)
{
	switch (bankno)
	{
		case PINIO_BANK_ROM:
			return ws_page_led_io & WS_PAGE_MASK;
		default:
			return 0;
	}
}


/***************************************************************
 * Flippers
 ***************************************************************/

extern inline U8 wpc_read_flippers (void)
{
	return 0;
}


extern inline void wpc_write_flippers (U8 val)
{
}

extern inline void pinio_enable_flippers (void)
{
}

extern inline void pinio_disable_flippers (void)
{
}


/********************************************/
/* Locale                                   */
/********************************************/


extern inline U8 wpc_get_jumpers (void)
{
	return ~readb (WS_SW_DIP);
}

extern inline U8 pinio_read_locale (void)
{
	return 0;
}


/********************************************/
/* Lamps                                    */
/********************************************/

#define CONFIG_LAMP_STROBE16
#define CONFIG_LAMP_ROW8
#define PINIO_NUM_LAMPS 80

extern inline void pinio_write_lamp_strobe (U16 val)
{
	writeb (WS_LAMP_COLUMN_STROBE, val & 0xFF);
	writeb (WS_LAMP_COLUMN_STROBE+1, val >> 8);
}

extern inline void pinio_write_lamp_data (U8 val)
{
	writeb (WS_LAMP_ROW_OUTPUT, val);
}

/********************************************/
/* Solenoids                                */
/********************************************/

extern inline void pinio_write_solenoid_set (U8 set, U8 val)
{
	switch (set)
	{
	case 0:
		writeb (WS_SOLA, val);
		break;
	case 1:
		writeb (WS_SOLB, val);
		break;
	case 2:
		writeb (WS_SOLC, val);
		break;
	case 3:
		writeb (WS_FLASHERS, val);
		break;
	}
}


/********************************************/
/* Sound                                    */
/********************************************/

extern inline void pinio_reset_sound (void)
{
}

extern inline void pinio_write_sound (U8 val)
{
	writeb (WS_SOUND_OUT, val);
}

extern inline bool pinio_sound_ready_p (void)
{
	return FALSE;
}

extern inline U8 pinio_read_sound (void)
{
	return 0;
}

#define SW_VOLUME_UP SW_GREEN_BUTTON
#define SW_VOLUME_DOWN SW_RED_BUTTON

/********************************************/
/* Switches                                 */
/********************************************/

#define SW_ENTER SW_BLACK_BUTTON
#define SW_UP SW_GREEN_BUTTON
#define SW_DOWN SW_RED_BUTTON

extern inline void pinio_write_switch_column (U8 val)
{
	writeb (WS_SW_COLUMN_STROBE, val);
}

extern inline U8 pinio_read_switch_rows (void)
{
	return ~readb (WS_SW_ROW_INPUT);
}

extern inline U8 pinio_read_dedicated_switches (void)
{
	return ~readb (WS_SW_DEDICATED);
}


/********************************************/
/* Triacs                                   */
/********************************************/

#define PINIO_GI_STRINGS 0x1

extern inline void pinio_write_gi (U8 val)
{
	val = ~val;
	ws_aux_ctrl_io = 0xFE | (val & 0x1);
	writeb (WS_AUX_CTRL, ws_aux_ctrl_io);
}

/********************************************/
/* Miscellaneous                            */
/********************************************/

extern U8 db_paused;

extern inline void wpc_debug_write (U8 val)
{
}

extern inline U8 wpc_debug_read (void)
{
	return 0;
}

extern inline void pinio_watchdog_reset (void)
{
}

extern inline void pinio_clear_periodic (void)
{
}

/********************************************/
/* Dot-Matrix Remote Protocol               */
/********************************************/

#define WPC_DMD_HIGH_PAGE       0x0
#define WPC_DMD_FIRQ_ROW_VALUE  0x0
#define WPC_DMD_LOW_PAGE        0x0
#define WPC_DMD_ACTIVE_PAGE     0x0
extern U8 *pinio_dmd_low_page, *pinio_dmd_high_page;

typedef struct
{
	U8 len;
	U8 options;
	U8 cmd;
	U8 args[8];
} ws_dmd_msg_t;

#endif /* _WHITESTAR_H */

