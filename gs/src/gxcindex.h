/* Portions Copyright (C) 2001 artofcode LLC.
   Portions Copyright (C) 1996, 2001 Artifex Software Inc.
   Portions Copyright (C) 1988, 2000 Aladdin Enterprises.
   This software is based in part on the work of the Independent JPEG Group.
   All Rights Reserved.

   This software is distributed under license and may not be copied, modified
   or distributed except as expressly authorized under the terms of that
   license.  Refer to licensing information at http://www.artifex.com/ or
   contact Artifex Software, Inc., 101 Lucas Valley Road #110,
   San Rafael, CA  94903, (415)492-9861, for further information. */

/*$RCSfile$ $Revision$ */
/* Define the device color index type and macros */

#ifndef gxcindex_INCLUDED
#  define gxcindex_INCLUDED

#include "gsbitops.h"		/* for sample_store macros */

/*
 * Define the maximum number of components in a device color.
 * The minimum value is 4, to handle CMYK; the maximum value is
 * sizeof(gx_color_index) * 8, since for larger values, there aren't enough
 * bits in a gx_color_index to have even 1 bit per component.
 */
#define GX_DEVICE_COLOR_MAX_COMPONENTS 6

/*
 * We might change gx_color_index to a pointer or a structure in the
 * future.  These disabled options help us assess how much disruption
 * such a change might cause.
 */
/*#define TEST_CINDEX_POINTER*/
/*#define TEST_CINDEX_STRUCT*/

/*
 * Internally, a (pure) device color is represented by opaque values of
 * type gx_color_index, which are tied to the specific device.  The driver
 * maps between these values and RGB[alpha] or CMYK values.  In this way,
 * the driver can convert RGB values to its most natural color representation,
 * and have the graphics library cache the result.
 */

#ifdef TEST_CINDEX_STRUCT

/* Define the type for device color index (pixel value) data. */
typedef struct { ulong value[2]; } gx_color_index_data;

#else  /* !TEST_CINDEX_STRUCT */

/* Define the type for device color index (pixel value) data. */
typedef ulong gx_color_index_data;

#endif /* (!)TEST_CINDEX_STRUCT */

#ifdef TEST_CINDEX_POINTER

/* Define the type for device color indices (pixel values). */
typedef gx_color_index_data * gx_color_index;
#define arch_sizeof_color_index arch_sizeof_ptr

extern const gx_color_index_data gx_no_color_index_data;
#define gx_no_color_index_values (&gx_no_color_index_data)
#define gx_no_color_index (&gx_no_color_index_data)

#else  /* !TEST_CINDEX_POINTER */

/* Define the type for device color indices (pixel values). */
typedef gx_color_index_data gx_color_index;
#define arch_sizeof_color_index arch_sizeof_long

/* Define the 'transparent' color index. */
#define gx_no_color_index_value (-1)	/* no cast -> can be used in #if */

/* The SGI C compiler provided with Irix 5.2 gives error messages */
/* if we use the proper definition of gx_no_color_index: */
/*#define gx_no_color_index ((gx_color_index)gx_no_color_index_value) */
/* Instead, we must spell out the typedef: */
#define gx_no_color_index ((unsigned long)gx_no_color_index_value)

#endif /* (!)TEST_CINDEX_POINTER */

/*
 * Define macros for accumulating a scan line of a colored image.
 * The usage is as follows:
 *	DECLARE_LINE_ACCUM(line, bpp, xo);
 *	for ( x = xo; x < xe; ++x ) {
 *	    << compute color at x >>
 *          LINE_ACCUM(color, bpp);
 *      }
 * This code must be enclosed in { }, since DECLARE_LINE_ACCUM declares
 * variables.  Supported values of bpp are 1, 2, 4, 8, 12, 16, 24, 32.
 *
 * Note that DECLARE_LINE_ACCUM declares the variables l_dptr, l_dbyte, and
 * l_dbit.  Other code in the loop may use these variables.
 */
#define DECLARE_LINE_ACCUM(line, bpp, xo)\
	sample_store_declare_setup(l_dptr, l_dbit, l_dbyte, line, 0, bpp)
#define LINE_ACCUM(color, bpp)\
	sample_store_next32(color, l_dptr, l_dbit, bpp, l_dbyte)
#define LINE_ACCUM_SKIP(bpp)\
	sample_store_skip_next(l_dptr, l_dbit, bpp, l_dbyte)
#define LINE_ACCUM_STORE(bpp)\
	sample_store_flush(l_dptr, l_dbit, bpp, l_dbyte)
/*
 * Declare additional macros for accumulating a scan line with copying
 * to a device.  Note that DECLARE_LINE_ACCUM_COPY also declares l_xprev.
 * LINE_ACCUM_COPY is called after the accumulation loop.
 */
#define DECLARE_LINE_ACCUM_COPY(line, bpp, xo)\
	DECLARE_LINE_ACCUM(line, bpp, xo);\
	int l_xprev = (xo)
#define LINE_ACCUM_COPY(dev, line, bpp, xo, xe, raster, y)\
	if ( (xe) > l_xprev ) {\
	    int code;\
	    LINE_ACCUM_STORE(bpp);\
	    code = (*dev_proc(dev, copy_color))\
	      (dev, line, l_xprev - (xo), raster,\
	       gx_no_bitmap_id, l_xprev, y, (xe) - l_xprev, 1);\
	    if ( code < 0 )\
	      return code;\
	}

#endif /* gxcindex_INCLUDED */
