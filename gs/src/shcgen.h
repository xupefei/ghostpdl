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
/* Interface for Huffman code generation */
/* Requires shc.h */

#ifndef shcgen_INCLUDED
#  define shcgen_INCLUDED

/* Compute an optimal Huffman code from an input data set. */
/* The client must have set all the elements of *def. */
/* The definition is guaranteed to be well-behaved. */
int hc_compute(P3(hc_definition * def, const long *freqs, gs_memory_t * mem));

/* Convert a definition to a byte string. */
/* The caller must provide the byte string, of length def->num_values. */
/* Assume (do not check) that the definition is well-behaved. */
/* Return the actual length of the string. */
int hc_bytes_from_definition(P2(byte * dbytes, const hc_definition * def));

/* Extract num_counts and num_values from a byte string. */
void hc_sizes_from_bytes(P3(hc_definition * def, const byte * dbytes, int num_bytes));

/* Convert a byte string back to a definition. */
/* The caller must initialize *def, including allocating counts and values. */
void hc_definition_from_bytes(P2(hc_definition * def, const byte * dbytes));

/* Generate the encoding table from the definition. */
/* The size of the encode array is def->num_values. */
void hc_make_encoding(P2(hce_code * encode, const hc_definition * def));

/* Calculate the size of the decoding table. */
uint hc_sizeof_decoding(P2(const hc_definition * def, int initial_bits));

/* Generate the decoding tables. */
void hc_make_decoding(P3(hcd_code * decode, const hc_definition * def,
			 int initial_bits));

#endif /* shcgen_INCLUDED */
