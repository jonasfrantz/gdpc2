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
#include <string.h>
#include "parameters.h"

gint NumFrameRI = 0;
FILE *NewFP = NULL;
gint lastframedone = 0;

/************************************************************************/
/* Reads the input file and processes it, then it calls rotateatoms to	*/
/* rotate the coordinates and draw them.				*/
/************************************************************************/
void * readinput(struct GlobalParams *params) {
	gchar buf[160];
	gchar arg[20][64];
	gchar timestr[64] = "0.0\0";
	gchar AType[MAXTYPES][5];

	gint n, i, j, numtypes, nreadxyz, numalloc, numatoms;

	double maxx, maxy, maxz, minx, miny, minz;

	gboolean timecheck, endframe, framecheck, typescheck;

	struct xyzstruc *coords;
	struct xyzstruc lastframe;

	FILE *fpRI;

#if Debug
	printf("Starting reading thread.\n");
#endif

	framecheck = FALSE;

	while (1) {
		g_mutex_lock(params->atEnd);
		g_mutex_lock(params->framedata[NumFrameRI].framedrawn);

#if Debug
		printf("Reading frame : %d\n",NumFrameRI);
#endif

		lastframedone = 0;
		numtypes = 0;
		minx = 0.0;
		miny = 0.0;
		minz = 0.0;
		maxx = 0.0;
		maxy = 0.0;
		maxz = 0.0;

		if (NewFP != NULL) {
			g_mutex_lock(params->filewait);
			params->fp = NewFP;
			NewFP = NULL;
			fclose(fpRI);
		}
		fpRI = params->fp;

		/* If file is in xyz format start reading here ! */

		if (params->fxyz) {
			if (fgets(buf, 160, fpRI) == NULL) {
				NumFrameRI++;
				if (NumFrameRI == NUMFRAMES)
					NumFrameRI = 0;
				lastframedone = 1;
				continue;
			} else
				g_mutex_unlock(params->atEnd);

			n = sscanf(buf, "%d", &nreadxyz);
			if (n != 1) {
				printf("xyz format ERROR on line 1 : %s\nToo many "
						"columns on first row of frame. "
						"Make sure the input file is in xyz format.\n", buf);
				gtk_main_quit();
//		     gtk_exit(0);
			}
			if (fgets(buf, 160, fpRI) == NULL) {
				printf("Abnormal end of input.\n");
				continue;
			}
			n = sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s %s %s"
					" %s %s %s %s %s %s %s", arg[0], arg[1], arg[2], arg[3],
					arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10],
					arg[11], arg[12], arg[13], arg[14], arg[15], arg[16],
					arg[17], arg[18], arg[19]);
			timecheck = FALSE;
			for (i = 1; i < n; i++) {
				if (strcmp(arg[i], params->timedelim) == 0) {
					strcpy((char *) timestr, (char *) arg[i - 1]);
					timecheck = TRUE;
				}
			}
			if (timecheck)
				n = sscanf(timestr, "%lf", &params->framedata[NumFrameRI].atime);
			else {
				printf("Warning : Missing time variable\n");
				params->framedata[NumFrameRI].atime = -1;
			}
			if (n == 0) {
				printf("Warning : Invalid time variable : %s\n", timestr);
				params->framedata[NumFrameRI].atime = -1;
			}

			coords = (struct xyzstruc *) g_malloc(
					nreadxyz * sizeof(struct xyzstruc));
			if (coords == NULL) {
				printf("Out of memory!\nTry reducing number of frames used.\n");
				gtk_main_quit();
//		gtk_exit(0);
			}
			params->framedata[NumFrameRI].numAtoms = nreadxyz;
			if (params->framedata[NumFrameRI].atomdata != NULL)
				g_free(params->framedata[NumFrameRI].atomdata);
			params->framedata[NumFrameRI].atomdata = coords;
			endframe = FALSE;
			numatoms = 0;
			numtypes = 0;

			for (i = 0; i < nreadxyz; i++) {
				if (fgets(buf, 160, fpRI) == NULL)
					endframe = TRUE;
				n = sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s %s %s"
						"%s %s %s %s %s %s %s", arg[0], arg[1], arg[2], arg[3],
						arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10],
						arg[11], arg[12], arg[13], arg[14], arg[15], arg[16],
						arg[17], arg[18], arg[19]);
				if (params->scol > 0) {
					if (strcmp(params->fstring, arg[params->scol - 1]))
						continue;
				}
				if (n < params->xcolumn || n < params->ycolumn
						|| n < params->zcolumn || n < params->tcolumn) {
					printf("Error in xyz input file : %s\nExiting.\n", buf);
					gtk_main_quit();
//		    gtk_exit(0);
				}

				typescheck = FALSE;
				for (j = 0; j < numtypes; j++) {
					if (strcmp(AType[j], arg[0]) == 0) {
						typescheck = TRUE;
						break;
					}
				}
				if (!typescheck) {
					strcpy(AType[numtypes], arg[0]);
					coords[numatoms].atype = numtypes;
					numtypes++;
					if (numtypes > MAXTYPES) {
						printf("Maximum number of atomtypes reached!\n");
						gtk_main_quit();
//			gtk_exit(0);
					}
				} else
					coords[numatoms].atype = j;
				n = sscanf(arg[params->xcolumn - 1], "%lf",
						&coords[numatoms].xcoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->xcolumn - 1]);

				n = sscanf(arg[params->ycolumn - 1], "%lf",
						&coords[numatoms].ycoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->ycolumn - 1]);

				n = sscanf(arg[params->zcolumn - 1], "%lf",
						&coords[numatoms].zcoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->zcolumn - 1]);

				if (coords[numatoms].xcoord > maxx)
					maxx = coords[numatoms].xcoord;
				if (coords[numatoms].ycoord > maxy)
					maxy = coords[numatoms].ycoord;
				if (coords[numatoms].zcoord > maxz)
					maxz = coords[numatoms].zcoord;
				if (coords[numatoms].xcoord < minx)
					minx = coords[numatoms].xcoord;
				if (coords[numatoms].ycoord < miny)
					miny = coords[numatoms].ycoord;
				if (coords[numatoms].zcoord < minz)
					minz = coords[numatoms].zcoord;

				numatoms++;
				if (endframe) {
					printf("Anomaly : End of file reached !\n");
					break;
				}
			}
			if (params->xmin == 65535.0) {
				params->framedata[NumFrameRI].xmax = maxx;
				params->framedata[NumFrameRI].xmin = minx;
			} else {
				params->framedata[NumFrameRI].xmax = params->xmax;
				params->framedata[NumFrameRI].xmin = params->xmin;
			}
			if (params->ymin == 65535.0) {
				params->framedata[NumFrameRI].ymax = maxy;
				params->framedata[NumFrameRI].ymin = miny;
			} else {
				params->framedata[NumFrameRI].ymax = params->ymax;
				params->framedata[NumFrameRI].ymin = params->ymin;
			}
			if (params->zmin == 65535.0) {
				params->framedata[NumFrameRI].zmax = maxz;
				params->framedata[NumFrameRI].zmin = minz;
			} else {
				params->framedata[NumFrameRI].zmax = params->zmax;
				params->framedata[NumFrameRI].zmin = params->zmin;
			}

			params->numtypes = numtypes;
			g_mutex_unlock(params->framedata[NumFrameRI].frameready);
			NumFrameRI++;
			if (NumFrameRI == NUMFRAMES)
				NumFrameRI = 0;
			if (endframe) {
				printf("endframe = TRUE\n");
				return NULL;
			}
		}

		/* If not in xyz format start redaing from here ! */

		else {
			numalloc = ALLOCTHIS;
			i = 0;
			coords = (struct xyzstruc *) g_malloc(
					numalloc * sizeof(struct xyzstruc));
			if (coords == NULL) {
				printf("Out of memory!\nTry reducing number of frames used.\n");
				gtk_main_quit();
//		gtk_exit(0);
			}
			if (framecheck) {
				coords[i].xcoord = lastframe.xcoord;
				coords[i].ycoord = lastframe.ycoord;
				coords[i].zcoord = lastframe.zcoord;
				coords[i].tcoord = lastframe.tcoord;
				i++;
			}
			framecheck = TRUE;
			endframe = TRUE;
			while (fgets(buf, 160, fpRI) != NULL) {
				if (i + 1 == numalloc) {
					numalloc += ALLOCTHIS;
					coords = g_realloc(coords,
							numalloc * sizeof(struct xyzstruc));
					if (coords == NULL) {
						printf(
								"Out of memory!\nTry reducing number of frames used.\n");
						gtk_main_quit();
//			gtk_exit(0);
					}
				}
				n = sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s %s %s "
						"%s %s %s %s %s %s %s", arg[0], arg[1], arg[2], arg[3],
						arg[4], arg[5], arg[6], arg[7], arg[8], arg[9], arg[10],
						arg[11], arg[12], arg[13], arg[14], arg[15], arg[16],
						arg[17], arg[18], arg[19]);
				if (params->scol > 0) {
					if (strcmp(params->fstring, arg[params->scol - 1]))
						continue;
				}
				if (n < params->xcolumn || n < params->ycolumn
						|| n < params->zcolumn || n < params->tcolumn) {
					printf(
							"Error in input file : %s\nAre you sure the input file isn't in xyz "
									"format ?\nExiting.\n", buf);
					gtk_main_quit();
//		    gtk_exit(0);
				}
				n = sscanf(arg[params->xcolumn - 1], "%lf", &coords[i].xcoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->xcolumn - 1]);
				n = sscanf(arg[params->ycolumn - 1], "%lf", &coords[i].ycoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->ycolumn - 1]);
				n = sscanf(arg[params->zcolumn - 1], "%lf", &coords[i].zcoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->zcolumn - 1]);
				n = sscanf(arg[params->tcolumn - 1], "%lf", &coords[i].tcoord);
				if (n == 0)
					printf("There seems to be a problem with converting \'%s\'"
							" to a number.\n", arg[params->tcolumn - 1]);
				if (coords[i].tcoord == coords[0].tcoord) {
					if (coords[i].xcoord > maxx)
						maxx = coords[i].xcoord;
					if (coords[i].ycoord > maxy)
						maxy = coords[i].ycoord;
					if (coords[i].zcoord > maxz)
						maxz = coords[i].zcoord;
					if (coords[i].xcoord < minx)
						minx = coords[i].xcoord;
					if (coords[i].ycoord < miny)
						miny = coords[i].ycoord;
					if (coords[i].zcoord < minz)
						minz = coords[i].zcoord;
					i++;
				} else {
					endframe = FALSE;
					break;
				}
			}
			params->framedata[NumFrameRI].atime = coords[i - 1].tcoord;
			lastframe.xcoord = coords[i].xcoord;
			lastframe.ycoord = coords[i].ycoord;
			lastframe.zcoord = coords[i].zcoord;
			lastframe.tcoord = coords[i].tcoord;
			if (params->xmin == 65535.0) {
				params->framedata[NumFrameRI].xmax = maxx;
				params->framedata[NumFrameRI].xmin = minx;
			} else {
				params->framedata[NumFrameRI].xmax = params->xmax;
				params->framedata[NumFrameRI].xmin = params->xmin;
			}
			if (params->ymin == 65535.0) {
				params->framedata[NumFrameRI].ymax = maxy;
				params->framedata[NumFrameRI].ymin = miny;
			} else {
				params->framedata[NumFrameRI].ymax = params->ymax;
				params->framedata[NumFrameRI].ymin = params->ymin;
			}
			if (params->zmin == 65535.0) {
				params->framedata[NumFrameRI].zmax = maxz;
				params->framedata[NumFrameRI].zmin = minz;
			} else {
				params->framedata[NumFrameRI].zmax = params->zmax;
				params->framedata[NumFrameRI].zmin = params->zmin;
			}

			params->framedata[NumFrameRI].numAtoms = i;
			if (params->framedata[NumFrameRI].atomdata != NULL)
				g_free(params->framedata[NumFrameRI].atomdata);
			params->framedata[NumFrameRI].atomdata = coords;
			g_mutex_unlock(params->framedata[NumFrameRI].frameready);
			NumFrameRI++;
			if (NumFrameRI == NUMFRAMES)
				NumFrameRI = 0;
			if (endframe) {
				framecheck = FALSE;
			} else
				g_mutex_unlock(params->atEnd);
		}
	}
}
