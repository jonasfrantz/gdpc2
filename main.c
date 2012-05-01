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

extern gint NumFrameRI; /* Contains which frame is about to be
 read from the input data */
extern FILE *NewFP; /* Filepointer to new file if one has
 been chosen, otherwise = NULL */

/************************************************************************/
/* This function is called when the quit button is pressed.		*/
/************************************************************************/
void quit(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

/************************************************************************/
/* This function is called when the pause button is pressed.		*/
/************************************************************************/
void pauseButtonPressed(GtkToggleButton *widget, struct Context *context) {
#if Debug 
	if (gtk_toggle_button_get_active(widget))
	printf("Setting animation on pause.\n");
	else printf("Unpausing animation.\n");
#endif

	context->pausecheck = gtk_toggle_button_get_active(widget);
}

/************************************************************************/
/* This function is called when the restart button is pressed.   	*/
/* It simply resets the filepointer and clears the drawingboard. 	*/
/************************************************************************/
void restartAnimation(GtkWidget *widget, struct Context *context) {
	gint i;

#if Debug
	printf("Restarting animation.\n");
	printf("Clearing framedrawn semaphores.\n");
#endif

	for (i = NUMFRAMES - 1; i >= 0; i--) {
		g_mutex_trylock(context->framedata[i].framecomplete);
		g_mutex_lock(context->framedata[i].framedrawn);
		context->framedata[i].lastFrame = FALSE;
	}

#if Debug
	printf("Opening new file.\n");
#endif

	NewFP = fopen(context->config->file, "r");
	if (NewFP == NULL) {
		printf("Error opening file: %s\n", context->config->file);
		gtk_main_quit();
	}
	fseek(NewFP, SEEK_SET, 0);
	g_mutex_unlock(context->atEnd);
//	context->config->numframe = 1;

#if Debug
	printf("Reinitialize filewait/frameready/framedrawn semaphores.\n");
#endif

	g_mutex_unlock(context->filewait);

	for (i = NUMFRAMES - 1; i >= 0; i--) {
		g_mutex_trylock(context->framedata[i].frameready);
	}

	context->nextFrameNum = NumFrameRI;

	for (i = 0; i < NUMFRAMES; i++) {
		g_mutex_unlock(context->framedata[i].framecomplete);
		g_mutex_unlock(context->framedata[i].framedrawn);
	}

	context->currentFrame = NULL;
}

/************************************************************************/
/* This function is called when the setup button is pressed.		*/
/* It stops the animation and calls the setupwindow function.		*/
/************************************************************************/
void setupButtonPressed(GtkWidget *widget, struct Context *context) {
	context->setupstop = TRUE;
	showSetupWindow(context);
}

/************************************************************************/
/************************************************************************/
gboolean updateImageArea(GtkWidget *widget, cairo_t *cr,
		struct Context *context) {
	guint width, height;
	cairo_t *first_cr;
	cairo_surface_t *first;
	char tstr[64];

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	if (context->currentFrame != NULL) {
		if (g_mutex_trylock((context->currentFrame)->framedrawn) == TRUE) {
			first = cairo_surface_create_similar(cairo_get_target(cr),
					CAIRO_CONTENT_COLOR, width, height);

			first_cr = cairo_create(first);

			context->crXSize = width;
			context->crYSize = height;

			drawFrame(context, first_cr);

			cairo_set_source_surface(cr, first, 0, 0);
			cairo_paint(cr);

			cairo_surface_destroy(first);

			cairo_destroy(first_cr);

			sprintf(tstr, "X: %4.3f - %4.3f",
					(context->currentFrame)->xmin,
					(context->currentFrame)->xmax);
			gtk_entry_set_text((GtkEntry *) maxx_entry, tstr);
			sprintf(tstr, "Y: %4.3f - %4.3f",
					(context->currentFrame)->ymin,
					(context->currentFrame)->ymax);
			gtk_entry_set_text((GtkEntry *) maxy_entry, tstr);
			sprintf(tstr, "Z: %4.3f - %4.3f",
					(context->currentFrame)->zmin,
					(context->currentFrame)->zmax);
			gtk_entry_set_text((GtkEntry *) maxz_entry, tstr);

			sprintf(tstr, "Time: %5.3f fs", (context->currentFrame)->atime);
			gtk_entry_set_text((GtkEntry *) time_entry, tstr);

			g_mutex_unlock((context->currentFrame)->framedrawn);
		}
	}

	return TRUE;
}

/************************************************************************/
/* This procedure is called when keyboard key is pressed.		*/
/* Escape key quits gdpc, space is used with the bsleep option.		*/
/************************************************************************/
gboolean keyPressEvent(GtkWidget *widget, GdkEventKey *event, gpointer data) {
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
gint buttonPressEvent(GtkWidget *widget, GdkEventButton *event,
		struct Context *params) {
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
gint buttonReleaseEvent(GtkWidget *widget, GdkEventButton *event,
		struct Context *params) {
#if Debug
	printf("Button released.\n");
#endif

	if (event->button == 1) {
		params->pressed = FALSE;
		mouseRotate(widget, (params->xpress - event->x),
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
gint motionNotifyEvent(GtkWidget *widget, GdkEventMotion *event,
		struct Context *context) {
	gint x, y;
	char xstr[64];
	GdkModifierType state;
	gint NumFrame;

#if Debug
	printf("Fetching coordinates of pointer.\n");
#endif

	gdk_window_get_pointer(event->window, &x, &y, &state);

	if (context->pressed) {
		if (context->xpress - x > ROTATETRESHOLD
				|| context->ypress - y > ROTATETRESHOLD
				|| context->xpress - x < -ROTATETRESHOLD
				|| context->ypress - y < -ROTATETRESHOLD) {
#if Debug
			printf("Starting rotating of scene.\n");
#endif
			mouseRotate(widget, (context->xpress - x), (context->ypress - y),
					context);
			context->xpress = x;
			context->ypress = y;
		}
	}

	NumFrame = context->nextFrameNum;
	NumFrame--;
	if (NumFrame < 0)
		NumFrame += NUMFRAMES;

#if Debug
	printf("Fetching and setting coordinates of pointer at scene.\n");
#endif
	sprintf(
			xstr,
			"X: %5.3f   Y: %5.3f",
			(((context->framedata[NumFrame].xmax
					- context->framedata[NumFrame].xmin) * (x - xborder)
					/ (double) context->config->absxsize)
					+ context->framedata[NumFrame].xmin),
			(((context->framedata[NumFrame].ymax
					- context->framedata[NumFrame].ymin)
					* (context->config->absysize - (y - yborder))
					/ (double) context->config->absysize))
					+ context->framedata[NumFrame].ymin);
	gtk_entry_set_text((GtkEntry *) coord_entry, xstr);

	return TRUE;
}

/************************************************************************/
/************************************************************************/
void setContextConfig(struct Context *context, struct Configuration *newconfig) {
	struct Configuration *oldconfig;

	setColorset(newconfig);

	oldconfig = context->config;
	context->config = newconfig;
	if (oldconfig != NULL) {
		free(oldconfig);
	}
}


/************************************************************************/
/* This function is called at the end of setupwindow if the ok button 	*/
/* was pressed, it reinitializes gdpc if necessary and then continous	*/
/* the animation.							*/
/************************************************************************/
void setupStartOk(struct Context *context, struct Configuration *newconfig) {

	if (context->config->absxsize != newconfig->absxsize
			|| context->config->absysize != newconfig->absysize) {
		gtk_widget_set_size_request(context->drawing_area,
				newconfig->absxsize + 2 * xborder, newconfig->absysize + 2 * yborder);
	}

	if (context->config->xcolumn != newconfig->xcolumn || context->config->ycolumn != newconfig->ycolumn
			|| context->config->zcolumn != newconfig->zcolumn
			|| context->config->tcolumn != newconfig->tcolumn) {
		fseek(context->fp, SEEK_SET, 0);
		g_mutex_unlock(context->atEnd);
	}
	if (strlen(newconfig->file) > 0) {
		fclose(context->fp);
		context->fp = fopen(newconfig->file, "r");
		if (context->fp == NULL) {
			printf("Error opening file: %s\n", newconfig->file);
			gtk_main_quit();
		}
		fseek(context->fp, 0, 0);
		g_mutex_unlock(context->atEnd);
	}
	setContextConfig(context, newconfig);

	context->setupstop = FALSE;
}

/************************************************************************/
/* This function is called at the end of setwindow if the cancel button */
/* was pressed. It simply continous the animation.						*/
/************************************************************************/
void setupStartCancel(struct Context *context) {
	context->setupstop = FALSE;
}

/************************************************************************/
/* This function is called when the apply button is pressed in the 		*/
/* setup window. It just redraws the frame.								*/
/************************************************************************/
void setupApplyNewConfig(struct Context *context, struct Configuration *newconfig) {

	if (context->config->absxsize != newconfig->absxsize
			|| context->config->absysize != newconfig->absysize) {
		gtk_widget_set_size_request(context->drawing_area,
				newconfig->absxsize + 2 * xborder, newconfig->absysize + 2 * yborder);
	}

	setContextConfig(context, newconfig);

	triggerImageRedraw(NULL, context);
}

/************************************************************************/
/* This function is called when there has been a change in angle from	*/
/* pressing a button.													*/
/************************************************************************/
void triggerImageRedraw(GtkWidget *widget, struct Context *params) {
	gtk_widget_queue_draw(params->drawing_area);
}

/************************************************************************/
/* This function is the callback for the timeout instruction.		*/
/* It checks if a frame is being drawed or pause is pressed or if the	*/
/* animation is at the end, else it calls drawatoms which starts drawing*/
/* the next frame. When drawatoms is done, if updates the time in the	*/
/* timeentry and puts the pixmap onto the screen.			*/
/************************************************************************/
gboolean switchToNextFrame(struct Context *context) {
	char tstr[64];
	char picname[128];
	char pictype[16];
	struct timeval tv;
	struct timezone tz;
	static long previous_usec = 0;
	static long previous_sec = 0;
	gboolean previousDrawn;
	struct Frame *previousFrame;

	gettimeofday(&tv, &tz);

	if ((((double) tv.tv_sec - previous_sec) * 1000
			+ ((double) tv.tv_usec - previous_usec) / 1000)
			> ((double) context->config->interval)) {
		previous_usec = tv.tv_usec;
		previous_sec = tv.tv_sec;

		if (!context->pausecheck && !context->setupstop
				&& (!context->config->mbsleep || MB_pressed)) {
			MB_pressed = FALSE;

			if (context->config->once) {
				if (context->currentFrame != NULL) {
					if ((context->currentFrame)->lastFrame) {
						gtk_main_quit();
					}
				}
			}

			previousDrawn = FALSE;
			if (context->currentFrame != NULL) {
				if (!(context->currentFrame)->lastFrame) {
					if (g_mutex_trylock((context->currentFrame)->framedrawn)
							== TRUE) {
						previousDrawn = TRUE;
					}
				}
			} else {
				previousDrawn = TRUE;

			}

			if (g_mutex_trylock(context->framedata[context->nextFrameNum].frameready)
					== TRUE && previousDrawn) {

				previousFrame = context->currentFrame;
				context->currentFrame = &(context->framedata[context->nextFrameNum]);

				gtk_widget_queue_draw(context->drawing_area);

				if (previousFrame != NULL) {
					g_mutex_unlock(previousFrame->framedrawn);
					g_mutex_unlock(previousFrame->framecomplete);
				}

				context->nextFrameNum++;
				if (context->nextFrameNum == NUMFRAMES) {
					context->nextFrameNum = 0;
				}

#if Debug
				printf("%s\n",context->dumpname);
#endif
				if (context->config->dumpname[0] != '\0') {
#if Debug
					printf("Creating image of frame to dump.\n");
#endif

					if (context->config->tifjpg) {
#if Debug
						printf("Dumping png.\n");
#endif
						sprintf(pictype, "png");
						if (context->config->dumpnum)
							sprintf(picname, "%s-%d.png", context->config->dumpname,
									context->currentFrame->numframe);
						else
							sprintf(picname, "%s-%5.3f.png", context->config->dumpname,
									context->currentFrame->atime);
					} else {
#if Debug
						printf("Dumping jpg.\n");
#endif
						sprintf(pictype, "jpeg");
						if (context->config->dumpnum)
							sprintf(picname, "%s-%d.jpg", context->config->dumpname,
									context->currentFrame->numframe);
						else
							sprintf(picname, "%s-%5.3f.jpg", context->config->dumpname,
									context->currentFrame->atime);
					}
				}
				context->currentFrame->numframe++;
			}
		}
	}

#if Debug
	printf("Done with drawing part of timeoutcallback.\n");
#endif


#if Debug
	printf("Setting angle entryboxes.\n");
#endif

	sprintf(tstr, "X angle: %f", context->config->xc);
	gtk_entry_set_text((GtkEntry *) xc_entry, tstr);
	sprintf(tstr, "Y angle: %f", context->config->yc);
	gtk_entry_set_text((GtkEntry *) yc_entry, tstr);
	sprintf(tstr, "Z angle: %f", context->config->zc);
	gtk_entry_set_text((GtkEntry *) zc_entry, tstr);

	return TRUE;
}

/************************************************************************/
/************************************************************************/
GtkWidget * getMainWindow(struct Context *context) {
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

	/* Set the window title. */
	sprintf(buf, "%s - " PROGNAME " "GDPCVER, context->config->file);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), buf);
	gtk_window_set_resizable(GTK_WINDOW (window), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER (window), 5);

	g_signal_connect_swapped(G_OBJECT (window), "key_press_event",
			G_CALLBACK (keyPressEvent), NULL);

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
	gtk_widget_set_size_request(drawing_area, context->config->absxsize + 2 * xborder,
			context->config->absysize + 2 * yborder);

	context->drawing_area = drawing_area;

	gtk_box_pack_start(GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);

	/* Connect the events to their procedures. */
	g_signal_connect(G_OBJECT (drawing_area), "draw",
			G_CALLBACK (updateImageArea), context);

	g_signal_connect(G_OBJECT (drawing_area), "button_press_event",
			G_CALLBACK (buttonPressEvent), context);
	g_signal_connect(G_OBJECT (drawing_area), "button_release_event",
			G_CALLBACK (buttonReleaseEvent), context);

	g_signal_connect(G_OBJECT (drawing_area), "motion_notify_event",
			G_CALLBACK (motionNotifyEvent), context);

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
	g_signal_connect(G_OBJECT (reseto_button), "clicked", G_CALLBACK (resetOrientationButtonPressed),
			(gpointer) context);

	/* Create restartAnimation button. */
	restart_button = gtk_button_new_with_mnemonic("_Restart");
	gtk_box_pack_start(GTK_BOX (hboxsetup), restart_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (restart_button), "clicked", G_CALLBACK (restartAnimation),
			(gpointer) context);

	setup_button = gtk_button_new_with_mnemonic("_Setup");
	;
	gtk_box_pack_start(GTK_BOX (hboxsetup), setup_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (setup_button), "clicked", G_CALLBACK (setupButtonPressed),
			(gpointer) context);
	gtk_box_pack_start(GTK_BOX (vboxleft), hboxsetup, TRUE, TRUE, 0);

	/* Create pause button. */
	pause_button = gtk_toggle_button_new_with_mnemonic("_Pause");
	gtk_box_pack_start(GTK_BOX (vboxmiddle), pause_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (pause_button), "clicked", G_CALLBACK (pauseButtonPressed),
			(gpointer) context);

	/* Create quit button. */
	quit_button = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_box_pack_start(GTK_BOX (vboxright), quit_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (quit_button), "clicked", G_CALLBACK (quit),
			window);

	xminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxx), xminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xminus10_button), "clicked",
			G_CALLBACK (xminus10b), (gpointer) context);
	xminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxx), xminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xminus_button), "clicked", G_CALLBACK (xminusb),
			(gpointer) context);
	gtk_box_pack_start(GTK_BOX (hboxx), xlabel, TRUE, TRUE, 0);
	xplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxx), xplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xplus_button), "clicked", G_CALLBACK (xplusb),
			(gpointer) context);
	xplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxx), xplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (xplus10_button), "clicked",
			G_CALLBACK (xplus10b), (gpointer) context);

	yminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxy), yminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yminus10_button), "clicked",
			G_CALLBACK (yminus10b), (gpointer) context);
	yminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxy), yminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yminus_button), "clicked", G_CALLBACK (yminusb),
			(gpointer) context);
	gtk_box_pack_start(GTK_BOX (hboxy), ylabel, TRUE, TRUE, 0);
	yplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxy), yplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yplus_button), "clicked", G_CALLBACK (yplusb),
			(gpointer) context);
	yplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxy), yplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (yplus10_button), "clicked",
			G_CALLBACK (yplus10b), (gpointer) context);

	zminus10_button = gtk_button_new_with_label("<<");
	gtk_box_pack_start(GTK_BOX (hboxz), zminus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zminus10_button), "clicked",
			G_CALLBACK (zminus10b), (gpointer) context);
	zminus_button = gtk_button_new_with_label("<");
	gtk_box_pack_start(GTK_BOX (hboxz), zminus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zminus_button), "clicked", G_CALLBACK (zminusb),
			(gpointer) context);
	gtk_box_pack_start(GTK_BOX (hboxz), zlabel, TRUE, TRUE, 0);
	zplus_button = gtk_button_new_with_label(">");
	gtk_box_pack_start(GTK_BOX (hboxz), zplus_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zplus_button), "clicked", G_CALLBACK (zplusb),
			(gpointer) context);
	zplus10_button = gtk_button_new_with_label(">>");
	gtk_box_pack_start(GTK_BOX (hboxz), zplus10_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (zplus10_button), "clicked",
			G_CALLBACK (zplus10b), (gpointer) context);

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

	return window;
}

