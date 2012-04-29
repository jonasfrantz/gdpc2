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

static double ic[3][3] = {{ 1.0, 0.0, 0.0 },
						  { 0.0, 1.0, 0.0 },
						  { 0.0, 0.0, 1.0 }};

/************************************************************************/
/* Transforms relative coordinates in input file to absolute coordinates*/
/* on the drawable pixmap.												*/
/************************************************************************/
gint transf_abs(double x, double xmin, double xmax, gint absxsize) {
	double newx;

	newx = (x - xmin) / (xmax - xmin);
	return (gint) (newx * absxsize);
}

/************************************************************************/
/* This function does the actual drawing of the circles accordingly to	*/
/* mode.																*/
/************************************************************************/
void drawcircles(cairo_t *cr, struct Frame *frame, struct Atom *coords,
		gint numatoms, struct Configuration *config) {
	gint x, y, c, i, rtmp;
	gint radius;
	cairo_pattern_t *pat;

	radius = config->radius / 2;
	for (i = 0; i < numatoms; i++) {
		x = transf_abs(coords[i].xcoord, frame->xmin, frame->xmax,
				config->absxsize);
		y = transf_abs(coords[i].ycoord, frame->ymin, frame->ymax,
				config->absysize);
		if (coords[i].zcoord >= frame->zmin
				&& coords[i].zcoord <= frame->zmax) {

			if (config->usetypes)
				c = transf_abs(coords[i].atype, 0, config->numtypes + 1,
						NUMCOLORS);
			else
				c = transf_abs(coords[i].zcoord, frame->zmin, frame->zmax,
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
/* This function rotates the coordinates of the atoms, sorts them and	*/
/* calls the drawcircles to draw them.									*/
/************************************************************************/
void rotateAtoms(struct Context *context) {
	gint i, j, numatoms;

	double isin, icos, jsin, jcos, ksin, kcos;
	double maxx, minx, maxy, miny, maxz, minz;
	double imsin, imcos, jmsin, jmcos;
	double ictmp[3];
	double newic[3][3];

	struct Atom *newcoords;
	struct Atom *coords;
	struct Configuration *config;

	config = context->config;

	coords = (context->currentFrame)->atomdata;
	numatoms = (context->currentFrame)->numAtoms;

	minx = 0.0;
	maxx = 0.0;
	miny = 0.0;
	maxy = 0.0;
	minz = 0.0;
	maxz = 0.0;
	isin = sin(context->iangle * (-PI / 180.0));
	icos = cos(context->iangle * (-PI / 180.0));
	jsin = sin(context->jangle * (PI / 180.0));
	jcos = cos(context->jangle * (PI / 180.0));
	ksin = sin(context->kangle * (-PI / 180.0));
	kcos = cos(context->kangle * (-PI / 180.0));
	imsin = sin(context->jmangle * (-PI / 180.0));
	imcos = cos(context->jmangle * (-PI / 180.0));
	jmsin = sin(context->imangle * (-PI / 180.0));
	jmcos = cos(context->imangle * (-PI / 180.0));

	if (config->erase) {
		clearDrawable(context);
	}

	newcoords = (struct Atom *) g_malloc(numatoms * sizeof(struct Atom));

	for (i = 0; i < 3; i++)
		newic[0][i] = ic[0][i] * jcos * kcos + ic[1][i] * (-jcos * ksin)
				+ ic[2][i] * jsin;
	for (i = 0; i < 3; i++)
		newic[1][i] = ic[0][i] * (isin * jsin * kcos + icos * ksin)
				+ ic[1][i] * (-isin * jsin * ksin + icos * kcos)
				+ ic[2][i] * (-isin * jcos);
	for (i = 0; i < 3; i++)
		newic[2][i] = ic[0][i] * (-icos * jsin * kcos + isin * ksin)
				+ ic[1][i] * (icos * jsin * ksin + isin * kcos)
				+ ic[2][i] * icos * jcos;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			ic[i][j] = newic[i][j];

	for (i = 0; i < 3; i++)
		newic[0][i] = ic[0][i] * jmcos + ic[2][i] * jmsin;
	for (i = 0; i < 3; i++)
		newic[1][i] = ic[0][i] * imsin * jmsin + ic[1][i] * imcos
				+ ic[2][i] * (-imsin * jmcos);
	for (i = 0; i < 3; i++)
		newic[2][i] = ic[0][i] * (-imcos * jmsin) + ic[1][i] * imsin
				+ ic[2][i] * imcos * jmcos;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			ic[i][j] = newic[i][j];

	for (i = 0; i < numatoms; i++) {
		newcoords[i].xcoord = ic[0][0] * coords[i].xcoord
				+ ic[0][1] * coords[i].ycoord + ic[0][2] * coords[i].zcoord;
		newcoords[i].ycoord = ic[1][0] * coords[i].xcoord
				+ ic[1][1] * coords[i].ycoord + ic[1][2] * coords[i].zcoord;
		newcoords[i].zcoord = ic[2][0] * coords[i].xcoord
				+ ic[2][1] * coords[i].ycoord + ic[2][2] * coords[i].zcoord;
		newcoords[i].atype = coords[i].atype;
		newcoords[i].index = i;
		if (newcoords[i].xcoord > maxx)
			maxx = newcoords[i].xcoord;
		if (newcoords[i].ycoord > maxy)
			maxy = newcoords[i].ycoord;
		if (newcoords[i].zcoord > maxz)
			maxz = newcoords[i].zcoord;
		if (newcoords[i].xcoord < minx)
			minx = newcoords[i].xcoord;
		if (newcoords[i].ycoord < miny)
			miny = newcoords[i].ycoord;
		if (newcoords[i].zcoord < minz)
			minz = newcoords[i].zcoord;
	}

	context->iangle = 0.0;
	context->jangle = 0.0;
	context->kangle = 0.0;
	context->imangle = 0.0;
	context->jmangle = 0.0;

	if (ic[0][0] != 0.0)
		config->zc = atan(ic[0][1] / ic[0][0]) * (180.0 / PI);
	else
		config->zc = 0.0;
	if (ic[0][0] < 0.0 && ic[0][1] > 0.0)
		config->zc += 180;
	else if (ic[0][0] < 0.0 && ic[0][1] < 0.0)
		config->zc += 180;
	else if (ic[0][0] > 0.0 && ic[0][1] < 0.0)
		config->zc += 360;

	ictmp[0] = ic[2][0] * cos(-config->zc * (PI / 180.0))
			- ic[2][1] * sin(-config->zc * (PI / 180.0));

	if (ic[2][2] != 0.0)
		config->yc = atan(-ictmp[0] / ic[2][2]) * (180.0 / PI);
	else
		config->yc = 0.0;
	if (ic[2][2] < 0.0 && ictmp[0] > 0.0)
		config->yc += 180;
	else if (ic[2][2] < 0.0 && ictmp[0] < 0.0)
		config->yc += 180;
	else if (ic[2][2] > 0.0 && ictmp[0] < 0.0)
		config->yc += 360;

	ictmp[0] = ic[1][0] * cos(-config->zc * (PI / 180.0))
			- ic[1][1] * sin(-config->zc * (PI / 180.0));
	ictmp[1] = ic[1][0] * sin(-config->zc * (PI / 180.0))
			+ ic[1][1] * cos(-config->zc * (PI / 180.0));
	ictmp[2] = ictmp[0] * sin(-config->yc * (PI / 180.0))
			+ ic[1][2] * cos(-config->yc * (PI / 180.0));

	if (ictmp[1] != 0.0)
		config->xc = atan(ictmp[2] / ictmp[1]) * (180.0 / PI);
	else
		config->xc = 0.0;
	if (ictmp[1] < 0.0 && ictmp[2] > 0.0)
		config->xc += 180;
	else if (ictmp[1] < 0.0 && ictmp[2] < 0.0)
		config->xc += 180;
	else if (ictmp[1] > 0.0 && ictmp[2] < 0.0)
		config->xc += 360;

	if (config->xc <= 0.0)
		config->xc += 360.0;
	if (config->xc >= 360.0)
		config->xc -= 360.0;
	if (config->yc <= 0.0)
		config->yc += 360.0;
	if (config->yc >= 360.0)
		config->yc -= 360.0;
	if (config->zc <= 0.0)
		config->zc += 360.0;
	if (config->zc >= 360.0)
		config->zc -= 360.0;

	if (config->sort == 2) {
		sortatoms(newcoords, 0, numatoms - 1, FALSE);
	} else
		sortatoms(newcoords, 0, numatoms - 1, TRUE);

	for (i = 0; i < numatoms; i++) {
		newcoords[i].zcoord = coords[newcoords[i].index].zcoord;
	}

	if (config->xmin == 65535.0) {
		(context->currentFrame)->xmax = maxx;
		(context->currentFrame)->xmin = minx;
	} else {
		(context->currentFrame)->xmax = config->xmax;
		(context->currentFrame)->xmin = config->xmin;
	}
	if (config->ymin == 65535.0) {
		(context->currentFrame)->ymax = maxy;
		(context->currentFrame)->ymin = miny;
	} else {
		(context->currentFrame)->ymax = config->ymax;
		(context->currentFrame)->ymin = config->ymin;
	}
	if (config->zmin == 65535.0) {
		(context->currentFrame)->zmax = maxz;
		(context->currentFrame)->zmin = minz;
	} else {
		(context->currentFrame)->zmax = config->zmax;
		(context->currentFrame)->zmin = config->zmin;
	}

	drawcircles(context->cr, context->currentFrame, newcoords, numatoms,
			context->config);

	g_free(newcoords);
}

/************************************************************************/
/* Clears the drawable area and draws the rectangle which represents	*/
/* border of the simulationbox.						*/
/************************************************************************/
void clearDrawable(struct Context *context) {

	cairo_rectangle(context->cr, 0.0, 0.0, context->crXSize, context->crYSize);
	if (context->config->whitebg) {
		cairo_set_source_rgb(context->cr, 1, 1, 1);
		cairo_fill(context->cr);
		cairo_rectangle(context->cr, xborder, yborder,
				context->config->absxsize, context->config->absysize);
		cairo_set_source_rgb(context->cr, 0, 0, 0);
		cairo_stroke(context->cr);
	} else {
		cairo_set_source_rgb(context->cr, 0, 0, 0);
		cairo_fill(context->cr);
		cairo_rectangle(context->cr, xborder, yborder,
				context->config->absxsize, context->config->absysize);
		cairo_set_source_rgb(context->cr, 1, 1, 1);
		cairo_stroke(context->cr);
	}

}

/************************************************************************/
/* This function resets the orientation of the system by reseting the 	*/
/* vector that handles the rotation.					*/
/************************************************************************/
void resetic() {
	ic[0][0] = 1.0;
	ic[0][1] = 0.0;
	ic[0][2] = 0.0;
	ic[1][0] = 0.0;
	ic[1][1] = 1.0;
	ic[1][2] = 0.0;
	ic[2][0] = 0.0;
	ic[2][1] = 0.0;
	ic[2][2] = 1.0;
}
