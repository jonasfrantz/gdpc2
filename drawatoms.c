/*

 gdpc - a program for visualising molecular dynamic simulations
 Copyright (C) 2000 Jonas Frantz

 This file is part of gdpc.

 gdpc is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 gdpc is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


 Authors email : jonas.frantz@helsinki.fi

 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include "parameters.h"

/************************************************************************/
/* Transforms relative coordinates in input file to absolute coordinates*/
/* on the drawable pixmap.												*/
/************************************************************************/
gint transformAbsoluteToRelative(double x, double xmin, double xmax, gint absxsize) {
	double newx;

	newx = (x - xmin) / (xmax - xmin);
	return (gint) (newx * absxsize);
}

/************************************************************************/
/* This function does the actual drawing of the circles accordingly to	*/
/* mode.																*/
/************************************************************************/
void drawAtoms(cairo_t *cr, struct Frame *frame, struct Atom *coords,
		gint numatoms, struct Configuration *config) {
	gint x, y, c, i, rtmp;
	gint radius;
	cairo_pattern_t *pat;

	radius = config->radius / 2;
	for (i = 0; i < numatoms; i++) {
		x = transformAbsoluteToRelative(coords[i].xcoord, frame->xmin, frame->xmax,
				config->absxsize);
		y = transformAbsoluteToRelative(coords[i].ycoord, frame->ymin, frame->ymax,
				config->absysize);
		if (coords[i].zcoord >= frame->zmin
				&& coords[i].zcoord <= frame->zmax) {

			if (config->usetypes)
				c = transformAbsoluteToRelative(coords[i].atype, 0, config->numtypes + 1,
						NUMCOLORS);
			else
				c = transformAbsoluteToRelative(coords[i].zcoord, frame->zmin, frame->zmax,
						NUMCOLORS);

			if (config->vary == 1) {
				rtmp = (int) (radius
						* (0.5 * (coords[i].zcoord - frame->zmin)
								/ (frame->zmax - frame->zmin)) + 0.5 * radius);
			} else if (config->vary == 2) {
				rtmp = (int) (radius
						* (0.5 * (-coords[i].zcoord + frame->zmax)
								/ (frame->zmax - frame->zmin)) + 0.5 * radius);
			} else
				rtmp = radius;

			if (x > 0 && y > 0 && x < (config->absxsize - radius / 2)
					&& y < (config->absysize - radius / 2)) {
				if (config->mode == 0) {
					cairo_rectangle(cr, x - rtmp / 2 + xborder,
							(config->absysize - y) - rtmp / 2 + yborder, rtmp,
							rtmp);
					cairo_set_source_rgb(cr, config->xcolorset[c][0],
							config->xcolorset[c][1], config->xcolorset[c][2]);
					cairo_fill(cr);
				} else if (config->mode == 1) {
					cairo_arc(cr, x + xborder, (config->absysize - y) + yborder,
							rtmp, 0, 2 * M_PI);
					cairo_set_source_rgb(cr, config->xcolorset[c][0],
							config->xcolorset[c][1], config->xcolorset[c][2]);
					cairo_fill(cr);
				} else if (config->mode == 2) {
					pat = cairo_pattern_create_radial(x + xborder - rtmp / 6.0,
							(config->absysize - y) + yborder - rtmp / 3.0,
							rtmp / 10.0, //115.2, 102.4, 25.6,
							x + xborder - rtmp / 3.0,
							(config->absysize - y) + yborder - rtmp / 3.0,
							rtmp * 1.67); //102.4,  102.4, 128.0);
					cairo_pattern_add_color_stop_rgba(pat, 0, 1, 1, 1, 1);
					cairo_pattern_add_color_stop_rgba(pat, 0.2,
							config->xcolorset[c][0], config->xcolorset[c][1],
							config->xcolorset[c][2], 1);
					cairo_pattern_add_color_stop_rgba(pat, 1,
							0.2 * config->xcolorset[c][0],
							0.2 * config->xcolorset[c][1],
							0.2 * config->xcolorset[c][2], 1);
					cairo_set_source(cr, pat);
					cairo_arc(cr, x + xborder, (config->absysize - y) + yborder,
							rtmp, 0, 2 * M_PI);
					cairo_fill(cr);
					cairo_pattern_destroy(pat);
				}
			}
		}
	}
}


/************************************************************************/
/* Clears the drawable area and draws the rectangle which represents	*/
/* border of the simulationbox.						*/
/************************************************************************/
void clearFrame(struct Context *context, cairo_t *cr) {

	cairo_rectangle(cr, 0.0, 0.0, context->crXSize, context->crYSize);
	if (context->config->whitebg) {
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_fill(cr);
		cairo_rectangle(cr, xborder, yborder,
				context->config->absxsize, context->config->absysize);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_stroke(cr);
	} else {
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_fill(cr);
		cairo_rectangle(cr, xborder, yborder,
				context->config->absxsize, context->config->absysize);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_stroke(cr);
	}

}

/************************************************************************/
/************************************************************************/
void drawFrame(struct Context *context, cairo_t *cr) {
	struct Atom *newcoords;

	if (context->config->erase) {
		clearFrame(context, cr);
	}

	newcoords = rotateAtoms(context);

	drawAtoms(cr, context->currentFrame, newcoords, context->currentFrame->numAtoms,
			context->config);

	g_free(newcoords);
}
