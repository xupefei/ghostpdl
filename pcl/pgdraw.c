/* Copyright (C) 1996, 1997 Aladdin Enterprises.  All rights reserved.
   Unauthorized use, copying, and/or distribution prohibited.
 */

/* pgdraw.c */
/* HP-GL/2 line drawing/path building routines. */

#include "stdio_.h"		
#include "math_.h"
#include "gdebug.h"            
#include "gstypes.h"		/* for gsstate.h */
#include "gsmatrix.h"		/* for gsstate.h */
#include "gsmemory.h"		/* for gsstate.h */
#include "gsstate.h"
#include "gscoord.h"
#include "gspath.h"
#include "gspaint.h"
#include "gxfixed.h"
#include "pgmand.h"
#include "pgdraw.h"
#include "pggeom.h"
#include "pgmisc.h"
#include "pcdraw.h"             /* included for setting pcl's ctm */

/* HAS: update ghostscript's gs and the current ctm to reflect hpgl's.
   embodied in hpgl_set_graphics_dash_state(),
   hpgl_set_graphics_line_attributes(), and hpgl_set_graphics_state() below.
   Note: For now the gs gstate is updated each time graphics are
   rendered and the ctm is updated each time a new path is started.
   Potential performance issue.  The design choice is based solely on
   ease of implementation.  */

 private int
hpgl_set_graphics_dash_state(hpgl_state_t *pgls)
{
	bool adaptive = ( pgls->g.line.current.type < 0 );
	int entry = abs(pgls->g.line.current.type);
	hpgl_line_type_t *pat;
	float length;
	float pattern[20];
	int i;

	/* handle the simple case (no dash) and return */
	if ( pgls->g.line.current.is_solid ) 
	  {
	    /* use a 0 count pattern to turn off dashing in case it is
               set */
	    hpgl_call(gs_setdash(pgls->pgs, pattern, 0, 0));
	    return 0;
	  }
	
	if ( entry == 0 )
	  {
	    /* dot length NOTE this is in absolute 1/72" units not
               user space */
	    /* Use an adaptive pattern with an infinitely long segment length
	       to get the dots drawn just at the ends of lines. */
	    pattern[0] = 0;
	    pattern[1] = 1.0e6;	/* "infinity" */
	    hpgl_call(gs_setdash(pgls->pgs, pattern, 2, 0));
	    gs_setdashadapt(pgls->pgs, true);
	    hpgl_call(gs_setdotlength(pgls->pgs, (0.00098), 
				      true));
	    return 0;
	  }

	pat = ((adaptive) ?  
	       (&pgls->g.adaptive_line_type[entry - 1]) :
	       (&pgls->g.fixed_line_type[entry - 1]));
	
	length = ((pgls->g.line.current.pattern_length_relative) ?
		  (pgls->g.line.current.pattern_length * 
		   hpgl_compute_distance(pgls->g.P1.x,
					 pgls->g.P1.y,
					 pgls->g.P2.x,
					 pgls->g.P2.y)) :
		  (mm_2_plu(pgls->g.line.current.pattern_length)));

	gs_setdashadapt(pgls->pgs, adaptive);
	/* HAS not correct as user space may not be PU */
	
	for ( i = 0; i < pat->count; i++ )
	  {
	    pattern[i] = length * pat->gap[i];
	  }
	
	/* HAS does not handle residual / offset yet.  It is
	   not clear where the calculation should take place */
	hpgl_call(gs_setdash(pgls->pgs, pattern, pat->count, 0));
	
	return 0;
}

/* set up joins, caps, miter limit, and line width */
 private int
