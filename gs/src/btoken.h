/* Copyright (C) 1990, 2000 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* Definitions for Level 2 binary tokens */

#ifndef btoken_INCLUDED
#  define btoken_INCLUDED

/*
 * Define accessors for pointers to the system and user name tables
 * (arrays).  Note that these refer implicitly to i_ctx_p.  Note also
 * that these pointers may be NULL: clients must check this.
 */
#define system_names_p (gs_imemory.space_global->names_array)
#define user_names_p (gs_imemory.space_local->names_array)

/* Create a system or user name table (in the stable memory of mem). */
int create_names_array(P3(ref **ppnames, gs_memory_t *mem,
			  client_name_t cname)); /* in zbseq.c */

/* Convert an object to its representation in a binary object sequence. */
int encode_binary_token(P5(i_ctx_t *i_ctx_p, const ref *obj, long *ref_offset,
			   long *char_offset, byte *str)); /* in iscanbin.c */

/* Define the current binary object format for operators. */
/* This is a ref so that it can be managed properly by save/restore. */
#define ref_binary_object_format_container i_ctx_p
#define ref_binary_object_format\
  (ref_binary_object_format_container->binary_object_format)

#endif /* btoken_INCLUDED */
