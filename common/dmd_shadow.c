/*
 * Copyright 2008-2010 by Brian Dominy <brian@oddchange.com>
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
 * \brief Dot-matrix text shadowing
 *
 */

#include <freewpc.h>

/* dmd_shadow() is the low-level routine to generate a shadow copy.
 * It takes an image in the low mapped page, and generates a new
 * image in the high mapped page where all of the pixels are
 * "blurred" outwards.  Inverting this new page produces an outline
 * mask that can be ANDed to give fonts an outline.
 */
#ifdef __m6809__
extern void dmd_shadow (void);
#else
#define dmd_shadow() /* TODO - no C version of shadowing */
#endif


/**
 * Generate a text outline.
 *
 * On input, provide a mono DMD page in the low-mapped buffer.  The
 *    general use case is for this page to have only fonts on it.
 *
 * On output, the high mapped buffer will contain the outline mask.
 *    This page can be ANDed to blacken the pixels needed to form
 *    the outline.
 */
void dmd_text_outline (void)
{
	dmd_shadow ();
	dmd_invert_page (dmd_high_buffer);
	dmd_flip_low_high ();
}


/**
 * Generate a text blur.
 *
 * On input, provide a mono DMD page in the low-mapped buffer.  The
 *    general use case is for this page to have only fonts on it.
 *
 * On output, the high mapped buffer will contain a blurred copy.
 *
 * This function is identical to 'dmd_text_outline' without the
 * inversion.
 */
void dmd_text_blur (void)
{
	dmd_shadow ();
	dmd_flip_low_high ();
}


/**
 * Apply an overlay that contains an outline font onto color pages.
 *
 * On entry, the low/high mapped pages point to a color image, with
 * the low page having the darker bits and the high page having the
 * brighter bits, as usual.  The overlay pages contain a mono
 * font and its outline mask, respectively.  This function applies
 * the overlay text onto the current working pages.
 */
void dmd_overlay_outline (void)
{
	dmd_pagepair_t dst = wpc_dmd_get_mapped ();
	pinio_dmd_window_set (PINIO_DMD_WINDOW_1, DMD_OVERLAY_PAGE+1);
	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, dst.u.first);
	dmd_and_page ();
	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, dst.u.second);
	dmd_and_page ();

	pinio_dmd_window_set (PINIO_DMD_WINDOW_1, DMD_OVERLAY_PAGE);
	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, dst.u.first);
	dmd_or_page ();
	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, dst.u.second);
	dmd_or_page ();

	wpc_dmd_set_mapped (dst);
}


/**
 * Allocate and map a new pair of DMD pages that are initialized
 * with the current mapped contents.
 *
 * This function can be used instead of dmd_alloc_pair() when
 * the new pages need to be initialized with an exact copy of
 * the pages that are already visible.  This is useful when
 * a new image needs to be rendered that is not very different
 * from what is already showing.
 */
void dmd_dup_mapped (void)
{
	dmd_pagepair_t old, new;

	old = wpc_dmd_get_mapped ();
	dmd_alloc_pair ();
	new = wpc_dmd_get_mapped ();

	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, old.u.second);
	dmd_copy_low_to_high ();

	pinio_dmd_window_set (PINIO_DMD_WINDOW_0, old.u.first);
	pinio_dmd_window_set (PINIO_DMD_WINDOW_1, new.u.first);
	dmd_copy_low_to_high ();

	wpc_dmd_set_mapped (new);
}

