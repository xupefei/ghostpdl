/* Copyright (C) 1998, 1999 Aladdin Enterprises.  All rights reserved.
  
  This software is licensed to a single customer by Artifex Software Inc.
  under the terms of a specific OEM agreement.
*/

/*$RCSfile$ $Revision$ */
/* DCTDecode filter parameter setting and reading */
#include "std.h"
#include "jpeglib_.h"
#include "gserror.h"
#include "gserrors.h"
#include "gstypes.h"
#include "gsmemory.h"
#include "gsparam.h"
#include "strimpl.h"		/* sdct.h requires this */
#include "sdct.h"
#include "sdcparam.h"
#include "sjpeg.h"

/* ================ Get parameters ================ */

stream_state_proc_get_params(s_DCTD_get_params, stream_DCT_state);	/* check */

int
s_DCTD_get_params(gs_param_list * plist, const stream_DCT_state * ss, bool all)
{
    stream_DCT_state dcts_defaults;
    const stream_DCT_state *defaults;

    if (all)
	defaults = 0;
    else {
	(*s_DCTE_template.set_defaults) ((stream_state *) & dcts_defaults);
	defaults = &dcts_defaults;
    }
/****** NYI ******/
    return s_DCT_get_params(plist, ss, defaults);
}

/* ================ Put parameters ================ */

stream_state_proc_put_params(s_DCTD_put_params, stream_DCT_state);	/* check */

int
s_DCTD_put_params(gs_param_list * plist, stream_DCT_state * pdct)
{
    int code;

    if ((code = s_DCT_put_params(plist, pdct)) < 0 ||
    /*
     * DCTDecode accepts quantization and huffman tables
     * in case these tables have been omitted from the datastream.
     */
	(code = s_DCT_put_huffman_tables(plist, pdct, false)) < 0 ||
	(code = s_DCT_put_quantization_tables(plist, pdct, false)) < 0
	)
	DO_NOTHING;
    return code;
}
