/*

 gdpc2 - a program for visualising molecular dynamic simulations
 Copyright (C) 2012 Jonas Frantz

 This file is a part of gdpc2.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 Authors email: jonas@frantz.fi

 */

#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include "parameters.h"

static double rotationVector[3][3] = { X_VECTOR, Y_VECTOR, Z_VECTOR };

/************************************************************************
 * The following procedure is the callback for angular change button
 * presses.
 ************************************************************************/
void angleAdjustmentButtonPressed(GtkWidget *widget, struct AngleAdjustment *angleAdjustment) {
	gint getval;

	if (g_mutex_trylock(angleAdjustment->context->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(angleAdjustment->context->atEnd);
	} else
		getval = 0;

	angleAdjustment->context->iangle = angleAdjustment->idelta;
	angleAdjustment->context->jangle = angleAdjustment->jdelta;
	angleAdjustment->context->kangle = angleAdjustment->kdelta;
	if (angleAdjustment->context->pausecheck || getval == 0)
		triggerImageRedraw(widget, angleAdjustment->context);
}


/************************************************************************
 * This procedure is called when the Reset orientation button is pressed.
 * It resets the angles and prints them in their entries.
 ************************************************************************/
void resetOrientationButtonPressed(GtkWidget *widget, struct Context *context) {
	resetOrientation();
	triggerImageRedraw(widget, context);
}

/************************************************************************/
/* This procedure is called after the mouse has been "dragged" on the	*/
/* drawingboard. It calculates the rotational angles depending on the	*/
/* dragged distance and direction and calls the rotating procedure.	*/
/************************************************************************/
void mouseRotate(GtkWidget *widget, gint xdelta, gint ydelta,
		struct Context *context) {
	gint getval;

	if (g_mutex_trylock(context->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(context->atEnd);
	} else
		getval = 0;

	context->imangle = (xdelta * 90.0) / (double) context->config->absxsize;
	context->jmangle = (ydelta * 90.0) / (double) context->config->absysize;
	if (context->pausecheck || context->config->mbsleep || getval == 0)
		triggerImageRedraw(widget, context);
}

/************************************************************************/
/* This function rotates the coordinates of the atoms, sorts them and	*/
/* calls the drawcircles to draw them.									*/
/************************************************************************/
struct Atom * rotateAtoms(struct Context *context) {
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

	newcoords = (struct Atom *) g_malloc(numatoms * sizeof(struct Atom));

	for (i = 0; i < 3; i++)
		newic[0][i] = rotationVector[0][i] * jcos * kcos + rotationVector[1][i] * (-jcos * ksin)
				+ rotationVector[2][i] * jsin;
	for (i = 0; i < 3; i++)
		newic[1][i] = rotationVector[0][i] * (isin * jsin * kcos + icos * ksin)
				+ rotationVector[1][i] * (-isin * jsin * ksin + icos * kcos)
				+ rotationVector[2][i] * (-isin * jcos);
	for (i = 0; i < 3; i++)
		newic[2][i] = rotationVector[0][i] * (-icos * jsin * kcos + isin * ksin)
				+ rotationVector[1][i] * (icos * jsin * ksin + isin * kcos)
				+ rotationVector[2][i] * icos * jcos;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			rotationVector[i][j] = newic[i][j];

	for (i = 0; i < 3; i++)
		newic[0][i] = rotationVector[0][i] * jmcos + rotationVector[2][i] * jmsin;
	for (i = 0; i < 3; i++)
		newic[1][i] = rotationVector[0][i] * imsin * jmsin + rotationVector[1][i] * imcos
				+ rotationVector[2][i] * (-imsin * jmcos);
	for (i = 0; i < 3; i++)
		newic[2][i] = rotationVector[0][i] * (-imcos * jmsin) + rotationVector[1][i] * imsin
				+ rotationVector[2][i] * imcos * jmcos;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			rotationVector[i][j] = newic[i][j];

	for (i = 0; i < numatoms; i++) {
		newcoords[i].xcoord = rotationVector[0][0] * coords[i].xcoord
				+ rotationVector[0][1] * coords[i].ycoord + rotationVector[0][2] * coords[i].zcoord;
		newcoords[i].ycoord = rotationVector[1][0] * coords[i].xcoord
				+ rotationVector[1][1] * coords[i].ycoord + rotationVector[1][2] * coords[i].zcoord;
		newcoords[i].zcoord = rotationVector[2][0] * coords[i].xcoord
				+ rotationVector[2][1] * coords[i].ycoord + rotationVector[2][2] * coords[i].zcoord;
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

	if (rotationVector[0][0] != 0.0)
		config->zc = atan(rotationVector[0][1] / rotationVector[0][0]) * (180.0 / PI);
	else
		config->zc = 0.0;
	if (rotationVector[0][0] < 0.0 && rotationVector[0][1] > 0.0)
		config->zc += 180;
	else if (rotationVector[0][0] < 0.0 && rotationVector[0][1] < 0.0)
		config->zc += 180;
	else if (rotationVector[0][0] > 0.0 && rotationVector[0][1] < 0.0)
		config->zc += 360;

	ictmp[0] = rotationVector[2][0] * cos(-config->zc * (PI / 180.0))
			- rotationVector[2][1] * sin(-config->zc * (PI / 180.0));

	if (rotationVector[2][2] != 0.0)
		config->yc = atan(-ictmp[0] / rotationVector[2][2]) * (180.0 / PI);
	else
		config->yc = 0.0;
	if (rotationVector[2][2] < 0.0 && ictmp[0] > 0.0)
		config->yc += 180;
	else if (rotationVector[2][2] < 0.0 && ictmp[0] < 0.0)
		config->yc += 180;
	else if (rotationVector[2][2] > 0.0 && ictmp[0] < 0.0)
		config->yc += 360;

	ictmp[0] = rotationVector[1][0] * cos(-config->zc * (PI / 180.0))
			- rotationVector[1][1] * sin(-config->zc * (PI / 180.0));
	ictmp[1] = rotationVector[1][0] * sin(-config->zc * (PI / 180.0))
			+ rotationVector[1][1] * cos(-config->zc * (PI / 180.0));
	ictmp[2] = ictmp[0] * sin(-config->yc * (PI / 180.0))
			+ rotationVector[1][2] * cos(-config->yc * (PI / 180.0));

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

	return newcoords;
}


/************************************************************************/
/* This function resets the orientation of the system by reseting the 	*/
/* vector that handles the rotation.									*/
/************************************************************************/
void resetOrientation() {
	const static double x_vector[3] = X_VECTOR;
	const static double y_vector[3] = Y_VECTOR;
	const static double z_vector[3] = Z_VECTOR;

	rotationVector[0][0] = x_vector[0];
	rotationVector[0][1] = x_vector[1];
	rotationVector[0][2] = x_vector[2];
	rotationVector[1][0] = y_vector[0];
	rotationVector[1][1] = y_vector[1];
	rotationVector[1][2] = y_vector[2];
	rotationVector[2][0] = z_vector[0];
	rotationVector[2][1] = z_vector[1];
	rotationVector[2][2] = z_vector[2];
}
