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

/* Here the versionnumber of gdpc is set, that will be reported. */

#define GDPCVER "1.0.0"

/* Define the value of pi */

#define PI 3.141592654

/* Define how large the border around the simulationbox should be,
 if you change this remember to also change it in main.c.        */

#define xborder 15		/* X border */
#define yborder 15		/* Y border */

/* Allocate this many number of structures at a time in the non-xyz
 format reading. */

#define ALLOCTHIS 1000

/* Number of colors to use, dont change this if youre not also 
 changing colors.c.                                          */

#define NUMCOLORS 16

/* Define the size of the drawable area without borders */

#define drawXsize 600		/* X-size */
#define drawYsize 600		/* Y-size */

/* Define the minimum interval between timeouts, if you're animation is
 very slow try reducing this. Though should not be changed unless needed */

#define MININTERVAL 0

/* Define the treshold of movement with the mousebutton pressed down before
 rotating the atoms and drawing them. */

#define ROTATETRESHOLD 0

/* Define the maximum numbers of different atom types used in animation */

#define MAXTYPES 100

/* Define the default string to search for to determine xyz-format time */

#define TIMESTRING "fs"

/* Define the number of frames to be able to read in advance from input file, should be >0 */

#define NUMFRAMES 8

/* Define debug constant, if set to TRUE additional debugging info will be printed 
 out during the running of the program. */

#define Debug FALSE

/* Define some animation default values */

#define RADIUS 5
#define MODE 1
#define COLORSET 0
#define VARY 0
#define SCOL 0
#define SORT 0
#define XCOLUMN 1
#define YCOLUMN 2
#define ZCOLUMN 3
#define TCOLUMN 4
#define WHITEBG FALSE
#define ERASE FALSE
#define FXYZ FALSE
#define DUMPNUM FALSE

/* Declaration of structure which describes the atoms data. */

struct xyzstruc {
	double xcoord; /* X-coordinate */
	double ycoord; /* Y-coordinate */
	double zcoord; /* Z-coordinate */
	double tcoord; /* t-coordinate */
	gint atype; /* atom type */
	gint index; /* index */
};

 struct FrameData {
 	double xmin; 				/* Minimum x coordinate of frame */
 	double xmax; 				/* Maximum x coordinate of frame */
 	double ymin; 				/* Minimum y coordinate of frame */
 	double ymax; 				/* Maximum y coordinate of frame */
 	double zmin; 				/* Minimum z coordinate of frame */
 	double zmax; 				/* Maximum z coordinate of frame */
 	GMutex *frameready; 		/* Control variables for 'Is the whole frame read?' */
 	GMutex *framecomplete; 		/* Control variables for 'Has the frame been completely handled?' */
 	GMutex *framedrawn; 		/* Control variables for 'Is the frame currently being drawn?' */
 	gint numAtoms; 				/* Number of atoms in frame */
 	struct xyzstruc *atomdata;	/* Data of frame */
 	double atime; 				/* Timestamp of frame */
 	gboolean lastFrame;
 };


/* Declaration of structure used for storing internal data of the program */

