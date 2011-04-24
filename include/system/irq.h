/*
 * Copyright 2006, 2007, 2008 by Brian Dominy <brian@oddchange.com>
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

#ifndef _SYS_IRQ_H
#define _SYS_IRQ_H

#ifdef __m6809__

/** How to enable/disable the IRQ */
#define disable_irq() m6809_orcc (CC_IRQ)
#define enable_irq() m6809_andcc (~CC_IRQ)

/** How to enable/disable the FIRQ */
#define disable_firq() m6809_orcc (CC_FIRQ)
#define enable_firq() m6809_andcc (~CC_FIRQ)

/** How to enable/disable all interrupts */
#define disable_interrupts() m6809_orcc (CC_IRQ|CC_FIRQ)
#define enable_interrupts() m6809_andcc (~(CC_IRQ|CC_FIRQ))

#else /* __m6809__ */

extern bool linux_irq_enable;
extern bool linux_firq_enable;

#define disable_irq()	linux_irq_enable = FALSE;
#define disable_firq()	linux_firq_enable = FALSE;
#define enable_irq()		linux_irq_enable = TRUE;
#define enable_firq()	linux_firq_enable = TRUE;
#define disable_interrupts()	do { disable_irq(); disable_firq(); } while (0)
#define enable_interrupts()	do { enable_irq(); enable_firq(); } while (0)

#endif /* __m6809__ */

#endif /* _SYS_IRQ_H */
