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
/* CCITTFax filter parameter setting and reading */
#include "std.h"
#include "gserror.h"
#include "gserrors.h"
#include "gstypes.h"
#include "gsmemory.h"
#include "gsparam.h"
#include "scommon.h"
#include "scf.h"		/* for cfe_max_width */
#include "scfx.h"

/* Define the CCITTFax parameters. */
private const gs_param_item_t s_CF_param_items[] =
{
#define cfp(key, type, memb) { key, type, offset_of(stream_CF_state, memb) }
    cfp("Uncompressed", gs_param_type_bool, Uncompressed),
    cfp("K", gs_param_type_int, K),
    cfp("EndOfLine", gs_param_type_bool, EndOfLine),
    cfp("EncodedByteAlign", gs_param_type_bool, EncodedByteAlign),
    cfp("Columns", gs_param_type_int, Columns),
    cfp("Rows", gs_param_type_int, Rows),
    cfp("EndOfBlock", gs_param_type_bool, EndOfBlock),
    cfp("BlackIs1", gs_param_type_bool, BlackIs1),
    cfp("DamagedRowsBeforeError", gs_param_type_int, DamagedRowsBeforeError),
    cfp("FirstBitLowOrder", gs_param_type_bool, FirstBitLowOrder),
    cfp("DecodedByteAlign", gs_param_type_int, DecodedByteAlign),
#undef cfp
    gs_param_item_end
};

/* Define a limit on the Rows parameter, close to max_int. */
#define cf_max_height 32000

/* Get non-default CCITTFax filter parameters. */
stream_state_proc_get_params(s_CF_get_params, stream_CF_state);		/* check */
int
s_CF_get_params(gs_param_list * plist, const stream_CF_state * ss, bool all)
{
    stream_CF_state cfs_defaults;
    const stream_CF_state *defaults;

    if (all)
	defaults = 0;
    else {
	s_CF_set_defaults_inline(&cfs_defaults);
	defaults = &cfs_defaults;
    }
    return gs_param_write_items(plist, ss, defaults, s_CF_param_items);
}

/* Put CCITTFax filter parameters. */
stream_state_proc_put_params(s_CF_put_params, stream_CF_state);		/* check */
int
s_CF_put_params(gs_param_list * plist, stream_CF_state * ss)
{
    stream_CF_state state;
    int code;

    state = *ss;
    code = gs_param_read_items(plist, (void *)&state, s_CF_param_items);
    if (code >= 0 &&
	(state.K < -cf_max_height || state.K > cf_max_height ||
	 state.Columns < 0 || state.Columns > cfe_max_width ||
	 state.Rows < 0 || state.Rows > cf_max_height ||
	 state.DamagedRowsBeforeError < 0 ||
	 state.DamagedRowsBeforeError > cf_max_height ||
	 state.DecodedByteAlign < 1 || state.DecodedByteAlign > 16 ||
	 (state.DecodedByteAlign & (state.DecodedByteAlign - 1)) != 0)
	)
	code = gs_note_error(gs_error_rangecheck);
    if (code >= 0)
	*ss = state;
    return code;
}
