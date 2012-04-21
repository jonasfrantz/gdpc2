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
#include <math.h>
#include <stdio.h>
#include "parameters.h"

/************************************************************************/
/* The following procedures take care of the one step angle change.	*/
/************************************************************************/
void xplusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->iangle = 1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void yplusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->jangle = 1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void zplusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->kangle = 1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void xminusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->iangle = -1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void yminusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->jangle = -1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void zminusb(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->kangle = -1.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

/************************************************************************/
/* The following procedures take care of the 10 step angle change.	*/
/************************************************************************/
void xplus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->iangle = 10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void yplus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->jangle = 10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void zplus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->kangle = 10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void xminus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->iangle = -10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void yminus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->jangle = -10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

void zminus10b(GtkWidget *widget, struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->kangle = -10.0;
	if (params->pausecheck || getval == 0)
		drawrotate(widget, params);
}

/************************************************************************/
/* This procedure is called when the Reset orientation button is 	*/
/* pressed. It resets the angles and prints them in their entries.	*/
/************************************************************************/
void resetob(GtkWidget *widget, struct GlobalParams *params) {
	resetic();
	drawrotate(widget, params);
}

/************************************************************************/
/* This procedure is called after the mouse has been "dragged" on the	*/
/* drawingboard. It calculates the rotational angles depending on the	*/
/* dragged distance and direction and calls the rotating procedure.	*/
/************************************************************************/
void mouserotate(GtkWidget *widget, gint xdelta, gint ydelta,
		struct GlobalParams *params) {
	gint getval;

	if (g_mutex_trylock(params->atEnd) == TRUE) {
		getval = 1;
		g_mutex_unlock(params->atEnd);
	} else
		getval = 0;

	params->imangle = (xdelta * 90.0) / (double) params->absxsize;
	params->jmangle = (ydelta * 90.0) / (double) params->absysize;
	if (params->pausecheck || params->mbsleep || getval == 0)
		drawrotate(widget, params);
}
