/* Copyright (C) 1996, 1999 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* Definitions for PNGPredictor filters */
/* Requires strimpl.h */

#ifndef spngpx_INCLUDED
#  define spngpx_INCLUDED

/* PNGPredictorDecode / PNGPredictorEncode */
typedef struct stream_PNGP_state_s {
    stream_state_common;
    /* The client sets the following before initialization. */
    int Colors;			/* # of colors, 1..16 */
    int BitsPerComponent;	/* 1, 2, 4, 8, 16 */
    uint Columns;		/* >0 */
    int Predictor;		/* 10-15, only relevant for Encode */
    /* The init procedure computes the following. */
    uint row_count;		/* # of bytes per row */
    byte end_mask;		/* mask for left-over bits in last byte */
    int bpp;			/* bytes per pixel */
    byte *prev_row;		/* previous row */
    int case_index;		/* switch index for case dispatch, */
				/* set dynamically when decoding */
    /* The following are updated dynamically. */
    long row_left;		/* # of bytes left in row */
    byte prev[32];		/* previous samples */
} stream_PNGP_state;

#define private_st_PNGP_state()	/* in sPNGP.c */\
  gs_private_st_ptrs1(st_PNGP_state, stream_PNGP_state,\
    "PNGPredictorEncode/Decode state", pngp_enum_ptrs, pngp_reloc_ptrs,\
    prev_row)
#define s_PNGP_set_defaults_inline(ss)\
  ((ss)->Colors = 1, (ss)->BitsPerComponent = 8, (ss)->Columns = 1,\
   (ss)->Predictor = 15,\
		/* Clear pointers */\
   (ss)->prev_row = 0)
extern const stream_template s_PNGPD_template;
extern const stream_template s_PNGPE_template;

#endif /* spngpx_INCLUDED */