/************************************************************************/
/* The StartEverything function is called by main() after the 		*/
/* commandline arguments or the setupwindow has finished processing of 	*/
/* the parameters. This function sets up all the buttons, entries, 	*/
/* boxes,the timeout and the drawingboard.				*/
/************************************************************************/
void StartEverything(struct Context *context) {
	GtkWidget *window;
	gint i;

	window = getMainWindow(context);

	context->StartedAlready = TRUE;

	/* Open the input file, if it fails exit. */
	if (context->config->file[0] == '_')
		context->fp = stdin;
	else {
		context->fp = NULL;
		context->fp = fopen(context->config->file, "r");
		if (context->fp == NULL) {
			printf("Error opening file: %s\n", context->config->file);
			gtk_main_quit();
		}
		fseek(context->fp, 0, 0);
	}

	/* Allocate colors */
	setColorset(context->config);

	/* Show all boxes,entries,buttons and pixmaps. */
	gtk_widget_show_all(window);

	for (i = 0; i < NUMFRAMES; i++) {
		context->framedata[i].frameready = g_mutex_new();
		g_mutex_lock(context->framedata[i].frameready);
		context->framedata[i].framecomplete = g_mutex_new();
		g_mutex_unlock(context->framedata[i].framecomplete);
		context->framedata[i].framedrawn = g_mutex_new();
		g_mutex_unlock(context->framedata[i].framedrawn);
		context->framedata[i].atomdata = NULL;
	}
	context->nextFrameNum = 0;
	context->currentFrame = NULL;

	context->filewait = g_mutex_new();
	g_mutex_lock(context->filewait);
	context->atEnd = g_mutex_new();

	th_a =
			g_thread_create ((GThreadFunc) readInput, (gpointer) context, TRUE, NULL);
	if (th_a == NULL) {
		fprintf(stderr, "Creating read thread failed.\n");
		gtk_main_quit();
	}

	/* Setup timeout. */
	g_idle_add((GSourceFunc) switchToNextFrame, context);
}

