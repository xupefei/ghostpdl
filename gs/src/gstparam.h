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
/* Transparency parameter definitions */

#ifndef gstparam_INCLUDED
#  define gstparam_INCLUDED

#include "gsccolor.h"
#include "gsrefct.h"

/* Define the names of the known blend modes. */
typedef enum {
    BLEND_MODE_Compatible,
    BLEND_MODE_Normal,
    BLEND_MODE_Multiply,
    BLEND_MODE_Screen,
    BLEND_MODE_Difference,
    BLEND_MODE_Darken,
    BLEND_MODE_Lighten,
    BLEND_MODE_ColorDodge,
    BLEND_MODE_ColorBurn,
    BLEND_MODE_Exclusion,
    BLEND_MODE_HardLight,
    BLEND_MODE_Overlay,
    BLEND_MODE_SoftLight,
    BLEND_MODE_Luminosity,
    BLEND_MODE_Hue,
    BLEND_MODE_Saturation,
    BLEND_MODE_Color
#define MAX_BLEND_MODE BLEND_MODE_Color
} gs_blend_mode_t;
#define GS_BLEND_MODE_NAMES\
  "Compatible", "Normal", "Multiply", "Screen", "Difference",\
  "Darken", "Lighten", "ColorDodge", "ColorBurn", "Exclusion",\
  "HardLight", "Overlay", "SoftLight", "Luminosity", "Hue",\
  "Saturation", "Color"

/* Define the common part for a transparency stack state. */
typedef enum {
    TRANSPARENCY_STATE_Group = 1,	/* must not be 0 */
    TRANSPARENCY_STATE_Mask
} gs_transparency_state_type_t;
#define GS_TRANSPARENCY_STATE_COMMON\
    gs_transparency_state_t *saved;\
    gs_transparency_state_type_t type
typedef struct gs_transparency_state_s gs_transparency_state_t;
struct gs_transparency_state_s {
    GS_TRANSPARENCY_STATE_COMMON;
};

/* Define the common part for a cached transparency mask. */
#define GS_TRANSPARENCY_MASK_COMMON\
    rc_header rc
typedef struct gs_transparency_mask_s {
    GS_TRANSPARENCY_MASK_COMMON;
} gs_transparency_mask_t;

/* Define the parameter structure for a transparency group. */
#ifndef gs_color_space_DEFINED
#  define gs_color_space_DEFINED
typedef struct gs_color_space_s gs_color_space;
#endif
/* (Update gs_trans_group_params_init if these change.) */
typedef struct gs_transparency_group_params_s {
    const gs_color_space *ColorSpace;
    bool Isolated;
    bool Knockout;
} gs_transparency_group_params_t;

/* Define the parameter structure for a transparency mask. */
typedef enum {
    TRANSPARENCY_MASK_Alpha,
    TRANSPARENCY_MASK_Luminosity
} gs_transparency_mask_subtype_t;
#define GS_TRANSPARENCY_MASK_SUBTYPE_NAMES\
  "Alpha", "Luminosity"
/* (Update gs_trans_mask_params_init if these change.) */
typedef struct gs_transparency_mask_params_s {
    gs_transparency_mask_subtype_t subtype;
    bool has_Background;
    float Background[GS_CLIENT_COLOR_MAX_COMPONENTS];
    int (*TransferFunction)(P3(floatp in, float *out, void *proc_data));
    void *TransferFunction_data;
} gs_transparency_mask_params_t;

/* Select the opacity or shape parameters. */
typedef enum {
    TRANSPARENCY_CHANNEL_Opacity = 0,
    TRANSPARENCY_CHANNEL_Shape = 1
} gs_transparency_channel_selector_t;

#endif /* gstparam_INCLUDED */