hpgl_set_graphics_line_attribute_state(hpgl_state_t *pgls,
				       hpgl_rendering_mode_t render_mode)
{

	  /* HAS *** We use a miter join instead of miter/beveled as I
             am not sure if the gs library supports it */
	  const gs_line_cap cap_map[] = {gs_cap_butt,      /* 0 not supported */
					 gs_cap_butt,      /* 1 butt end */
					 gs_cap_square,    /* 2 square end */
					 gs_cap_triangle,  /* 3 triangular end */
					 gs_cap_round};    /* 4 round cap */

	  const gs_line_join join_map[] = {gs_join_none,    /* 0 not supported */
					   gs_join_miter,    /* 1 mitered */
					   gs_join_miter,    /* 2 mitered/beveled */
					   gs_join_triangle, /* 3 triangular join */
					   gs_join_round,    /* 4 round join */
					   gs_join_bevel,    /* 5 bevel join */
					   gs_join_none};    /* 6 no join */

	  switch( render_mode )
	    {
	    case hpgl_rm_character:
	    case hpgl_rm_polygon:
	    case hpgl_rm_vector_fill:
	    case hpgl_rm_clip_and_fill_polygon:
	      hpgl_call(gs_setlinejoin(pgls->pgs, 
				       gs_join_round)); 
	      hpgl_call(gs_setlinecap(pgls->pgs, 
				      gs_cap_round));
	      break;
	    case hpgl_rm_vector:
vector:	      hpgl_call(gs_setlinejoin(pgls->pgs, 
				       join_map[pgls->g.line.join])); 
	      hpgl_call(gs_setlinecap(pgls->pgs, 
				      cap_map[pgls->g.line.cap]));
	      break;
	    default:
	      /* shouldn't happen as we must have a mode to properly
                 parse an hpgl file. */
	      dprintf("warning no hpgl rendering mode set using vector mode\n");
	      goto vector;
	    }

	  /* HAS -- yuck need symbolic names for GL join types.  Set
	     miter limit !very large! if there is not bevel.  Miter is
	     also sensitive to render mode but I have not figured it
	     out completely. */
	  hpgl_call(gs_setmiterlimit(pgls->pgs, 
				     ( pgls->g.line.join == 1 ) ?
				     5000.0 :
				     pgls->g.miter_limit));
	  
	  hpgl_call(gs_setlinewidth(pgls->pgs, 
				    pgls->g.pen_width[pgls->g.pen]));
	  
	  return 0;
}

/* A bounding box for the current polygon -- used for HPGL/2 vector
   fills.  We expand the bounding box by the current line width to
   avoid overhanging lines. */

 private int
hpgl_polyfill_bbox(hpgl_state_t *pgls, gs_rect *bbox)
{
	hpgl_call(gs_pathbbox(pgls->pgs, bbox));
	bbox->p.x -= pgls->g.pen_width[pgls->g.pen];
	bbox->p.y -= pgls->g.pen_width[pgls->g.pen];
	bbox->q.x += pgls->g.pen_width[pgls->g.pen];
	bbox->q.y += pgls->g.pen_width[pgls->g.pen];
	return 0;
}

/* set up an hpgl clipping region wrt last IW command */
 private int
hpgl_set_clipping_region(hpgl_state_t *pgls, hpgl_rendering_mode_t render_mode)
{
	gs_fixed_rect fixed_box;
	gs_rect float_box;
	gs_matrix mat;

	/* use the path for the current clipping region.  Note this is
           incorrect as it should be the intersection of IW and
           the bounding box of the current clip region. */

	/* if we are doing vector fill a clipping path has already
           been set up using the last polygon */
	if ( render_mode == hpgl_rm_vector_fill )
	  return 0;
	else
	  {
	    hpgl_call(gs_currentmatrix(pgls->pgs, &mat));

	    hpgl_call(gs_bbox_transform(&pgls->g.window, 
					&mat,
					&float_box));

	    /* HAS maybe a routine that does this?? */
	    fixed_box.p.x = float2fixed(float_box.p.x);
	    fixed_box.p.y = float2fixed(float_box.p.y);
	    fixed_box.q.x = float2fixed(float_box.q.x);
	    fixed_box.q.y = float2fixed(float_box.q.y);

	    hpgl_call(gx_clip_to_rectangle(pgls->pgs, &fixed_box));
	    
	  }
	return 0;
}

 private int
