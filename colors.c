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

#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include "parameters.h"


/* These definitions control the change in color over
   one ball in drawingmode 2.                         */
#define comp1 56152-(sin(PI/2)*84304)
#define comp2 56152-(sin(0.875*PI/2)*84304)
#define comp3 56152-(sin(0.750*PI/2)*84304)
#define comp4 56152-(sin(0.625*PI/2)*84304)
#define comp5 56152-(sin(0.500*PI/2)*84304)
#define comp6 56152-(sin(0.375*PI/2)*84304)
#define comp7 58152-(sin(0.250*PI/2)*84304)
#define comp8 60152-(sin(0.125*PI/2)*84304)


/* xcolor contains the default colors for gdpc. */
   gint xcolor[17][3] = {
               {40000,00000,00000},
               {53000,00000,00000},
               {65535,00000,00000},
               {65535,22000,00000},
               {65535,33000,00000},
               {65535,40000,00000},
               {65535,50000,00000},
               {65000,65000,20000},
               {50000,65000,20000},
               {30000,65000,30000},
               {10000,65535,10000},
               {10000,60000,50000},
               {10000,50000,65535},
               {13000,38000,65535},
               {13000,13000,65535},
               {42000,13000,65535},
               {42000,13000,65535} };


/* xcolorinv contains the inverted colors. */
   gint xcolorinv[17][3] = {
               {26000,26000,65535},
               {13000,26000,65535},
               {13000,38000,65535},
               {10000,50000,65535},
               {10000,60000,50000},
               {10000,65535,10000},
               {30000,65000,30000},
               {50000,65000,20000},
               {65000,65000,20000},
               {65535,50000,00000},
               {65535,40000,00000},
               {65535,33000,00000},
               {65535,22000,00000},
               {65535,00000,00000},
               {56000,00000,00000},
	       {40000,00000,00000},
	       {40000,00000,00000}};


/* xcoldcolor contains the coldcolor colorset. */
   gint xcoldcolor[17][3] = {
               {255*256,224*256,255*256},
               {224*256,192*256,255*256},
               {192*256,160*256,255*256},
               {160*256,128*256,255*256},
               {128*256, 64*256,255*256},
               { 64*256,  0*256,255*256},
               {  0*256,  0*256,255*256},
               { 64*256,  0*256,224*256},
               {128*256,  0*256,192*256},
               {160*256,  0*256,160*256},
               {192*256,  0*256,128*256},
               {224*256,  0*256, 96*256},
               {255*256,  0*256,  0*256},
               {224*256,  0*256,  0*256},
               {192*256,  0*256,  0*256},
               {160*256,  0*256,  0*256},
               {157*256,  0*256,  0*256} };


/* xcoldcolor2 contains the coldcolor2 colorset. */
   gint xcoldcolor2[17][3] = {
               { 64*256,  0*256,255*256},
               {  0*256,  0*256,255*256},
               { 60*256, 20*256,255*256},
               { 90*256, 40*256,240*256},
               {120*256, 80*256,220*256},
               {140*256, 90*256,200*256},
               {160*256,100*256,180*256},
               {180*256, 90*256,160*256},
               {200*256, 80*256,128*256},
               {220*256, 40*256, 96*256},
               {236*256, 20*256, 64*256},
               {255*256,  0*256,  0*256},
               {224*256,  0*256,  0*256},
               {192*256,  0*256,  0*256},
               {160*256,  0*256,  0*256},
               {157*256,  0*256, 80*256},
               {157*256,  0*256,120*256} };

void setColorset(struct Configuration *config) {
	gint i, j;

	for (i = 0; i < 17; i++) {
		for (j = 0; j < 3; j++) {
			if (config->colorset == 1) {
				config->xcolorset[i][j] = xcolorinv[i][j] / 65535.0;
			} else if (config->colorset == 2) {
				config->xcolorset[i][j] = xcoldcolor[i][j] / 65535.0;
			} else if (config->colorset == 3) {
				config->xcolorset[i][j] = xcoldcolor2[i][j] / 65535.0;
			} else if (config->colorset == 4) {
				config->xcolorset[i][j] = xcoldcolor[i][j] / 65535.0;
			} else {
				config->xcolorset[i][j] = xcolor[i][j] / 65535.0;
			}
		}
	}
}
