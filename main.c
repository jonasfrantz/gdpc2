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
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parameters.h"
#include <stddef.h>
#include <unistd.h>
#include <sys/time.h> 

/* Some internal variables declared next */
GtkWidget *time_entry;
GtkWidget *coord_entry;
GtkWidget *maxx_entry, *maxy_entry, *maxz_entry;
GtkWidget *xc_entry, *yc_entry, *zc_entry;

gboolean MB_pressed;

gint retcode; /* Return value from create_thread command */
GThread *th_a; /* Thread structure */

struct DrawStruct DrawData; /* Universal structure drawing variables */

extern gint NumFrameRI; /* Contains which frame is about to be
 read from the input data */
extern double FrameTime; /* Contains time of frame just shown */
extern FILE *NewFP; /* Filepointer to new file if one has
 been chosen, otherwise = NULL */

extern gint lastframedone; /* Have we shown the last frame ? */

/************************************************************************/
/* This function is called when the quit button is pressed.		*/
/************************************************************************/
void quit(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

/************************************************************************/
/* This function is called when the pause button is pressed.		*/
/************************************************************************/
void pauseb(GtkToggleButton *widget, struct GlobalParams *params) {
#if Debug 
	if (gtk_toggle_button_get_active(widget))
	printf("Setting animation on pause.\n");
	else printf("Unpausing animation.\n");
#endif

	params->pausecheck = gtk_toggle_button_get_active(widget);
}

/************************************************************************/
/* This function is called when the restart button is pressed.   	*/
/* It simply resets the filepointer and clears the drawingboard. 	*/
/************************************************************************/
void restart(GtkWidget *widget, struct GlobalParams *params) {
	gint i;

#if Debug 
	printf("Restarting animation.\n");
	printf("Clearing framedrawn semaphores.\n");
#endif

	for (i = NUMFRAMES - 1; i >= 0; i--) {
		g_mutex_trylock(params->framedata[i].framedrawn);
	}

#if Debug
	printf("Opening new file.\n");
#endif

	NewFP = fopen(params->file, "r");
	if (NewFP == NULL) {
		printf("Error opening file: %s\n", params->file);
		gtk_main_quit();
	}
	fseek(NewFP, SEEK_SET, 0);
	g_mutex_unlock(params->atEnd);
	cleardrawable(DrawData);
	params->numframe = 1;

#if Debug
	printf("Reinitialize filewait/frameready/framedrawn semaphores.\n");
#endif

	g_mutex_unlock(params->filewait);

	for (i = NUMFRAMES - 1; i >= 0; i--) {
		g_mutex_trylock(params->framedata[i].frameready);
	}

	DrawData.NumFrame = NumFrameRI;

	for (i = 0; i < NUMFRAMES; i++) {
		g_mutex_unlock(params->framedata[i].framedrawn);
	}
}

/************************************************************************/
/* This function is called when the setup button is pressed.		*/
/* It stops the animation and calls the setupwindow function.		*/
/************************************************************************/
void setup(GtkWidget *widget, struct GlobalParams *params) {
#if Debug
	printf("Starting setup window.\n");
#endif

	params->setupstop = TRUE;
	params->oldxc = params->xcolumn;
	params->oldyc = params->ycolumn;
	params->oldzc = params->zcolumn;
	params->oldtc = params->tcolumn;
	params->oldxsize = params->absxsize;
	params->oldysize = params->absysize;
	setupwindow(params);
}

/************************************************************************/
/************************************************************************/
gboolean updateImageArea(GtkWidget *widget, cairo_t *cr,
		struct GlobalParams *params) {
	guint width, height;
	cairo_t *first_cr;
	cairo_surface_t *first;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	first = cairo_surface_create_similar(cairo_get_target(cr),
			CAIRO_CONTENT_COLOR, width, height);

	first_cr = cairo_create(first);

	DrawData.cr = first_cr;
	DrawData.crXSize = width;
	DrawData.crYSize = height;

	cleardrawable(DrawData);
	rotateatoms(DrawData);

	cairo_set_source_surface(cr, first, 0, 0);
	cairo_paint(cr);

	cairo_surface_destroy(first);

	cairo_destroy(first_cr);

	return TRUE;
}

/************************************************************************/
/* This procedure is called when keyboard key is pressed.		*/
/* Escape key quits gdpc, space is used with the bsleep option.		*/
/************************************************************************/
gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	switch (event->keyval) {
	case GDK_KEY_Escape:
		gtk_main_quit();
		break;

	case GDK_KEY_space:
		MB_pressed = TRUE;
		return FALSE;
		break;

	default:
		return FALSE;
		break;
	}
	return TRUE;
}