hpgl_set_plu_to_device_ctm(hpgl_state_t *pgls)
{
	pcl_set_ctm(pgls, false);
	/* translate the coordinate system to the anchor point */
	hpgl_call(gs_translate(pgls->pgs, 
			       pgls->g.picture_frame.anchor_point.x,
			       pgls->g.picture_frame.anchor_point.y));
	/* move the origin */
	hpgl_call(gs_translate(pgls->pgs, 0, pgls->g.picture_frame_height));

	hpgl_call(gs_rotate(pgls->pgs, pgls->g.rotation));
	  /* account for rotated coordinate system */
	switch (pgls->g.rotation)
	  {
	  case 0 :
	    hpgl_call(gs_translate(pgls->pgs, 
				   0, 
				   0));
	    break;
	  case 90 : 
	    hpgl_call(gs_translate(pgls->pgs, 
				   0,
				   pgls->g.picture_frame_height));

	    break;
	  case 180 :
	    hpgl_call(gs_translate(pgls->pgs, 
				   (pgls->g.picture_frame_width), 
				   (pgls->g.picture_frame_height)));
	    break;
	  case 270 :
	    hpgl_call(gs_translate(pgls->pgs, 
				   (pgls->g.picture_frame_width), 
				   0));
	    break;
	  }
	/* scale to plotter units and a flip for y */
	hpgl_call(gs_scale(pgls->pgs, (7200.0/1016.0), -(7200.0/1016.0)));
	/* set up scaling wrt plot size and picture frame size.  HAS
	   we still have the line width issue when scaling is
	   assymetric !!  */
	/* if any of these are zero something is wrong */
        if ( (pgls->g.picture_frame_height == 0) ||
	     (pgls->g.picture_frame_width == 0) ||
	     (pgls->g.plot_width == 0) ||
	     (pgls->g.plot_height == 0) )
	  return 1;
	hpgl_call(gs_scale(pgls->pgs, 
			   ((hpgl_real_t)pgls->g.picture_frame_height /
			    (hpgl_real_t)pgls->g.plot_height),
			   ((hpgl_real_t)pgls->g.picture_frame_width /
			    (hpgl_real_t)pgls->g.plot_width)));
	return 0;
}

 private int
hpgl_set_label_to_plu_ctm(hpgl_state_t *pgls)
{
	/* if we are not in character mode do nothing -- identity
           transformation */
        /* HAS font is selection not complete */
	pcl_font_selection_t *pfs = 
	  &pgls->g.font_selection[pgls->g.font_selected];
	hpgl_real_t height_points = pfs->params.height_4ths / 4.0;
	hpgl_real_t width_points = pl_fp_pitch_cp(&pfs->params) / 100.0;
	hpgl_call(gs_translate(pgls->pgs, pgls->g.pos.x, pgls->g.pos.y));
	hpgl_call(gs_scale(pgls->pgs, 
			   points_2_plu(height_points),
			   points_2_plu(width_points)));
	return 0;
}


 private int
hpgl_set_user_units_to_plu_ctm(hpgl_state_t *pgls)
{

			       
	/* finally scale to user units.  HAS this only handles the
           simple scaling scale for the moment. */
	if ( pgls->g.scaling_type != hpgl_scaling_none )
	  { 
	    
	    floatp scale_x = (pgls->g.P2.x - pgls->g.P1.x) / 
	      (pgls->g.scaling_params.pmax.x - pgls->g.scaling_params.pmin.x);
	    floatp scale_y = (pgls->g.P2.y - pgls->g.P1.y) / 
	      (pgls->g.scaling_params.pmax.y - pgls->g.scaling_params.pmin.y);
	    hpgl_call(gs_translate(pgls->pgs, 
				   pgls->g.P1.x, 
				   pgls->g.P1.y));
	    hpgl_call(gs_scale(pgls->pgs, scale_x, scale_y));
	  }
	return 0;
}

/* set up ctm's.  Uses the current render mode to figure out which ctm
   is appropriate */
 int
hpgl_set_ctm(hpgl_state_t *pgls)
{
	/* convert pcl->device to plu->device */
	hpgl_call(hpgl_set_plu_to_device_ctm(pgls));

	/* concatenate on user units ctm or character ctm based on
           current mode */
	if ( pgls->g.current_render_mode == hpgl_rm_character )
	  hpgl_call(hpgl_set_label_to_plu_ctm(pgls));
	else
	  hpgl_call(hpgl_set_user_units_to_plu_ctm(pgls));

	return 0;
}

/* HAS should replicate lines beginning at the anchor corner to +X and
   +Y.  Not quite right - anchor corner not yet supported.
   pgls->g.anchor_corner needs to be used to set dash offsets */

 private int
