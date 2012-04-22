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

static double ic[3][3] = { { 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 } };

double FrameTime = 0;

/************************************************************************/
/* Transforms relative coordinates in input file to absolute coordinates*/
/* on the drawable pixmap.						*/
/************************************************************************/
gint transf_abs(double x, double xmin, double xmax, gint absxsize) {
	double newx;

	newx = (x - xmin) / (xmax - xmin);
	return (gint) (newx * absxsize);
}

/************************************************************************/
/* This function does the actual drawing of the circles accordingly to	*/
/* mode.								*/
/************************************************************************/
void drawcircles(cairo_t *cr, double xmin, double xmax, double ymin,
		double ymax, double zmin, double zmax, gint absxsize, gint absysize,
		gint mode, gint radius, gint vary, struct xyzstruc *coords,
		gboolean usetypes, gint numtypes, gint numatoms,
		struct GlobalParams *params) {
	gint x, y, c, i, rtmp;
	cairo_pattern_t *pat;

	radius /= 2;
	for (i = 0; i < numatoms; i++) {
		x = transf_abs(coords[i].xcoord, xmin, xmax, absxsize);
		y = transf_abs(coords[i].ycoord, ymin, ymax, absysize);
		if (coords[i].zcoord >= zmin && coords[i].zcoord <= zmax) {

			if (usetypes)
				c = transf_abs(coords[i].atype, 0, numtypes + 1, NUMCOLORS);
			else
				c = transf_abs(coords[i].zcoord, zmin, zmax, NUMCOLORS);

			if (vary == 1) {
				rtmp = (int) (radius
						* (0.5 * (coords[i].zcoord - zmin) / (zmax - zmin))
						+ 0.5 * radius);
			} else if (vary == 2) {
				rtmp = (int) (radius
						* (0.5 * (-coords[i].zcoord + zmax) / (zmax - zmin))
						+ 0.5 * radius);
			} else
				rtmp = radius;

			if (x > 0 && y > 0 && x < (absxsize - radius / 2)
					&& y < (absysize - radius / 2)) {
				if (mode == 0) {
					cairo_rectangle(cr, x - rtmp / 2 + xborder,
							(absysize - y) - rtmp / 2 + yborder, rtmp, rtmp);
					cairo_set_source_rgb(cr, params->xcolorset[c][0],
							params->xcolorset[c][1], params->xcolorset[c][2]);
					cairo_fill(cr);
				} else if (mode == 1) {
					cairo_arc(cr, x + xborder, (absysize - y) + yborder, rtmp,
							0, 2 * M_PI);
					cairo_set_source_rgb(cr, params->xcolorset[c][0],
							params->xcolorset[c][1], params->xcolorset[c][2]);
					cairo_fill(cr);
				} else if (mode == 2) {
					pat = cairo_pattern_create_radial(x + xborder - rtmp / 6.0,
							(absysize - y) + yborder - rtmp / 3.0, rtmp / 10.0, //115.2, 102.4, 25.6,
							x + xborder - rtmp / 3.0,
							(absysize - y) + yborder - rtmp / 3.0, rtmp * 1.67); //102.4,  102.4, 128.0);
					cairo_pattern_add_color_stop_rgba(pat, 0, 1, 1, 1, 1);
					cairo_pattern_add_color_stop_rgba(pat, 0.2,
							params->xcolorset[c][0], params->xcolorset[c][1],
							params->xcolorset[c][2], 1);
					cairo_pattern_add_color_stop_rgba(pat, 1,
							0.2 * params->xcolorset[c][0],
							0.2 * params->xcolorset[c][1],
							0.2 * params->xcolorset[c][2], 1);
					cairo_set_source(cr, pat);
					cairo_arc(cr, x + xborder, (absysize - y) + yborder, rtmp,
							0, 2 * M_PI);
					cairo_fill(cr);
					cairo_pattern_destroy(pat);
				}
			}
		}
	}
}