/************************************************************************/
/* This procedure is called when a mousebutton is pressed.		*/
/* If the right button is pressed it quits, to be backwards compatible	*/
/* with dpc. When the left button is pressed this procedure saves the	*/
/* position of the cursor.						*/
/************************************************************************/
gint button_press_event(GtkWidget *widget, GdkEventButton *event,
		struct GlobalParams *params) {
#if Debug
	printf("Button pressed.\n");
#endif

	if (event->button == 1) {
		params->pressed = TRUE;
		params->xpress = event->x;
		params->ypress = event->y;
	} else if (event->button == 2) {
		MB_pressed = TRUE;
	} else if (event->button == 3) {
		gtk_main_quit();
	}
	return TRUE;
}

/************************************************************************/
/* This procedure is called when a mousebutton is released.		*/
/* When released the procedure ends the tracking of mouse movement and	*/
/* rotates the atoms to their final position.				*/
/************************************************************************/
gint button_release_event(GtkWidget *widget, GdkEventButton *event,
		struct GlobalParams *params) {
#if Debug
	printf("Button released.\n");
#endif

	if (event->button == 1) {
		params->pressed = FALSE;
		mouserotate(widget, (params->xpress - event->x),
				(params->ypress - event->y), params);
	} else if (event->button == 2) {
	} else if (event->button == 3) {
		gtk_main_quit();
	}
	return TRUE;
}

/************************************************************************/
/* This function is called when the mouse is moved inside the window,	*/
/* it sets the coordinates in the coordinateentry. It also rotates the	*/
/* atoms according to the movement of the mouse while the left button	*/
/* is pressed down.							*/
/************************************************************************/
gint motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
		struct GlobalParams *params) {
	gint x, y;
	char xstr[64];
	GdkModifierType state;
	gint NumFrame;

#if Debug
	printf("Fetching coordinates of pointer.\n");
#endif

	gdk_window_get_pointer(event->window, &x, &y, &state);

	if (params->pressed) {
		if (params->xpress - x > ROTATETRESHOLD
				|| params->ypress - y > ROTATETRESHOLD
				|| params->xpress - x < -ROTATETRESHOLD
				|| params->ypress - y < -ROTATETRESHOLD) {
#if Debug
			printf("Starting rotating of scene.\n");
#endif
			mouserotate(widget, (params->xpress - x), (params->ypress - y),
					params);
			params->xpress = x;
			params->ypress = y;
		}
	}

	NumFrame = DrawData.NumFrame;
	NumFrame--;
	if (NumFrame < 0)
		NumFrame += NUMFRAMES;

#if Debug
	printf("Fetching and setting coordinates of pointer at scene.\n");
#endif
	sprintf(
			xstr,
			"X: %5.3f   Y: %5.3f",
			(((params->framedata[NumFrame].xmax
					- params->framedata[NumFrame].xmin) * (x - xborder)
					/ (double) params->absxsize)
					+ params->framedata[NumFrame].xmin),
			(((params->framedata[NumFrame].ymax
					- params->framedata[NumFrame].ymin)
					* (params->absysize - (y - yborder))
					/ (double) params->absysize))
					+ params->framedata[NumFrame].ymin);
	gtk_entry_set_text((GtkEntry *) coord_entry, xstr);

	return TRUE;
}