hpgl_polyfill(hpgl_state_t *pgls)
{
	hpgl_real_t diag_mag, startx, starty, endx, endy, direction,
	  x_fill_increment, y_fill_increment;
	gs_rect bbox;
	bool cross = (pgls->g.fill.type == hpgl_fill_crosshatch);
	/* get the bounding box */
        hpgl_call(hpgl_polyfill_bbox(pgls, &bbox));
	/* HAS calculate the offset for dashing - we do not need this
           for solid lines */
	/* get rid of the current path */
	hpgl_call(hpgl_clear_current_path(pgls));
	/* HAS calculate the diagonals magnitude.  Note we clip this
           latter in the library.  If desired we could clip to the
           actual bbox here to save path memory.  For now we simply
           draw all fill lines using the diagonals magnitude */
	diag_mag = 
	  hpgl_compute_distance(bbox.p.x, bbox.p.y, bbox.q.x, bbox.q.y);

	/* get the current direction in radians */
	direction = degrees_to_radians * pgls->g.fill.param.hatch.angle;
start:	x_fill_increment = 
	  fabs((pgls->g.fill.param.hatch.spacing) / sin(direction));
	endx = (diag_mag * cos(direction)) + bbox.p.x;
	endy = (diag_mag * sin(direction)) + bbox.p.y;
	startx = bbox.p.x; starty = bbox.p.y;
	/* travel along +x using current spacing */
	do
	  {
	    if ( startx > bbox.q.x ) break; /* done */
	    /* draw the line */
	    {
	      hpgl_args_t args;
	      hpgl_args_set_real(&args, startx);
	      hpgl_args_add_real(&args, starty);
	      hpgl_PU(&args, pgls);
	      hpgl_args_set_real(&args, endx);
	      hpgl_args_add_real(&args, endy);
	      hpgl_PD(&args, pgls);
	    }
	    /* update startx by current spacing */
	    startx += x_fill_increment;
	    endx += x_fill_increment;
	  } 
	while (1);
	hpgl_call(hpgl_draw_current_path(pgls, hpgl_rm_vector_fill));
	/* same for y */
	startx = bbox.p.x; starty = bbox.p.y;
	endx = (diag_mag * cos(direction)) + startx;
	endy = (diag_mag * sin(direction)) + starty;
	y_fill_increment = 
	  fabs((pgls->g.fill.param.hatch.spacing) / cos(direction));
	do
	  {
	    if ( starty > bbox.q.y ) break; /* done */
	    /* draw the line */
	    {
	      hpgl_args_t args;
	      hpgl_args_set_real(&args, startx);
	      hpgl_args_add_real(&args, starty);
	      hpgl_PU(&args, pgls);
	      hpgl_args_set_real(&args, endx);
	      hpgl_args_add_real(&args, endy);
	      hpgl_PD(&args, pgls);
	    }
	    /* update starty by current spacing */
	    starty += y_fill_increment;
	    endy += y_fill_increment;
	  } 
	while (1);
	hpgl_call(hpgl_draw_current_path(pgls, hpgl_rm_vector_fill));
	/* HAS not working yet */
	if ( cross )
	  {
	    cross = false;
	    direction += (M_PI / 2.0);
	    goto start;
	  }
	return 0;
}

/* HAS - probably not necessary and clip intersection with IW */
 private int
hpgl_polyfill_using_current_line_type(hpgl_state_t *pgls)
{
	/* gsave and grestore used to preserve the clipping region */
  	hpgl_call(hpgl_gsave(pgls));
	/* use the current path to set up a clipping path */
	/* beginning at the anchor corner replicate lines */
	hpgl_call(gs_clip(pgls->pgs));
	hpgl_call(hpgl_polyfill(pgls));
	hpgl_call(hpgl_grestore(pgls));
	hpgl_call(hpgl_clear_current_path(pgls));

	return 0;
}	      
/* maps current hpgl fill type to pcl pattern type.  */
 private pcl_pattern_type_t
hpgl_map_fill_type(hpgl_state_t *pgls, hpgl_rendering_mode_t render_mode)
{

	if ( render_mode == hpgl_rm_character )
	  {
	    switch (pgls->g.character.fill_mode)
	      {
	      case 0 : ; return pcpt_solid_black;
	      case 1 : ; /* HAS NOT IMPLEMENTED */
	      case 2 : ; /* HAS NOT IMPLEMENTED */
	      case 3 : ; /* HAS NOT IMPLEMENTED */
		dprintf("hpgl fill should not be mapped\n");
		break;
	      default :
		dprintf1("Unknown character fill mode falling back to solid%d\n",  
			 pgls->g.character.fill_mode);
		break;
	      }
	  }
	else
	  {
	    switch (pgls->g.fill.type)
	      {
	      case hpgl_fill_solid : 
	      case hpgl_fill_solid2 :
	      case hpgl_fill_hatch :
	      case hpgl_fill_crosshatch :
		return pcpt_solid_black;
		break;
	      case hpgl_fill_pcl_crosshatch : return pcpt_cross_hatch;
	      case hpgl_fill_shaded : return pcpt_shading;
	      case hpgl_fill_pcl_user_defined : 
		dprintf("No key mapping support falling back to solid\n");
		break;
	      default : 
		dprintf1("Unsupported fill type %d falling back to solid\n",
			 pgls->g.fill.type);
	      }
	  }
	return pcpt_solid_black;
}