/************************************************************************/
/************************************************************************/
struct Configuration * copyConfiguration(struct Configuration *oldconfig) {
	struct Configuration *newconfig;

	newconfig = malloc(sizeof(struct Configuration));
	memcpy(newconfig, oldconfig, sizeof(struct Configuration));
	return newconfig;
}

/************************************************************************/
/* Set all parameters to their default values, dont change this unless 	*/
/* you know what you're doing. 											*/
/************************************************************************/
struct Configuration * getNewConfiguration() {
	struct Configuration *config;

	config = malloc(sizeof(struct Configuration));

	if (config != NULL) {
		config->absxsize = DEFAULT_DRAWING_AREA_X_SIZE;
		config->absysize = DEFAULT_DRAWING_AREA_Y_SIZE;

		config->xmin = 65535.0;
		config->ymin = 65535.0;
		config->zmin = 65535.0;
		config->xmax = 0.0;
		config->ymax = 0.0;
		config->zmax = 0.0;

		config->vary = DEFAULT_VARY;
		config->scol = DEFAULT_SCOL;
		config->sort = DEFAULT_SORT;
		config->radius = DEFAULT_ATOM_RADIUS;
		config->mode = DEFAULT_MODE;
		config->colorset = DEFAULT_COLORSET;
		config->dumpnum = DEFAULT_DUMPNUM;
		config->whitebg = DEFAULT_WHITEBG;
		config->erase = DEFAULT_ERASE;
		config->fxyz = DEFAULT_FXYZ;
		config->mbsleep = FALSE;
		config->once = FALSE;
		config->usetypes = FALSE;

		config->xcolumn = DEFAULT_XCOLUMN;
		config->ycolumn = DEFAULT_YCOLUMN;
		config->zcolumn = DEFAULT_ZCOLUMN;
		config->tcolumn = DEFAULT_TCOLUMN;

		config->interval = DEFAULT_INTERVAL;

		config->dumpname[0] = DEFAULT_DUMPNAME;

		strcpy(config->timedelim, TIMESTRING);
	}