/************************************************************************/
/* This function is called at the end of setupwindow if the ok button 	*/
/* was pressed, it reinitializes gdpc if necessary and then continous	*/
/* the animation.							*/
/************************************************************************/
void SetupStartOk(struct GlobalParams *params) {
	if (params->absxsize != params->oldxsize
			|| params->absysize != params->oldysize) {
		gtk_widget_set_size_request(params->drawing_area,
				params->absxsize + 2 * xborder, params->absysize + 2 * yborder);

		params->redrawcheck = TRUE;
	}

	setColorset(params, params->colorset);

	if (params->xcolumn != params->oldxc || params->ycolumn != params->oldyc
			|| params->zcolumn != params->oldzc
			|| params->tcolumn != params->oldtc) {
		fseek(params->fp, SEEK_SET, 0);
		g_mutex_unlock(params->atEnd);
		cleardrawable(DrawData);
	}
	if (strlen(params->file) > 0) {
		fclose(params->fp);
		params->fp = fopen(params->file, "r");
		if (params->fp == NULL) {
			printf("Error opening file: %s\n", params->file);
			gtk_main_quit();
		}
		fseek(params->fp, 0, 0);
		params->numframe = 1;
		g_mutex_unlock(params->atEnd);
		cleardrawable(DrawData);
	} else {
		rotateatoms(DrawData);
		params->rotated = TRUE;
	}
	params->setupstop = FALSE;
}

/************************************************************************/
/* This function is called at the end of setwindow if the cancel button */
/* was pressed. It simply continous the animation.						*/
/************************************************************************/
void SetupStartCancel(struct GlobalParams *params) {
	params->setupstop = FALSE;
}

/************************************************************************/
/* This function is called when there has been a change in angle from	*/
/* pressing a button.													*/
/************************************************************************/
void drawrotate(GtkWidget *widget, struct GlobalParams *params) {
#if Debug
	printf("Drawing rotated scene.\n");
#endif

	DrawData.NumFrame--;
	gtk_widget_queue_draw(params->drawing_area);
	DrawData.NumFrame++;
	params->rotated = TRUE;
}

