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
/* Separation color space support */
#include "memory_.h"
#include "ghost.h"
#include "oper.h"
#include "gsstruct.h"
#include "gscolor.h"
#include "gsmatrix.h"		/* for gxcolor2.h */
#include "gscsepr.h"
#include "gxcspace.h"
#include "gxfixed.h"		/* ditto */
#include "gxcolor2.h"
#include "estack.h"
#include "ialloc.h"
#include "icsmap.h"
#include "ifunc.h"
#include "igstate.h"
#include "iname.h"
#include "ivmspace.h"
#include "store.h"

/* Imported from gscsepr.c */
extern const gs_color_space_type gs_color_space_type_Separation;

/* Forward references */
private int separation_map1(P1(i_ctx_t *));

/* Define the separation cache size.  This makes many useful tint values */
/* map to integer cache indices. */
#define SEPARATION_CACHE_SIZE 360

/* Tint transform procedure that just consults the cache. */
private int
lookup_tint(const gs_separation_params * params, floatp tint, float *values)
{
    int m = cs_num_components((const gs_color_space *)&params->alt_space);
    const gs_indexed_map *map = params->map;
    int value_index =
	(tint < 0 ? 0 : tint > 1 ? map->num_values - m :
	 (int)(tint * SEPARATION_CACHE_SIZE + 0.5) * m);
    const float *pv = &map->values[value_index];

    memcpy(values, pv, sizeof(*values) * m);
    return 0;
}

/* <array> .setseparationspace - */
/* The current color space is the alternate space for the separation space. */
private int
zsetseparationspace(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    const ref *pcsa;
    gs_color_space cs;
    ref_colorspace cspace_old;
    uint edepth = ref_stack_count(&e_stack);
    gs_indexed_map *map;
    ref sname;
    int code;

    check_read_type(*op, t_array);
    if (r_size(op) != 4)
	return_error(e_rangecheck);
    pcsa = op->value.const_refs + 1;
    sname = *pcsa;
    switch (r_type(&sname)) {
	default:
	    return_error(e_typecheck);
	case t_string:
	    code = name_from_string(&sname, &sname);
	    if (code < 0)
		return code;
	    /* falls through */
	case t_name:
	    break;
    }
    check_proc(pcsa[2]);
    cs = *gs_currentcolorspace(igs);
    if (!cs.type->can_be_alt_space)
	return_error(e_rangecheck);
    code = zcs_begin_map(i_ctx_p, &map, &pcsa[2], SEPARATION_CACHE_SIZE + 1,
			 (const gs_direct_color_space *)&cs,
			 separation_map1);
    if (code < 0)
	return code;
    map->proc.tint_transform = lookup_tint;
    /* See zcsindex.c for why we use memmove here. */
    memmove(&cs.params.separation.alt_space, &cs,
	    sizeof(cs.params.separation.alt_space));
    gs_cspace_init(&cs, &gs_color_space_type_Separation, NULL);
    cs.params.separation.sname = name_index(&sname);
    cs.params.separation.map = map;
    cspace_old = istate->colorspace;
    istate->colorspace.procs.special.separation.layer_name = pcsa[0];
    istate->colorspace.procs.special.separation.tint_transform = pcsa[2];
    code = gs_setcolorspace(igs, &cs);
    if (code < 0) {
	istate->colorspace = cspace_old;
	ref_stack_pop_to(&e_stack, edepth);
	return code;
    }
    pop(1);
    return (ref_stack_count(&e_stack) == edepth ? 0 : o_push_estack);	/* installation will load the caches */
}

/* Continuation procedure for saving transformed tint values. */
private int
separation_map1(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    es_ptr ep = esp;
    int i = (int)ep[csme_index].value.intval;

    if (i >= 0) {		/* i.e., not first time */
	int m = (int)ep[csme_num_components].value.intval;
	int code = float_params(op, m, &r_ptr(&ep[csme_map], gs_indexed_map)->values[i * m]);

	if (code < 0)
	    return code;
	pop(m);
	op -= m;
	if (i == (int)ep[csme_hival].value.intval) {	/* All done. */
	    /*
	     * If the tint_transform procedure is a Function, recognize it
	     * as such now.
	     */
	    gs_function_t *pfn = ref_function(&ep[csme_proc]);

	    if (pfn)
		gs_cspace_set_sepr_function(gs_currentcolorspace(igs), pfn);
	    esp -= num_csme;
	    return o_pop_estack;
	}
    }
    push(1);
    ep[csme_index].value.intval = ++i;
    make_real(op, i / (float)SEPARATION_CACHE_SIZE);
    make_op_estack(ep + 1, separation_map1);
    ep[2] = ep[csme_proc];	/* tint_transform */
    esp = ep + 2;
    return o_push_estack;
}

/* - currentoverprint <bool> */
private int
zcurrentoverprint(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    push(1);
    make_bool(op, gs_currentoverprint(igs));
    return 0;
}

/* <bool> setoverprint - */
private int
zsetoverprint(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    check_type(*op, t_boolean);
    gs_setoverprint(igs, op->value.boolval);
    pop(1);
    return 0;
}

/* - .currentoverprintmode <int> */
private int
zcurrentoverprintmode(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;

    push(1);
    make_int(op, gs_currentoverprintmode(igs));
    return 0;
}

/* <int> .setoverprintmode - */
private int
zsetoverprintmode(i_ctx_t *i_ctx_p)
{
    os_ptr op = osp;
    int param;
    int code = int_param(op, max_int, &param);

    if (code < 0 || (code = gs_setoverprintmode(igs, param)) < 0)
	return code;
    pop(1);
    return 0;
}

/* ------ Initialization procedure ------ */

const op_def zcssepr_l2_op_defs[] =
{
    op_def_begin_level2(),
    {"0currentoverprint", zcurrentoverprint},
    {"0.currentoverprintmode", zcurrentoverprintmode},
    {"1setoverprint", zsetoverprint},
    {"1.setoverprintmode", zsetoverprintmode},
    {"1.setseparationspace", zsetseparationspace},
		/* Internal operators */
    {"1%separation_map1", separation_map1},
    op_def_end(0)
};