/* HAS I don't much care for the idea of overloading pcl_id with
   shading and hatching information, but that appears to be the way
   pcl_set_drawing_color() is set up. */
 private pcl_id_t *
hpgl_map_id_type(hpgl_state_t *pgls, pcl_id_t *id)
{
	switch (pgls->g.fill.type)
	  {
	  case hpgl_fill_solid : 
	  case hpgl_fill_solid2 :
	  case hpgl_fill_hatch :
	  case hpgl_fill_crosshatch :
	    id = NULL;
	    break;
	  case hpgl_fill_pcl_crosshatch : 
	    id_set_value(*id, pgls->g.fill.param.pattern_type);
	    break;
	  case hpgl_fill_shaded : 
	    id_set_value(*id, (int)((pgls->g.fill.param.shading) * 100.0));
	    break;
	  case hpgl_fill_pcl_user_defined : 
	    id = NULL;
	    dprintf("No key mapping support yet");
	    break;
	  default : 
	    id = NULL;
	    dprintf1("Unsupported fill type %d falling back to pcl solid\n",
		     pgls->g.fill.type);
	  }
	return id;
} 
 
 private int
hpgl_set_drawing_color(hpgl_state_t *pgls, hpgl_rendering_mode_t render_mode)
{
	pcl_id_t pcl_id;
	if ( render_mode == hpgl_rm_vector_fill ) return 0;
	hpgl_call(pcl_set_drawing_color(pgls,
					hpgl_map_fill_type(pgls, 
							   render_mode),
					hpgl_map_id_type(pgls, &pcl_id)));
	if ( render_mode == hpgl_rm_clip_and_fill_polygon )
	  hpgl_call(hpgl_polyfill_using_current_line_type(pgls));
	return 0;
}

 private int
hpgl_set_graphics_state(hpgl_state_t *pgls, hpgl_rendering_mode_t render_mode)
{
        /* HACK to reset the ctm.  Note that in character mode we
           concatenate the character matrix to the ctm, but we do not
           want the tranformation to apply to the line width, miter,
           etc. */
	hpgl_call(hpgl_set_ctm(pgls));

	/* do dash stuff. */
	hpgl_call(hpgl_set_graphics_dash_state(pgls));

	/* joins, caps, and line width. */
	hpgl_call(hpgl_set_graphics_line_attribute_state(pgls, render_mode));
	
	/* set up a clipping region */
	hpgl_call(hpgl_set_clipping_region(pgls, render_mode));

	/* set up the hpgl fills. */
	hpgl_call(hpgl_set_drawing_color(pgls, render_mode));

	return 0;
}

 int
hpgl_get_current_position(hpgl_state_t *pgls, gs_point *pt)
{
	*pt = pgls->g.pos;
	return 0;
}					   

 int
hpgl_set_current_position(hpgl_state_t *pgls, gs_point *pt)
{
	if ( pgls->g.relative )
	  {
	    pgls->g.pos.x += pt->x;
	    pgls->g.pos.y += pt->y;
	  }
	else
	  {
	    pgls->g.pos.x = pt->x;
	    pgls->g.pos.y = pt->y;
	  }
	return 0;
}

/* function to set up a new path.  We assume the current path is
   empty.  Note point is passed by value. */
 private int
hpgl_start_path(hpgl_state_t *pgls, gs_point pt)
{
	/* set up the ctm for this path */
	hpgl_call(hpgl_set_ctm(pgls));
	/* create a new path */
	hpgl_call(gs_newpath(pgls->pgs));

	/* moveto the current position */
	hpgl_call(hpgl_get_current_position(pgls, &pt));
	hpgl_call_check_lost(gs_moveto(pgls->pgs, pt.x, pt.y));
	/* record the first point of the path in gl/2 state so that we
           can implicitly close the path if necessary */
	hpgl_call(hpgl_get_current_position(pgls, &pgls->g.first_point));

	/* if we are in lost indicate that we do not have a path */
	if (!hpgl_lost) pgls->g.have_first_moveto = true;


	
	return 0;
}

 int
