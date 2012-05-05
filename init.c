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
#include <stdio.h>
#include <string.h>
#include "parameters.h"

/************************************************************************/
/* This function is called if the help parameter has been given. 	*/
/* It prints a helppage for the usage of gdpc.				*/
/************************************************************************/
void printhelp() {
	printf("\n");
	printf(" Usage: gdpc [Options] <xcol> <ycol> <zcol> <tcol> <input-file>\n");
	printf("    Options:\n");
	printf(
			"\ts <xpixels> <ypixels>  Set drawable screenarea (default: 600 600)\n");
	printf(
			"\tcube <distance>        Set the volume to be drawn, distance from origo\n");
	printf("\t                       in all directions\n");
	printf("\tm <0/1/2>              Set drawingmode to use(default: 1)\n");
	printf(
			"\td <pixels>             Set drawingradius of each atom (default: 2)\n");
	printf(
			"\tx <xcoord> <xcoord>    Set the size to be drawn in x-direction\n");
	printf(
			"\ty <ycoord> <ycoord>    Set the size to be drawn in y-direction\n");
	printf(
			"\tz <zcoord> <zcoord>    Set the size to be drawn in z-direction\n");
	printf("\tsleep <seconds>        Set the minimum time between frames\n");
	printf(
			"\tbsleep                 Wait for middle mousebutton click on pixmap\n");
	printf(
			"\t                       or space key key press in window before showing\n");
	printf("\t                       the next frame\n");
	printf(
			"\terase                  Clear drawable area after each frame (default: off)\n");
	printf("\tw                      Use white background (default: off)\n");
	printf(
			"\tf <colnum> <string>    Just process linues with <string> in <colnum>\n");
	printf("\tgreyscale              Use greyscale colorset\n");
	printf("\tcolorinv               Use inverted colors\n");
	printf("\tcoldcolors             Use cold colorset\n");
	printf("\tcoldcolors2            Use cold colorset 2\n");
	printf(
			"\tv <1/2>                Vary atomsize according to z-column data\n");
	printf("\tsortr                  Sort atoms in reverse order\n");
	printf("\tusetypes               Color atoms depending on their type.\n");
	printf(
			"\ttimedel <delim>        Set the delimiter for the time in xyz header.\n");
	printf(
			"\tpngdump <name>         Dumps an image of each frame, see below\n");
	printf(
			"\tjpgdump <name>         Dumps an image of each frame, see below\n");
	printf(
			"\tdumpnum                Dumped images are named after framenumber.\n");
	printf(
			"\tonce                   Exit automatically after all frames has been shown.\n");
	printf("\trotate <x> <y> <z>     Use initial <x>, <>y and <z> rotations\n");
	printf(
			"\txyz                    Input file is in xyz format (default: off)\n");
	printf("\n");
	printf(" The drawingmodes are :          0 plain rectangles\n");
	printf("                                 1 plain circles\n");
	printf("                                 2 rendered balls\n");
	printf(" Varying of the size modes are : 1 Size decreases with z\n");
	printf("                                 2 Size increases with z\n\n");
	printf(
			" - If neither cube,x,y or z maxsize is determined by input coordinates.\n");
	printf(" - Atoms are automatically sorted by x, y and z.\n");
	printf(
			" - The filename given to the pngdump/jpgdump parameter should be given without\n");
	printf(
			"   extension, gdpc adds the time or number of the frame and the extension to\n");
	printf("   the end of the filename.\n");
	printf(" - If input file is in xyz format the t column will be ignored\n");
	printf(
			" - The usetypes parameter is not relevant if not used with xyz input file, and\n");
	printf("   will then be ignored.\n");
	printf(
			" - The only mandatory parameters are the column representations and the input\n");
	printf("   file.\n");
	printf(" - To read input data from stdin set filename to _\n");
	printf("\n");
	printf(" Original idea from dpc by Kai Nordlund.\n\n");
}

