/* Copyright (C) 1994, 1995, 1999 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* Definitions for PixelDifference filters */
/* Requires strimpl.h */

#ifndef spdiffx_INCLUDED
#  define spdiffx_INCLUDED

/*
 * Define the maximum value for Colors.  This must be at least 4, but can
 * be arbitrarily large: the only cost is a larger stream state structure.
 */
#define s_PDiff_max_Colors 16

/* PixelDifferenceDecode / PixelDifferenceEncode */
typedef struct stream_PDiff_state_s {
    stream_state_common;
    /* The client sets the following before initialization. */
    int Colors;			/* # of colors, 1..s_PDiff_max_Colors */
    int BitsPerComponent;	/* 1, 2, 4, 8 */
    int Columns;
    /* The init procedure computes the following. */
    uint row_count;		/* # of bytes per row */
    byte end_mask;		/* mask for left-over bits in last byte */
    int case_index;		/* switch index for case dispatch */
    /* The following are updated dynamically. */
    uint row_left;		/* # of bytes left in row */
    byte prev[s_PDiff_max_Colors];	/* previous sample */
} stream_PDiff_state;

#define private_st_PDiff_state()	/* in spdiff.c */\
  gs_private_st_simple(st_PDiff_state, stream_PDiff_state,\
    "PixelDifferenceEncode/Decode state")
#define s_PDiff_set_defaults_inline(ss)\
  ((ss)->Colors = 1, (ss)->BitsPerComponent = 8, (ss)->Columns = 1)
extern const stream_template s_PDiffD_template;
extern const stream_template s_PDiffE_template;

#endif /* spdiffx_INCLUDED */
