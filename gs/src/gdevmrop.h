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
/* Definitions for device RasterOp implementations. */
/* Requires gxdevmem.h, gsropt.h */

#ifndef gdevmrop_INCLUDED
#  define gdevmrop_INCLUDED

/*
 * Compute the effective RasterOp for the 1-bit case,
 * taking transparency into account.
 */
gs_rop3_t gs_transparent_rop(P1(gs_logical_operation_t lop));

#ifdef DEBUG
/* Trace a [strip_]copy_rop call. */
void trace_copy_rop(P16(const char *cname, gx_device * dev,
			const byte * sdata, int sourcex, uint sraster,
			gx_bitmap_id id, const gx_color_index * scolors,
			const gx_strip_bitmap * textures,
			const gx_color_index * tcolors,
			int x, int y, int width, int height,
			int phase_x, int phase_y, gs_logical_operation_t lop));
#endif

/*
 * PostScript colors normally act as the texture for RasterOp, with a null
 * (all zeros) source.  For images with CombineWithColor = true, we need
 * a way to use the image data as the source.  We implement this with a
 * device that applies RasterOp with a specified texture to drawing
 * operations, treating the drawing color as source rather than texture.
 * The texture is a gx_device_color; it may be any type of color, even a
 * pattern.
 */
#ifndef gx_device_color_DEFINED
#  define gx_device_color_DEFINED
typedef struct gx_device_color_s gx_device_color;
#endif

#ifndef gx_device_rop_texture_DEFINED
#  define gx_device_rop_texture_DEFINED
typedef struct gx_device_rop_texture_s gx_device_rop_texture;
#endif

struct gx_device_rop_texture_s {
    gx_device_forward_common;
    gs_logical_operation_t log_op;
    gx_device_color texture;
};

#define private_st_device_rop_texture()	/* in gdevrops.c */\
  gs_private_st_composite_use_final(st_device_rop_texture,\
    gx_device_rop_texture, "gx_device_rop_texture",\
    device_rop_texture_enum_ptrs, device_rop_texture_reloc_ptrs,\
    gx_device_finalize)

/* Create a RasterOp source device. */
int gx_alloc_rop_texture_device(P3(gx_device_rop_texture ** prsdev,
				   gs_memory_t * mem,
				   client_name_t cname));

/* Initialize a RasterOp source device. */
void gx_make_rop_texture_device(P4(gx_device_rop_texture * rsdev,
				   gx_device * target,
				   gs_logical_operation_t lop,
				   const gx_device_color * texture));

#endif /* gdevmrop_INCLUDED */
