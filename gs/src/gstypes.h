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
/* Miscellaneous common types for Ghostscript library */

#ifndef gstypes_INCLUDED
#  define gstypes_INCLUDED

/*
 * Define a type used internally for unique IDs of various kinds
 * (primarily, but not exclusively, character and halftone bitmaps).
 * These IDs bear no relation to any other ID space; we generate them all
 * ourselves.
 */
typedef long gs_id;

#define gs_no_id 0L

/*
 * Define a sensible representation of a string, as opposed to
 * the C char * type (which can't store arbitrary data, represent
 * substrings, or perform concatenation without destroying aliases).
 */
#define GS_STRING_COMMON\
    byte *data;\
    uint size
typedef struct gs_string_s {
    GS_STRING_COMMON;
} gs_string;
#define GS_CONST_STRING_COMMON\
    const byte *data;\
    uint size
typedef struct gs_const_string_s {
    GS_CONST_STRING_COMMON;
} gs_const_string;

/*
 * Since strings are allocated differently from ordinary objects, define a
 * structure that can reference either a string (if bytes == 0) or a byte
 * object (if bytes != 0, in which case data+size point within the object).
 *
 * Note: for garbage collection purposes, the string_common members must
 * come first.
 */
typedef struct gs_bytestring_s {
    GS_STRING_COMMON;
    byte *bytes;		/* see above */
} gs_bytestring;
typedef struct gs_const_bytestring_s {
    GS_CONST_STRING_COMMON;
    const byte *bytes;		/* see above */
} gs_const_bytestring;

#define gs_bytestring_from_string(pbs, dat, siz)\
  ((pbs)->data = (dat), (pbs)->size = (siz), (pbs)->bytes = 0)
#define gs_bytestring_from_bytes(pbs, byts, offset, siz)\
  ((pbs)->data = ((pbs)->bytes = (byts)) + (offset), (pbs)->size = (siz))

/*
 * Define types for Cartesian points.
 */
typedef struct gs_point_s {
    double x, y;
} gs_point;
typedef struct gs_int_point_s {
    int x, y;
} gs_int_point;

/*
 * Define a scale for oversampling.  Clients don't actually use this,
 * but this seemed like the handiest place for it.
 */
typedef struct gs_log2_scale_point_s {
    int x, y;
} gs_log2_scale_point;

/*
 * Define types for rectangles in the Cartesian plane.
 * Note that rectangles are half-open, i.e.: their width is
 * q.x-p.x and their height is q.y-p.y; they include the points
 * (x,y) such that p.x<=x<q.x and p.y<=y<q.y.
 */
typedef struct gs_rect_s {
    gs_point p, q;		/* origin point, corner point */
} gs_rect;
typedef struct gs_int_rect_s {
    gs_int_point p, q;
} gs_int_rect;

#endif /* gstypes_INCLUDED */