hpgl_add_point_to_path(hpgl_state_t *pgls, floatp x, floatp y, 
		       int (*gs_func)(gs_state *pgs, floatp x, floatp y))
{	
        bool new_path = !pgls->g.have_first_moveto;
	gs_point point;
	point.x = x;
	point.y = y;

	if ( new_path ) 
	  hpgl_call(hpgl_start_path(pgls, point));

	hpgl_call_check_lost((*gs_func)(pgls->pgs, point.x, point.y));

	/* update hpgl's state position */
	hpgl_call(hpgl_set_current_position(pgls, &point));

	return 0;
}

/* destroys the current path.  HAS probably don't need to create a new
   one also. */
 int 
hpgl_clear_current_path(hpgl_state_t *pgls)
{
	/* if a current path exists set the current state position */
	hpgl_call(gs_newpath(pgls->pgs));
	pgls->g.have_first_moveto = false;
	return 0;
}

/* closes the current path, making the first point and last point coincident */
 int 
hpgl_close_current_path(hpgl_state_t *pgls)
{
	hpgl_call(gs_closepath(pgls->pgs));
	return 0;
}

/* converts pcl coordinate to device space and back to hpgl space */
 int
hpgl_add_pcl_point_to_path(hpgl_state_t *pgls, gs_point *pcl_pt)
{
	gs_point dev_pt, hpgl_pt;
	hpgl_call(hpgl_clear_current_path(pgls));
	pcl_set_ctm(pgls, false);
	hpgl_call(gs_transform(pgls->pgs, pcl_pt->x, pcl_pt->y, &dev_pt));
	hpgl_call(hpgl_set_ctm(pgls));
	hpgl_call(gs_itransform(pgls->pgs, dev_pt.x, dev_pt.y, &hpgl_pt));
	hpgl_call(hpgl_add_point_to_path(pgls, hpgl_pt.x, hpgl_pt.y, gs_moveto));
	return 0;
}

 int
hpgl_add_arc_to_path(hpgl_state_t *pgls, floatp center_x, floatp center_y, 
		     floatp radius, floatp start_angle, floatp sweep_angle,
		     floatp chord_angle, bool start_moveto)
{
	floatp start_angle_radians = start_angle * degrees_to_radians;
	/*
	 * Ensure that the sweep angle is an integral multiple of the
	 * chord angle, by decreasing the chord angle if necessary.
	 */
	int num_chords = ceil(sweep_angle / chord_angle);
	floatp chord_angle_radians =
	  sweep_angle / num_chords * degrees_to_radians;
	int i;
	floatp arccoord_x, arccoord_y;

	hpgl_compute_arc_coords(radius, center_x, center_y, 
				start_angle_radians, 
				&arccoord_x, &arccoord_y);
	hpgl_call(hpgl_add_point_to_path(pgls, arccoord_x, arccoord_y, 
					 ((pgls->g.pen_down) && 
					 (!start_moveto)) ? 
					 gs_lineto : gs_moveto));

	/* HAS - pen up/down is invariant in the loop */
	for ( i = 0; i < num_chords; i++ ) 
	  {
	    start_angle_radians += chord_angle_radians;
	    hpgl_compute_arc_coords(radius, center_x, center_y, 
				    start_angle_radians, 
				    &arccoord_x, &arccoord_y);
	    hpgl_call(hpgl_add_point_to_path(pgls, arccoord_x, arccoord_y, 
					     ((pgls->g.pen_down) ? 
					      gs_lineto : gs_moveto)));
	  }
	return 0;
}

 int
hpgl_add_bezier_to_path(hpgl_state_t *pgls, floatp x1, floatp y1, 
			floatp x2, floatp y2, floatp x3, floatp y3, 
			floatp x4, floatp y4)
{
	hpgl_call(hpgl_add_point_to_path(pgls, x1, y1, gs_moveto));
	/* HAS we may need to flatten this here */
	hpgl_call(gs_curveto(pgls->pgs, x2, y2, x3, y3, x4, y4));
	/* set the state position */
	{
	  gs_point last_point;
	  last_point.x = x4;
	  last_point.y = y4;
	  hpgl_call(hpgl_set_current_position(pgls, (gs_point *)&last_point));
	}
	return 0;
}