/************************************************************************/
/* This function is the callback for the timeout instruction.		*/
/* It checks if a frame is being drawed or pause is pressed or if the	*/
/* animation is at the end, else it calls drawatoms which starts drawing*/
/* the next frame. When drawatoms is done, if updates the time in the	*/
/* timeentry and puts the pixmap onto the screen.			*/
/************************************************************************/
gboolean drawnext(struct GlobalParams *params) {
	char tstr[64];
	char picname[128];
	char pictype[16];
	struct timeval tv;
	struct timezone tz;
	static long previous_usec = 0;
	static long previous_sec = 0;

#if Debug
	printf("Idle callback called.\n");
#endif

	gettimeofday(&tv, &tz);

//printf("%f\n",((double)tv.tv_sec-previous_sec)*1000 + ((double) tv.tv_usec-previous_usec)/1000);

	if ((((double) tv.tv_sec - previous_sec) * 1000
			+ ((double) tv.tv_usec - previous_usec) / 1000)
			> ((double) params->interval)) {
		previous_usec = tv.tv_usec;
		previous_sec = tv.tv_sec;

		if (params->drawcheck && !params->pausecheck && !params->setupstop
				&& (!params->mbsleep || MB_pressed)) {
			params->drawcheck = FALSE;
			MB_pressed = FALSE;
			if (g_mutex_trylock(params->framedata[DrawData.NumFrame].frameready)
					== TRUE) {

#if Debug
				printf("Calling drawing function.\n");
#endif

				gtk_widget_queue_draw(params->drawing_area);

				params->redrawcheck = TRUE;

#if Debug
				printf("Done drawing.\n");
#endif

#if Debug
				printf("Setting entryboxes.\n");
#endif

				sprintf(tstr, "X: %4.3f - %4.3f",
						params->framedata[DrawData.NumFrame].xmin,
						params->framedata[DrawData.NumFrame].xmax);
				gtk_entry_set_text((GtkEntry *) maxx_entry, tstr);
				sprintf(tstr, "Y: %4.3f - %4.3f",
						params->framedata[DrawData.NumFrame].ymin,
						params->framedata[DrawData.NumFrame].ymax);
				gtk_entry_set_text((GtkEntry *) maxy_entry, tstr);
				sprintf(tstr, "Z: %4.3f - %4.3f",
						params->framedata[DrawData.NumFrame].zmin,
						params->framedata[DrawData.NumFrame].zmax);
				gtk_entry_set_text((GtkEntry *) maxz_entry, tstr);

				sprintf(tstr, "Time: %5.3f fs", FrameTime);
				gtk_entry_set_text((GtkEntry *) time_entry, tstr);

#if Debug
				printf("%s\n",params->dumpname);
#endif
				if (params->dumpname[0] != '\0') {
#if Debug
					printf("Creating image of frame to dump.\n");
#endif

					if (params->tifjpg) {
#if Debug
						printf("Dumping png.\n");
#endif
						sprintf(pictype, "png");
						if (params->dumpnum)
							sprintf(picname, "%s-%d.png", params->dumpname,
									params->numframe);
						else
							sprintf(picname, "%s-%5.3f.png", params->dumpname,
									FrameTime);
					} else {
#if Debug
						printf("Dumping jpg.\n");
#endif
						sprintf(pictype, "jpeg");
						if (params->dumpnum)
							sprintf(picname, "%s-%d.jpg", params->dumpname,
									params->numframe);
						else
							sprintf(picname, "%s-%5.3f.jpg", params->dumpname,
									FrameTime);
					}
				}
				params->numframe++;

				g_mutex_unlock(params->framedata[DrawData.NumFrame].framedrawn);
				DrawData.NumFrame++;
				DrawData.currentFrame = &(params->framedata[DrawData.NumFrame]);
				if (DrawData.NumFrame == NUMFRAMES)
					DrawData.NumFrame = 0;
				params->drawcheck = TRUE;
			} else
				params->drawcheck = TRUE;
		}
	}

#if Debug
	printf("Done with drawing part of timeoutcallback.\n");
#endif

	if (params->rotated) {
#if Debug
		printf("Writing out rotated frame pixmap to window.\n");
#endif
		params->rotated = FALSE;
	}

#if Debug
	printf("Setting angle entryboxes.\n");
#endif

	sprintf(tstr, "X angle: %f", params->xc);
	gtk_entry_set_text((GtkEntry *) xc_entry, tstr);
	sprintf(tstr, "Y angle: %f", params->yc);
	gtk_entry_set_text((GtkEntry *) yc_entry, tstr);
	sprintf(tstr, "Z angle: %f", params->zc);
	gtk_entry_set_text((GtkEntry *) zc_entry, tstr);

	if (params->once && lastframedone)
		gtk_main_quit();

	return TRUE;
}

/************************************************************************/
/* This function is called when the redraw button is pressed in the 	*/
/* setup window. It just redraws the frame.				*/
/************************************************************************/
void SetupRedraw(struct GlobalParams *params) {
	if (params->absxsize != params->oldxsize
			|| params->absysize != params->oldysize) {
		gtk_widget_set_size_request(params->drawing_area,
				params->absxsize + 2 * xborder, params->absysize + 2 * yborder);

		params->redrawcheck = TRUE;
		cleardrawable(DrawData);
	}

	if (params->erase)
		cleardrawable(DrawData);
	setColorset(params, params->colorset);

	rotateatoms(DrawData);
	params->rotated = TRUE;
	params->oldxc = params->xcolumn;
	params->oldyc = params->ycolumn;
	params->oldzc = params->zcolumn;
	params->oldtc = params->tcolumn;
	params->oldxsize = params->absxsize;
	params->oldysize = params->absysize;
}