	return config;
}

/************************************************************************/
/************************************************************************/
struct Context * getNewContext() {
	struct Context *context;

	context = malloc(sizeof(struct Context));

	if (context != NULL) {
		context->iangle = 0.0;
		context->jangle = 0.0;
		context->kangle = 0.0;
		context->imangle = 0.0;
		context->jmangle = 0.0;
		context->pausecheck = FALSE;
		context->setupstop = FALSE;
		context->pressed = FALSE;
		context->StartedAlready = FALSE;
		context->nextFrameNum = 0;
		context->config = NULL;
	}
	return context;
}

/************************************************************************/
/* The main function sets up default parameters and calls either		*/
/* handleargs or setupwindow depending on the number of commandline		*/
/* parameters.															*/
/************************************************************************/
int main(int argc, char **argv) {
	struct Context *context;
	struct Configuration *config;

	/* Start gtk initialization. */
	gtk_init(&argc, &argv);

	g_thread_init(NULL);

	printf(DISCLAIMER);

	context = getNewContext();

	/* Handle arguments passed to the program. */
	if (argc == 1) {
		config = getNewConfiguration();
		if (config != NULL) {
			setContextConfig(context, config);
		}
		showSetupWindow(context);
	} else {
		if ((config = handleArgs(argc, argv)) == NULL) {
			exit(-1);
		}
		setContextConfig(context, config);
		StartEverything(context);
	}

	/* Start gtk. */

	gtk_main();

	exit(0);
}