/* an implicit gl/2 style closepath.  If the first and last point are
   the same the path gets closed */
 private int
hpgl_close_path(hpgl_state_t *pgls)
{
	gs_point last_point;
	hpgl_call(hpgl_get_current_position(pgls, &last_point));

	if ( (pgls->g.first_point.x == last_point.x) &&
	     (pgls->g.first_point.y == last_point.y) )
	    hpgl_call(gs_closepath(pgls->pgs));

	return 0;
}
	
	  

/* HAS -- There will need to be compression phase here note that
   extraneous PU's do not result in separate subpaths.  */

/* Draw (stroke or fill) the current path. */
 int
hpgl_draw_current_path(hpgl_state_t *pgls, hpgl_rendering_mode_t render_mode)
{
	if ( !pgls->g.have_first_moveto ) return 0;

	hpgl_call(hpgl_close_path(pgls));

	hpgl_call(hpgl_set_graphics_state(pgls, render_mode));

	/* we reset the ctm before stroking to preserve the line width
           information */
	hpgl_call(hpgl_set_plu_to_device_ctm(pgls));
	
	switch ( render_mode )
	  {
	  case hpgl_rm_polygon:
	    if ( pgls->g.fill_type == hpgl_even_odd_rule )
	      hpgl_call(gs_eofill(pgls->pgs));
	    else /* hpgl_winding_number_rule */
	      hpgl_call(gs_fill(pgls->pgs));
	    break;
	  case hpgl_rm_clip_and_fill_polygon:
	    /* handled by hpgl_set_graphics_state */
	    break;
	  case hpgl_rm_vector:
	  case hpgl_rm_character:
	  case hpgl_rm_vector_fill:
	    hpgl_call(gs_stroke(pgls->pgs));
	    break;
	  default :
	    dprintf("unknown render mode\n");
	  }

	pgls->g.have_first_moveto = false;
	/* the page has been marked */
	pgls->have_page = true;
	return 0;
}

/* HAS needs error checking for empty paths and such */
 int 
hpgl_copy_current_path_to_polygon_buffer(hpgl_state_t *pgls)
{
	gx_path *ppath = gx_current_path(pgls->pgs);

	gx_path_copy(ppath, &pgls->g.polygon_buffer.path, true );
	pgls->g.polygon_buffer.have_first_moveto = pgls->g.have_first_moveto;
	return 0;
}

 int
hpgl_gsave(hpgl_state_t *pgls)
{
	hpgl_call(gs_gsave(pgls->pgs));
	pgls->g.saved_have_first_moveto = pgls->g.have_first_moveto;
	return 0;
}

 int
hpgl_grestore(hpgl_state_t *pgls)
{
	hpgl_call(gs_grestore(pgls->pgs));
	pgls->g.have_first_moveto = pgls->g.saved_have_first_moveto;
	return 0;
}


 int 
hpgl_copy_polygon_buffer_to_current_path(hpgl_state_t *pgls)
{
	gx_path *ppath = gx_current_path(pgls->pgs);
	gx_path_copy(&pgls->g.polygon_buffer.path, ppath, true );
	pgls->g.have_first_moveto = pgls->g.polygon_buffer.have_first_moveto;
	return 0;
}

 int
hpgl_draw_line(hpgl_state_t *pgls, floatp x1, floatp y1, floatp x2, floatp y2)
{
	hpgl_call(hpgl_add_point_to_path(pgls, x1, y1, 
					 ((pgls->g.pen_down) ? 
					  gs_lineto : gs_moveto)));
	hpgl_call(hpgl_add_point_to_path(pgls, x2, y2, 
					 ((pgls->g.pen_down) ? 
					  gs_lineto : gs_moveto)));
	hpgl_call(hpgl_draw_current_path(pgls, hpgl_rm_vector));
	return 0;
}

 int
hpgl_draw_dot(hpgl_state_t *pgls, floatp x1, floatp y1)
{
	hpgl_call(hpgl_add_point_to_path(pgls, x1, y1, 
					 ((pgls->g.pen_down) ? 
					  gs_lineto : gs_moveto)));
	hpgl_call(hpgl_draw_current_path(pgls, hpgl_rm_vector));

	return 0;
}