/************************************************************************/
/* This function rotates the coordinates of the atoms, sorts them and	*/
/* calls the drawcircles to draw them.					*/
/************************************************************************/
void rotateatoms(struct DrawStruct DrawData) {
	gint i, j, numatoms;

	double isin, icos, jsin, jcos, ksin, kcos;
	double maxx, minx, maxy, miny, maxz, minz;
	double imsin, imcos, jmsin, jmcos;
	double ictmp[3];
	double newic[3][3];

	struct xyzstruc *newcoords;
	struct xyzstruc *coords;
	struct GlobalParams *params;

	params = DrawData.params;

	coords = (DrawData.currentFrame)->atomdata;
	numatoms = (DrawData.currentFrame)->numAtoms;

	minx = 0.0;
	maxx = 0.0;
	miny = 0.0;
	maxy = 0.0;
	minz = 0.0;
	maxz = 0.0;
	isin = sin(params->iangle * (-PI / 180.0));
	icos = cos(params->iangle * (-PI / 180.0));
	jsin = sin(params->jangle * (PI / 180.0));
	jcos = cos(params->jangle * (PI / 180.0));
	ksin = sin(params->kangle * (-PI / 180.0));
	kcos = cos(params->kangle * (-PI / 180.0));
	imsin = sin(params->jmangle * (-PI / 180.0));
	imcos = cos(params->jmangle * (-PI / 180.0));
	jmsin = sin(params->imangle * (-PI / 180.0));
	jmcos = cos(params->imangle * (-PI / 180.0));

	if (params->erase) {
		cleardrawable(DrawData);
	}

	newcoords = (struct xyzstruc *) g_malloc(
			numatoms * sizeof(struct xyzstruc));

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

	params->iangle = 0.0;
	params->jangle = 0.0;
	params->kangle = 0.0;
	params->imangle = 0.0;
	params->jmangle = 0.0;

	if (ic[0][0] != 0.0)
		params->zc = atan(ic[0][1] / ic[0][0]) * (180.0 / PI);
	else
		params->zc = 0.0;
	if (ic[0][0] < 0.0 && ic[0][1] > 0.0)
		params->zc += 180;
	else if (ic[0][0] < 0.0 && ic[0][1] < 0.0)
		params->zc += 180;
	else if (ic[0][0] > 0.0 && ic[0][1] < 0.0)
		params->zc += 360;

	ictmp[0] = ic[2][0] * cos(-params->zc * (PI / 180.0))
			- ic[2][1] * sin(-params->zc * (PI / 180.0));

	if (ic[2][2] != 0.0)
		params->yc = atan(-ictmp[0] / ic[2][2]) * (180.0 / PI);
	else
		params->yc = 0.0;
	if (ic[2][2] < 0.0 && ictmp[0] > 0.0)
		params->yc += 180;
	else if (ic[2][2] < 0.0 && ictmp[0] < 0.0)
		params->yc += 180;
	else if (ic[2][2] > 0.0 && ictmp[0] < 0.0)
		params->yc += 360;

	ictmp[0] = ic[1][0] * cos(-params->zc * (PI / 180.0))
			- ic[1][1] * sin(-params->zc * (PI / 180.0));
	ictmp[1] = ic[1][0] * sin(-params->zc * (PI / 180.0))
			+ ic[1][1] * cos(-params->zc * (PI / 180.0));
	ictmp[2] = ictmp[0] * sin(-params->yc * (PI / 180.0))
			+ ic[1][2] * cos(-params->yc * (PI / 180.0));

	if (ictmp[1] != 0.0)
		params->xc = atan(ictmp[2] / ictmp[1]) * (180.0 / PI);
	else
		params->xc = 0.0;
	if (ictmp[1] < 0.0 && ictmp[2] > 0.0)
		params->xc += 180;
	else if (ictmp[1] < 0.0 && ictmp[2] < 0.0)
		params->xc += 180;
	else if (ictmp[1] > 0.0 && ictmp[2] < 0.0)
		params->xc += 360;

	if (params->xc <= 0.0)
		params->xc += 360.0;
	if (params->xc >= 360.0)
		params->xc -= 360.0;
	if (params->yc <= 0.0)
		params->yc += 360.0;
	if (params->yc >= 360.0)
		params->yc -= 360.0;
	if (params->zc <= 0.0)
		params->zc += 360.0;
	if (params->zc >= 360.0)
		params->zc -= 360.0;

	if (params->sort == 2) {
		sortatoms(newcoords, 0, numatoms - 1, FALSE);
	} else
		sortatoms(newcoords, 0, numatoms - 1, TRUE);

	for (i = 0; i < numatoms; i++) {
		newcoords[i].zcoord = coords[newcoords[i].index].zcoord;
	}

	if (params->xmin == 65535.0) {
		(DrawData.currentFrame)->xmax = maxx;
		(DrawData.currentFrame)->xmin = minx;
	} else {
		(DrawData.currentFrame)->xmax = params->xmax;
		(DrawData.currentFrame)->xmin = params->xmin;
	}
	if (params->ymin == 65535.0) {
		(DrawData.currentFrame)->ymax = maxy;
		(DrawData.currentFrame)->ymin = miny;
	} else {
		(DrawData.currentFrame)->ymax = params->ymax;
		(DrawData.currentFrame)->ymin = params->ymin;
	}
	if (params->zmin == 65535.0) {
		(DrawData.currentFrame)->zmax = maxz;
		(DrawData.currentFrame)->zmin = minz;
	} else {
		(DrawData.currentFrame)->zmax = params->zmax;
		(DrawData.currentFrame)->zmin = params->zmin;
	}

	drawcircles(DrawData.cr, (DrawData.currentFrame)->xmin, (DrawData.currentFrame)->xmax,
			(DrawData.currentFrame)->ymin, (DrawData.currentFrame)->ymax,
			(DrawData.currentFrame)->zmin, (DrawData.currentFrame)->zmax, params->absxsize,
			params->absysize, params->mode, params->radius, params->vary,
			newcoords, params->usetypes, params->numtypes, numatoms, params);

	FrameTime = (DrawData.currentFrame)->atime;
	g_free(newcoords);
}

/************************************************************************/
/* Clears the drawable area and draws the rectangle which represents	*/
/* border of the simulationbox.						*/
/************************************************************************/
void cleardrawable(struct DrawStruct DrawData) {

	cairo_rectangle(DrawData.cr, 0.0, 0.0, DrawData.crXSize, DrawData.crYSize);
	if (DrawData.params->whitebg) {
		cairo_set_source_rgb(DrawData.cr, 1, 1, 1);
		cairo_fill(DrawData.cr);
		cairo_rectangle(DrawData.cr, xborder, yborder,
				DrawData.params->absxsize, DrawData.params->absysize);
		cairo_set_source_rgb(DrawData.cr, 0, 0, 0);
		cairo_stroke(DrawData.cr);
	} else {
		cairo_set_source_rgb(DrawData.cr, 0, 0, 0);
		cairo_fill(DrawData.cr);
		cairo_rectangle(DrawData.cr, xborder, yborder,
				DrawData.params->absxsize, DrawData.params->absysize);
		cairo_set_source_rgb(DrawData.cr, 1, 1, 1);
		cairo_stroke(DrawData.cr);
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
