/* Copyright (C) 1998 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* DCT filter parameter setting and reading interface */

#ifndef sdcparam_INCLUDED
#  define sdcparam_INCLUDED

/*
 * All of these procedures are defined in sdcparam.c and are only for
 * internal use (by sddparam.c and sdeparam.c), so they are not
 * documented here.
 */

int s_DCT_get_params(P3(gs_param_list * plist, const stream_DCT_state * ss,
			const stream_DCT_state * defaults));
int s_DCT_get_quantization_tables(P4(gs_param_list * plist,
				     const stream_DCT_state * pdct,
				     const stream_DCT_state * defaults,
				     bool is_encode));
int s_DCT_get_huffman_tables(P4(gs_param_list * plist,
				const stream_DCT_state * pdct,
				const stream_DCT_state * defaults,
				bool is_encode));

int s_DCT_byte_params(P5(gs_param_list * plist, gs_param_name key, int start,
			 int count, UINT8 * pvals));
int s_DCT_put_params(P2(gs_param_list * plist, stream_DCT_state * pdct));
int s_DCT_put_quantization_tables(P3(gs_param_list * plist,
				     stream_DCT_state * pdct,
				     bool is_encode));
int s_DCT_put_huffman_tables(P3(gs_param_list * plist, stream_DCT_state * pdct,
				bool is_encode));

#endif /* sdcparam_INCLUDED */
