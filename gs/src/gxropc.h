/* Copyright (C) 1998 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* Internals for RasterOp compositing */

#ifndef gxropc_INCLUDED
#  define gxropc_INCLUDED

#include "gsropc.h"
#include "gxcomp.h"

/* Define RasterOp-compositing objects. */
typedef struct gs_composite_rop_s {
    gs_composite_common;
    gs_composite_rop_params_t params;
} gs_composite_rop_t;

#define private_st_composite_rop() /* in gsropc.c */\
  gs_private_st_ptrs1(st_composite_rop, gs_composite_rop_t,\
    "gs_composite_rop_t", composite_rop_enum_ptrs, composite_rop_reloc_ptrs,\
    params.texture)

/*
 * Initialize a RasterOp compositing function from parameters.
 * We make this visible so that clients can allocate gs_composite_rop_t
 * objects on the stack, to reduce memory manager overhead.
 */
void gx_init_composite_rop(P2(gs_composite_rop_t * pcte,
			      const gs_composite_rop_params_t * params));

#endif /* gxropc_INCLUDED */