/************************************************************************/
/* The StartEverything function is called by main() after the 		*/
/* commandline arguments or the setupwindow has finished processing of 	*/
/* the parameters. This function sets up all the buttons, entries, 	*/
/* boxes,the timeout and the drawingboard.				*/
/************************************************************************/
void StartEverything(struct GlobalParams *params) {
	GtkWidget *vbox, *hbox, *vboxleft, *vboxmiddle, *vboxright, *vboxrb;
	GtkWidget *vboxcoord, *hboxx, *hboxy, *hboxz, *hboxsetup;
	GtkWidget *quit_button, *restart_button, *pause_button, *reseto_button;
	GtkWidget *setup_button, *xplus_button, *yplus_button, *zplus_button;
	GtkWidget *xminus_button, *yminus_button, *zminus_button, *xplus10_button;
	GtkWidget *yplus10_button, *zplus10_button, *xminus10_button;
	GtkWidget *yminus10_button, *zminus10_button, *xlabel, *ylabel, *zlabel;
	GtkWidget *drawing_area;
	GtkWidget *window;

	char buf[128];
	gint i;

	params->StartedAlready = TRUE;

	/* Set the window title. */
	sprintf(buf, "gdpc "GDPCVER" : %s", params->file);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), buf);
	gtk_window_set_resizable(GTK_WINDOW (window), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER (window), 5);

	g_signal_connect_swapped(G_OBJECT (window), "key_press_event",
			G_CALLBACK (key_press_event), NULL);

	/* Create boxes for outlay. */
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	vboxleft = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxmiddle = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxright = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	hboxsetup = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	vboxrb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxcoord = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	hboxx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	hboxy = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	hboxz = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_container_add(GTK_CONTAINER (window), vbox);

	xlabel = gtk_label_new(" X ");
	ylabel = gtk_label_new(" Y ");
	zlabel = gtk_label_new(" Z ");

	/* Create the drawing area. */

	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, params->absxsize + 2 * xborder,
			params->absysize + 2 * yborder);

	params->drawing_area = drawing_area;

	gtk_box_pack_start(GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);

	/* Connect the events to their procedures. */
	g_signal_connect(G_OBJECT (drawing_area), "draw",
			G_CALLBACK (updateImageArea), params);

	g_signal_connect(G_OBJECT (drawing_area), "button_press_event",
			G_CALLBACK (button_press_event), params);
	g_signal_connect(G_OBJECT (drawing_area), "button_release_event",
			G_CALLBACK (button_release_event), params);

	g_signal_connect(G_OBJECT (drawing_area), "motion_notify_event",
			G_CALLBACK (motion_notify_event), params);

	gtk_widget_set_events(
			drawing_area,
			GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
					| GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	/* Create entries for the  x- and y- coordinates of the cursor and the time. */
	maxx_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) maxx_entry, FALSE);
	maxy_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) maxy_entry, FALSE);
	maxz_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) maxz_entry, FALSE);
	xc_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) xc_entry, FALSE);
	yc_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) yc_entry, FALSE);
	zc_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) zc_entry, FALSE);
	time_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) time_entry, FALSE);
	coord_entry = gtk_entry_new();
	gtk_editable_set_editable((GtkEditable *) coord_entry, FALSE);
	gtk_box_pack_start(GTK_BOX (vboxcoord), xc_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcoord), yc_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcoord), zc_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxleft), maxx_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxmiddle), maxy_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxright), maxz_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxleft), time_entry, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxmiddle), coord_entry, FALSE, FALSE, 0);

	/* Create buttons and connect them to their procedures. */
	/* Create reset orientation button */
	reseto_button = gtk_button_new_with_mnemonic("Reset _Orientation");
	gtk_box_pack_start(GTK_BOX (vboxright), reseto_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (reseto_button), "clicked", G_CALLBACK (resetob),
			(gpointer) params);

	/* Create restart button. */
	restart_button = gtk_button_new_with_mnemonic("_Restart");
	gtk_box_pack_start(GTK_BOX (hboxsetup), restart_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (restart_button), "clicked", G_CALLBACK (restart),
			(gpointer) params);

	setup_button = gtk_button_new_with_mnemonic("_Setup");
	;
	gtk_box_pack_start(GTK_BOX (hboxsetup), setup_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (setup_button), "clicked", G_CALLBACK (setup),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (vboxleft), hboxsetup, TRUE, TRUE, 0);

	/* Create pause button. */
	pause_button = gtk_toggle_button_new_with_mnemonic("_Pause");
	gtk_box_pack_start(GTK_BOX (vboxmiddle), pause_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (pause_button), "clicked", G_CALLBACK (pauseb),
			(gpointer) params);

	/* Create quit button. */
	quit_button = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_box_pack_start(GTK_BOX (vboxright), quit_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (quit_button), "clicked", G_CALLBACK (quit),
			window);

	xminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxx), xminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xminus10_button), "clicked",
			G_CALLBACK (xminus10b), (gpointer) params);
	xminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxx), xminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xminus_button), "clicked", G_CALLBACK (xminusb),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hboxx), xlabel, TRUE, TRUE, 0);
	xplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxx), xplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xplus_button), "clicked", G_CALLBACK (xplusb),
			(gpointer) params);
	xplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxx), xplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xplus10_button), "clicked",
			G_CALLBACK (xplus10b), (gpointer) params);

	yminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxy), yminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yminus10_button), "clicked",
			G_CALLBACK (yminus10b), (gpointer) params);
	yminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxy), yminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yminus_button), "clicked", G_CALLBACK (yminusb),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hboxy), ylabel, TRUE, TRUE, 0);
	yplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxy), yplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yplus_button), "clicked", G_CALLBACK (yplusb),
			(gpointer) params);
	yplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxy), yplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yplus10_button), "clicked",
			G_CALLBACK (yplus10b), (gpointer) params);

	zminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxz), zminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zminus10_button), "clicked",
			G_CALLBACK (zminus10b), (gpointer) params);
	zminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxz), zminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zminus_button), "clicked", G_CALLBACK (zminusb),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hboxz), zlabel, TRUE, TRUE, 0);
	zplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxz), zplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zplus_button), "clicked", G_CALLBACK (zplusb),
			(gpointer) params);
	zplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxz), zplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zplus10_button), "clicked",
			G_CALLBACK (zplus10b), (gpointer) params);

	g_signal_connect(G_OBJECT (window), "destroy", G_CALLBACK (quit), window);

	/* Put the hbox into the vbox. */
	gtk_box_pack_start(GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), vboxleft, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), vboxmiddle, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), vboxright, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), vboxrb, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox), vboxcoord, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxrb), hboxx, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxrb), hboxy, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vboxrb), hboxz, FALSE, FALSE, 0);

	/* Open the input file, if it fails exit. */
	if (params->file[0] == '_')
		params->fp = stdin;
	else {
		params->fp = NULL;
		params->fp = fopen(params->file, "r");
		if (params->fp == NULL) {
			printf("Error opening file: %s\n", params->file);
			gtk_main_quit();
		}
		fseek(params->fp, 0, 0);
	}

	/* Allocate colors */
	setColorset(params, params->colorset);

	/* Show all boxes,entries,buttons and pixmaps. */
	gtk_widget_show_all(window);

