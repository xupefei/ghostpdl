/* Copyright (C) 1994, 1995, 1996, 1997 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* Definitions for RunLength filters */
/* Requires scommon.h; strimpl.h if any templates are referenced */

#ifndef srlx_INCLUDED
#  define srlx_INCLUDED

/* Common state */
#define stream_RL_state_common\
	stream_state_common;\
	bool EndOfData		/* true if 128 = EOD */

/* RunLengthEncode */
typedef struct stream_RLE_state_s {
    stream_RL_state_common;
    /* The following parameters are set by the client. */
    ulong record_size;
    /* The following change dynamically. */
    ulong record_left;		/* bytes left in current record */
    int copy_left;		/* # of bytes waiting to be copied */
} stream_RLE_state;

#define private_st_RLE_state()	/* in srle.c */\
  gs_private_st_simple(st_RLE_state, stream_RLE_state, "RunLengthEncode state")
/* We define the initialization procedure here, so that clients */
/* can avoid a procedure call. */
#define s_RLE_set_defaults_inline(ss)\
  ((ss)->EndOfData = true, (ss)->record_size = 0)
#define s_RLE_init_inline(ss)\
  ((ss)->record_left =\
   ((ss)->record_size == 0 ? ((ss)->record_size = max_uint) :\
    (ss)->record_size),\
   (ss)->copy_left = 0)
extern const stream_template s_RLE_template;

/* RunLengthDecode */
typedef struct stream_RLD_state_s {
    stream_RL_state_common;
    /* The following change dynamically. */
    int copy_left;		/* # of output bytes waiting to be produced */
    int copy_data;		/* -1 if copying, repeated byte if repeating */
} stream_RLD_state;

#define private_st_RLD_state()	/* in srld.c */\
  gs_private_st_simple(st_RLD_state, stream_RLD_state, "RunLengthDecode state")
/* We define the initialization procedure here, so that clients */
/* can avoid a procedure call. */
#define s_RLD_set_defaults_inline(ss)\
  ((ss)->EndOfData = true)
#define s_RLD_init_inline(ss)\
  ((ss)->min_left = ((ss)->EndOfData ? 1 : 0), (ss)->copy_left = 0)
extern const stream_template s_RLD_template;

#endif /* srlx_INCLUDED */