struct GlobalParams {
	GtkWidget *drawing_area;
	gint xcolumn; /* Which column is the x coordinate */
	gint ycolumn; /* Which column is the y coordinate */
	gint zcolumn; /* Which column is the z coordinate */
	gint tcolumn; /* Which column is the t coordinate */
	gint absxsize; /* Size of drawing area x-wise */
	gint absysize; /* Size of drawing area y-wise */
	gint colorset; /* Set of colors to use */
	double xcolorset[17][3];
	gint mode; /* Drawing mode */
	gint radius; /* Radius of drawable objects */
	gint sort; /* Method of sorting atoms */
	gint vary; /* Method of varying drawable objectsize */
	gint scol; /* Something */
	gint xpress; /* x coordinate for mousebuttonpress */
	gint ypress; /* y coordinate for mousebuttonpress */
	gint oldxc; /* Old angular correction, x-wise */
	gint oldyc; /* Old angular correction, y-wise */
	gint oldzc; /* Old angular correction, z-wise */
	gint oldtc; /* Old angular correction, t-wise */
	gint oldxsize; /* Old absxsize */
	gint oldysize; /* Old absysize */
	gint numframe; /* Number of the frame */
	gint interval; /* Interval in time between frames */
	gint numtypes; /* Number of atomtypes */
	double iangle; /* Angle of view around x */
	double jangle; /* Angle of view around y */
	double kangle; /* Angle of view around z */
	double imangle; /* Angle change according to mouse movement x-wise */
	double jmangle; /* Angle change according to mouse movement y-wise */
	double xmin; /* Minumum x coordinate */
	double xmax; /* Maximum x coordinate */
	double ymin; /* Minumum y coordinate */
	double ymax; /* Maximum y coordinate */
	double zmin; /* Minumum z coordinate */
	double zmax; /* Maximum z coordinate */
	double xc; /* Angular correction, x-wise */
	double yc; /* Angular correction, y-wise */
	double zc; /* Angular correction, z-wise */
	gboolean mbsleep; /* Do we want to wait after every frame for a middle button press ? */
	gboolean pausecheck; /* Is animation on pause ? */
	gboolean setupstop; /* Is the animation being configured ? */
	gboolean whitebg; /* Do we want a white background ? */
	gboolean erase; /* Do we want to erase the old frame before drawing a new one ? */
	gboolean fxyz; /* Is input in xyz-format ? */
	gboolean pressed; /* Is mousebutton pressed down on pixmap ? */
	gboolean dumpnum; /* Do we want number-of-frame or timestamp on dumped images ? */
	gboolean tifjpg; /* Do we want tifs or jpgs to be dumped ? */
	gboolean StartedAlready; /* Is the animation started ? */
	gboolean usetypes; /* Will the be coloring according to atomtypes ? */
	gboolean once;
	gchar fstring[30]; /* String to check for in inputlines */
	gchar file[256]; /* Name of input file */
	gchar dumpname[50]; /* Names of dumped images */
	gchar timedelim[20]; /* Delimiter for time readings in xyz-format */
	FILE *fp; /* File pointer */
	GMutex *filewait; /* Wait for file control variable */
	GMutex *atEnd; /* Whole file read in control variable */
	struct FrameData framedata[NUMFRAMES];
};

/* Declaration of structure used for passing information to drawing functions */
struct DrawStruct {
	cairo_t *cr;
	gint crXSize, crYSize;
	struct GlobalParams *params;
	gint nextFrameNum;
	struct FrameData *currentFrame;
};

/* Declaration of extern functions used throughout the program */

void StartEverything(struct GlobalParams *params);
void setupStartOk(struct GlobalParams *params);
void setupStartCancel(struct GlobalParams *params);
void SetupRedraw(struct GlobalParams *params);

void clearDrawable(struct DrawStruct DrawData);

void setupwindow(struct GlobalParams *params);

void rotateAtoms(struct DrawStruct DrawData);

void mouserotate(GtkWidget *widget, gint xdelta, gint ydelta,
		struct GlobalParams *params);

void xplusb(GtkWidget *widget, struct GlobalParams *params);
void yplusb(GtkWidget *widget, struct GlobalParams *params);
void zplusb(GtkWidget *widget, struct GlobalParams *params);
void xminusb(GtkWidget *widget, struct GlobalParams *params);
void yminusb(GtkWidget *widget, struct GlobalParams *params);
void zminusb(GtkWidget *widget, struct GlobalParams *params);
void xplus10b(GtkWidget *widget, struct GlobalParams *params);
void yplus10b(GtkWidget *widget, struct GlobalParams *params);
void zplus10b(GtkWidget *widget, struct GlobalParams *params);
void xminus10b(GtkWidget *widget, struct GlobalParams *params);
void yminus10b(GtkWidget *widget, struct GlobalParams *params);
void zminus10b(GtkWidget *widget, struct GlobalParams *params);
void resetob(GtkWidget *widget, struct GlobalParams *params);

void sortatoms(struct xyzstruc *coords, gint left, gint right, gboolean sort);

void triggerImageRedraw(GtkWidget *widget, struct GlobalParams *params);
void resetic();

gint switchToNextFrame(struct GlobalParams *params);

//gint allocatecolors(GdkColor **colors, gint colorset, gint mode);
void setColorset(struct GlobalParams *params, gint colorsetnum);

void * readInput(struct GlobalParams *params);

gboolean handleargs(int args, char **argv, struct GlobalParams *params);