#if Debug 
	printf("Initialising filereading/framedrawing semaphores.\n");
#endif

	for (i = 0; i < NUMFRAMES; i++) {
		params->framedata[i].frameready = g_mutex_new();
		g_mutex_lock(params->framedata[i].frameready);
		params->framedata[i].framedrawn = g_mutex_new();
		g_mutex_unlock(params->framedata[i].framedrawn);
		params->framedata[i].atomdata = NULL;
	}
	DrawData.NumFrame = 0;
	DrawData.currentFrame = &(params->framedata[0]);

#if Debug 
	printf("Initialising filewait/EOF semaphores.\n");
#endif

	params->filewait = g_mutex_new();
	g_mutex_lock(params->filewait);
	params->atEnd = g_mutex_new();

#if Debug 
	printf("Starting filereading thread.\n");
#endif

	th_a =
			g_thread_create ((GThreadFunc) readinput, (gpointer) params, TRUE, NULL);
	if (th_a == NULL) {
		fprintf(stderr, "Creating read thread failed.\n");
		gtk_main_quit();
	}

#if Debug
	printf("Finished initialising threads.\n");
#endif

	/* Setup timeout. */
	g_idle_add((GSourceFunc) drawnext, params);
}

/************************************************************************/
/* The main function sets up default parameters and calls either	*/
/* handleargs or setupwindow depending on the number of commandline	*/
/* parameters.								*/
/************************************************************************/
int main(int argc, char **argv) {
	struct GlobalParams params;

#if Debug 
	printf("Starting program.\n");
#endif

	/* Control variables for program and default settings, do NOT change */

	params.iangle = 0.0;
	params.jangle = 0.0;
	params.kangle = 0.0;
	params.imangle = 0.0;
	params.jmangle = 0.0;
	params.numframe = 1;
	params.pausecheck = FALSE;
	params.drawcheck = TRUE;
	params.rotated = FALSE;
	params.setupstop = FALSE;
	params.pressed = FALSE;
	params.StartedAlready = FALSE;
	params.redrawcheck = FALSE;
	params.usetypes = FALSE;

	DrawData.NumFrame = 0;
	DrawData.params = &params;

#if Debug 
	printf("Fetching display variable.\n");
#endif

#if Debug 
	printf("Initializing GTK.\n");
#endif

	/* Start gtk initialization. */
	gtk_init(&argc, &argv);

	g_thread_init(NULL);

	printf("\n gdpc version "GDPCVER", Copyright (C) 2000 Jonas Frantz\n");
	printf(" gdpc comes with ABSOLUTELY NO WARRANTY; for details\n");
	printf(" check out the documentation.  This is free software, and\n");
	printf(" you are welcome to redistribute it under all conditions.\n\n");

	/* Set all parameters to their default values, dont change this unless */
	/* you know what you're doing. */

	params.absxsize = drawXsize;
	params.absysize = drawYsize;

	params.xmin = 65535.0;
	params.ymin = 65535.0;
	params.zmin = 65535.0;
	params.xmax = 0.0;
	params.ymax = 0.0;
	params.zmax = 0.0;

	params.vary = VARY;
	params.scol = SCOL;
	params.sort = SORT;
	params.radius = RADIUS;
	params.mode = MODE;
	params.colorset = COLORSET;
	params.dumpnum = DUMPNUM;
	params.whitebg = WHITEBG;
	params.erase = ERASE;
	params.fxyz = FXYZ;
	params.drawcheck = TRUE;
	params.mbsleep = FALSE;
	params.once = FALSE;

	params.xcolumn = XCOLUMN;
	params.ycolumn = YCOLUMN;
	params.zcolumn = ZCOLUMN;
	params.tcolumn = TCOLUMN;

	params.interval = 0;

	params.dumpname[0] = '\0';

	strcpy(params.timedelim, TIMESTRING);

	/* Handle arguments passed to the program. */
	if (argc == 1) {
#if Debug 
		printf("Starting setup window.\n");
#endif
		setupwindow(&params);
	} else {
#if Debug 
		printf("Start processing commandline arguments.\n");
#endif
		if (!handleargs(argc, argv, &params))
			exit(-1);
		StartEverything(&params);
	}

#if Debug 
	printf("Starting gtkmain().\n");
#endif

	/* Start gtk. */

	gtk_main();

	exit(0);
}
