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
/* Type 0 (composite) font data definition */

#ifndef gxfont0_INCLUDED
#  define gxfont0_INCLUDED

/* Define the composite font mapping types. */
/* These numbers must be the same as the values of FMapType */
/* in type 0 font dictionaries. */
typedef enum {
    fmap_8_8 = 2,
    fmap_escape = 3,
    fmap_1_7 = 4,
    fmap_9_7 = 5,
    fmap_SubsVector = 6,
    fmap_double_escape = 7,
    fmap_shift = 8,
    fmap_CMap = 9
} fmap_type;

#define fmap_type_min 2
#define fmap_type_max 9
#define fmap_type_is_modal(fmt)\
  ((fmt) == fmap_escape || (fmt) == fmap_double_escape || (fmt) == fmap_shift)

/* This is the type-specific information for a type 0 (composite) gs_font. */
#ifndef gs_cmap_DEFINED
#  define gs_cmap_DEFINED
typedef struct gs_cmap_s gs_cmap_t;
#endif
typedef struct gs_type0_data_s {
    fmap_type FMapType;
    byte EscChar, ShiftIn, ShiftOut;
    gs_const_string SubsVector;	/* fmap_SubsVector only */
    uint subs_size;		/* bytes per entry */
    uint subs_width;		/* # of entries */
    uint *Encoding;
    uint encoding_size;
    gs_font **FDepVector;
    uint fdep_size;
    const gs_cmap_t *CMap;	/* fmap_CMap only */
} gs_type0_data;

#define gs_type0_data_max_ptrs 3

typedef struct gs_font_type0_s {
    gs_font_common;
    gs_type0_data data;
} gs_font_type0;

extern_st(st_gs_font_type0);
#define public_st_gs_font_type0()	/* in gsfont0.c */\
  gs_public_st_complex_only(st_gs_font_type0, gs_font_type0, "gs_font_type0",\
    0, font_type0_enum_ptrs, font_type0_reloc_ptrs, gs_font_finalize)

/* Define the Type 0 font procedures. */
font_proc_define_font(gs_type0_define_font);
font_proc_make_font(gs_type0_make_font);
font_proc_init_fstack(gs_type0_init_fstack);
font_proc_next_char_glyph(gs_type0_next_char_glyph);

#endif /* gxfont0_INCLUDED */