/************************************************************************/
/* This function handles the parameters given at the command line, and	*/
/* sets the variables accordingly.					*/
/************************************************************************/
struct Configuration * handleArgs(int args, char **argv) {
	gint argl = 0, inttmp, control;
	gchar c[64];
	double tmp;
	gboolean setxcol, setycol, setzcol, settcol, setfile;
	struct Configuration *config;

	config = getNewConfiguration();
	if (config == NULL) {
		return NULL;
	}

	setxcol = FALSE;
	setycol = FALSE;
	setzcol = FALSE;
	settcol = FALSE;
	setfile = FALSE;

	while (args - 1 > argl) {
		strncpy(c, argv[argl + 1], strlen(argv[argl + 1]));
		c[strlen(argv[argl + 1])] = '\0';

		if (!strcmp(c, "s") && !setxcol && !setycol && !setzcol && !settcol) {
			control = sscanf(argv[argl + 2], "%d", &(config->absxsize));
			if (control == 0) {
				printf("First parameter is invalid or missing for option: s\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			control = sscanf(argv[argl + 3], "%d", &(config->absysize));
			if (control == 0) {
				printf(
						"Second parameter is invalid or missing for option: s\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->absxsize < 100 || config->absysize < 100) {
				printf("The window should be at least 100*100 pixels.\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 3;
		} else if (!strcmp(c, "x") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%lf", &(config->xmin));
			if (control == 0) {
				printf("First parameter is invalid or missing for option: x\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			control = sscanf(argv[argl + 3], "%lf", &(config->xmax));
			if (control == 0) {
				printf(
						"Second parameter is invalid or missing for option: x\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->xmin >= config->xmax) {
				printf("xmin must be smaller than xmax!\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 3;
		} else if (!strcmp(c, "y") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%lf", &(config->ymin));
			if (control == 0) {
				printf("First parameter is invalid or missing for option: y\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			control = sscanf(argv[argl + 3], "%lf", &(config->ymax));
			if (control == 0) {
				printf(
						"Second parameter is invalid or missing for option: y\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->ymin >= config->ymax) {
				printf("ymin must be smaller than ymax!\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 3;
		} else if (!strcmp(c, "z") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%lf", &(config->zmin));
			if (control == 0) {
				printf("First parameter is invalid or missing for option: z\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			control = sscanf(argv[argl + 3], "%lf", &(config->zmax));
			if (control == 0) {
				printf(
						"Second parameter is invalid for or missing option: z\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->zmin >= config->zmax) {
				printf("zmin must be smaller than zmax!\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 3;
		} else if (!strcmp(c, "cube") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%lf", &tmp);
			if (control == 0) {
				printf("Invalid or missing parameter for option: cube\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (tmp <= 0.0) {
				printf("cube value must be larger than 0.0!\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->xmin == 65535.0) {
				config->xmin = -tmp;
				config->xmax = tmp;
			}
			if (config->ymin == 65535.0) {
				config->ymin = -tmp;
				config->ymax = tmp;
			}
			if (config->zmin == 65535.0) {
				config->zmin = -tmp;
				config->zmax = tmp;
			}
			argl += 2;
		} else if (!strcmp(c, "timedel") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%s", config->timedelim);
			if (control == 0) {
				printf("Invalid or missing parameter for option: timedel\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 2;
		} else if (!strcmp(c, "erase") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->erasePreviousFrame = TRUE;
			argl++;
		} else if (!strcmp(c, "w") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->backgroundWhite = TRUE;
			argl++;
		} else if (!strcmp(c, "colorinv") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->colorset = 1;
			argl++;
		} else if (!strcmp(c, "coldcolors") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->colorset = 2;
			argl++;
		} else if (!strcmp(c, "coldcolors2") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->colorset = 3;
			argl++;
		} else if (!strcmp(c, "greyscale") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->colorset = 4;
			argl++;
		} else if (!strcmp(c, "sort") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->sort = 1;
			argl++;
		} else if (!strcmp(c, "sortr") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->sort = 2;
			argl++;
		} else if (!strcmp(c, "xyz") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->inputFormatXYZ = TRUE;
			argl++;
		} else if (!strcmp(c, "dumpnum") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->dumpnum = TRUE;
			argl++;
		} else if (!strcmp(c, "m") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%d", &(config->mode));
			if (control == 0) {
				printf("Invalid or missing parameter for option: m\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->mode > 2 || config->mode < 0) {
				printf("Unknown drawingmode : %d\n", config->mode);
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 2;
		} else if (!strcmp(c, "d") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%d", &(config->radius));
			if (control == 0) {
				printf("Invalid or missing parameter for option: d\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->radius < 2)
				config->radius = 2;
			if (config->radius > 25)
				config->radius = 25;
			argl += 2;
		} else if (!strcmp(c, "sleep") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%lf", &tmp);
			if (control == 0) {
				printf("Invalid or missing parameter for option: sleep\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 2;
			config->interval = (gint) 1000 * tmp;
//	    if (params->interval < 0) params->interval = 0;
		} else if (!strcmp(c, "v") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%d", &(config->vary));
			if (control == 0) {
				printf("Invalid or missing parameter for option: v\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->vary > 2 || config->vary < 1) {
				printf("Vary has only two valid modes: 1 or 2 !\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			argl += 2;
		} else if (!strcmp(c, "f") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%d", &(config->scol));
			if (control == 0) {
				printf("Invalid or missing parameter for option: f\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			if (config->scol < 1) {
				printf("Column must be greater than zero!\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			sscanf(argv[argl + 3], "%s", config->fstring);
			argl += 3;
		} else if (!strcmp(c, "pngdump") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%s", config->dumpname);
			if (control == 0) {
				printf("Invalid or missing parameter for option: tifdump\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			config->tifjpg = TRUE;
			argl += 2;
		} else if (!strcmp(c, "jpgdump") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			control = sscanf(argv[argl + 2], "%s", config->dumpname);
			if (control == 0) {
				printf("Invalid or missing parameter for option: jpgdump\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			config->tifjpg = FALSE;
			argl += 2;
		} else if (!strcmp(c, "usetypes") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->useTypesForColoring = TRUE;
			argl++;
		} else if (!strcmp(c, "bsleep") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->waitForNextFramePress = TRUE;
			argl++;
		} else if (!strcmp(c, "help") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			printhelp();
			return NULL;
		} else if (!strcmp(c, "once") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			config->oneLoop = TRUE;
			argl++;
		} else if (!strcmp(c, "rotate") && !setxcol && !setycol && !setzcol
				&& !settcol) {
			sscanf(argv[argl + 2], "%lf", &config->initIangle);
			sscanf(argv[argl + 3], "%lf", &config->initJangle);
			sscanf(argv[argl + 4], "%lf", &config->initKangle);
			argl += 4;
		} else if (sscanf(c, "%d", &inttmp) > 0
				&& (!setxcol || !setycol || !setzcol || !settcol)) {
			if (!setxcol) {
				config->xcolumn = inttmp;
				setxcol = TRUE;
			} else if (!setycol) {
				config->ycolumn = inttmp;
				setycol = TRUE;
			} else if (!setzcol) {
				config->zcolumn = inttmp;
				setzcol = TRUE;
			} else if (!settcol) {
				config->tcolumn = inttmp;
				settcol = TRUE;
			}
			argl++;
		} else if (setxcol && setycol && setzcol && settcol) {
			control = sscanf(c, "%s", config->file);
			if (control == 0) {
				printf("Invalid or missing filename for input\n");
				printf(
						"Use option 'help' for list of all valid command line parameters\n");
				return NULL;
			}
			setfile = TRUE;
			argl += 1;
			break;
		} else {
			printf("Unknown option: %s\n", c);
			printf("Please check your command line arguments.\n");
			printf(
					"Use option 'help' for list of all valid command line parameters\n");
			return NULL;
		}
	}

	if (!config->inputFormatXYZ)
		config->useTypesForColoring = FALSE;
	if (setxcol && setycol && setzcol && settcol && setfile)
		return config;
	else {
		printf(
				"You at least have to specify input data columns and input file if\n");
		printf("you're going to start gdpc from the commandline.\n\n");
	}
	return NULL;
}
