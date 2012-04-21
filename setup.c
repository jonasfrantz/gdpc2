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
#include "tooltips.h"

/* GTK widgets which define changes in setup window */

/* Different windows and okey-button */
GtkWidget *filew, *setupwin, *okeyb;

/* Entries, check-button, spin-buttons and so on */
GtkWidget *file_entry, *dump_entry, *dump_label, *dtcheck, *dncheck;
GtkWidget *scol_entry, *scol_entry, *scol_label, *cubespinner, *xspinner;
GtkWidget *yspinner, *zspinner, *x2spinner, *y2spinner, *z2spinner;
GtkWidget *ssxspinner, *ssyspinner, *xcspinner, *ycspinner, *zcspinner;
GtkWidget *tcspinner, *sleepspinner, *dspinner, *scolspinner;
GtkWidget *usetypescheck, *timedel_entry, *timedel_label;

/* Adjustments for the spin-buttons */
GtkAdjustment *adjcube, *adjx, *adjy, *adjz, *adjxc, *adjyc, *adjzc, *adjtc;
GtkAdjustment *adjssizex, *adjssizey, *adjsleep, *adjd;
GtkAdjustment *adjx2, *adjy2, *adjz2, *adjscol;

struct GlobalParams newparams; /* New values if OK is pressed, else ignored */

gboolean usescol, usedump, filewopen; /* Help variables to define usage of
 specific features */

/************************************************************************/
/* This function just kills the setup window. 				*/
/************************************************************************/
void destroy(GtkWidget *widget, gpointer data) {
	if (filewopen)
		gtk_widget_destroy(filew);
	gtk_widget_destroy(setupwin);
}

/************************************************************************/
/* This function creates a filebrowser window, connects all the buttons	*/
/* and shows it.							*/
/************************************************************************/
void filebrowser(GtkWidget *widget, gpointer data) {
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW (setupwin),
			GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
		gtk_entry_set_text(GTK_ENTRY (file_entry), filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

/************************************************************************/
/* This function is called when the OK button is pressed, it gets all	*/
/* the filenames and sets all the variables according to the buttons 	*/
/* and spinners.							*/
/************************************************************************/
void okeypressed(GtkWidget *widget, struct GlobalParams *params) {
	double tmpvalue;
	sprintf(params->file, "%s", gtk_entry_get_text(GTK_ENTRY (file_entry)));
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (xspinner));

	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->xmin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (x2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->xmax = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (yspinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->ymin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (y2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->ymax = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (zspinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->zmin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (z2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->zmax = tmpvalue;
	}

	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (cubespinner));
	if (tmpvalue > 0.0) {
		if (params->xmin == 65535.0) {
			params->xmin = -tmpvalue;
			params->xmax = tmpvalue;
		}
		if (params->ymin == 65535.0) {
			params->ymin = -tmpvalue;
			params->ymax = tmpvalue;
		}
		if (params->zmin == 65535.0) {
			params->zmin = -tmpvalue;
			params->zmax = tmpvalue;
		}
	}

	params->radius = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (dspinner));
	params->xcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (xcspinner));
	params->ycolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ycspinner));
	params->zcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (zcspinner));
	params->tcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (tcspinner));
	params->absxsize = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ssxspinner));
	params->absysize = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ssyspinner));
	tmpvalue = (gint) 1000
			* gtk_spin_button_get_value(GTK_SPIN_BUTTON (sleepspinner));
	if (tmpvalue < MININTERVAL)
		params->interval = MININTERVAL;
	else
		params->interval = tmpvalue;

	if (usescol) {
		sprintf(params->fstring, "%s",
				gtk_entry_get_text(GTK_ENTRY (scol_entry)));
		params->scol = gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON (scolspinner));
	} else {
		params->scol = 0;
	}

	if (usedump) {
		sprintf(params->dumpname, "%s",
				gtk_entry_get_text(GTK_ENTRY (dump_entry)));
	} else {
		*(params->dumpname) = '\0';
	}

	sprintf(params->timedelim, "%s",
			gtk_entry_get_text(GTK_ENTRY (timedel_entry)));

	params->fxyz = newparams.fxyz;
	params->mode = newparams.mode;
	params->vary = newparams.vary;
	params->colorset = newparams.colorset;
	params->erase = newparams.erase;
	params->whitebg = newparams.whitebg;
	params->dumpnum = newparams.dumpnum;
	params->sort = newparams.sort;
	params->tifjpg = newparams.tifjpg;
	params->usetypes = newparams.usetypes;

	if (!params->StartedAlready)
		StartEverything(params);
	else
		SetupStartOk(params);
	destroy(widget, NULL);
}

/************************************************************************/
/* This function is called when the redraw button is pressed, it	*/
/* practically does the same thing as okeypressed, but it doesn't kill	*/
/* the setup window.							*/
/************************************************************************/
void redrawpressed(GtkWidget *widget, struct GlobalParams *params) {
	double tmpvalue;
	sprintf(params->file, "%s", gtk_entry_get_text(GTK_ENTRY (file_entry)));
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (xspinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->xmin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (x2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->xmax = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (yspinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->ymin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (y2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->ymax = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (zspinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->zmin = tmpvalue;
	}
	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (z2spinner));
	if (tmpvalue > 0.0 || tmpvalue < 0.0) {
		params->zmax = tmpvalue;
	}

	tmpvalue = gtk_spin_button_get_value(GTK_SPIN_BUTTON (cubespinner));
	if (tmpvalue > 0.0) {
		if (params->xmin == 65535.0) {
			params->xmin = -tmpvalue;
			params->xmax = tmpvalue;
		}
		if (params->ymin == 65535.0) {
			params->ymin = -tmpvalue;
			params->ymax = tmpvalue;
		}
		if (params->zmin == 65535.0) {
			params->zmin = -tmpvalue;
			params->zmax = tmpvalue;
		}
	}

	params->radius = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (dspinner));
	params->xcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (xcspinner));
	params->ycolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ycspinner));
	params->zcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (zcspinner));
	params->tcolumn = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (tcspinner));
	params->absxsize = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ssxspinner));
	params->absysize = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON (ssyspinner));
	tmpvalue = (gint) 1000
			* gtk_spin_button_get_value(GTK_SPIN_BUTTON (sleepspinner));
	if (tmpvalue < MININTERVAL)
		params->interval = MININTERVAL;
	else
		params->interval = tmpvalue;

	if (usescol) {
		sprintf(params->fstring, "%s",
				gtk_entry_get_text(GTK_ENTRY (scol_entry)));
		params->scol = gtk_spin_button_get_value_as_int(
				GTK_SPIN_BUTTON (scolspinner));
	} else {
		params->scol = 0;
	}

	if (usedump) {
		sprintf(params->dumpname, "%s",
				gtk_entry_get_text(GTK_ENTRY (dump_entry)));
	} else {
		*(params->dumpname) = '\0';
	}

	params->fxyz = newparams.fxyz;
	params->mode = newparams.mode;
	params->vary = newparams.vary;
	params->colorset = newparams.colorset;
	params->erase = newparams.erase;
	params->whitebg = newparams.whitebg;
	params->dumpnum = newparams.dumpnum;
	params->sort = newparams.sort;

	SetupRedraw(params);
}

/************************************************************************/
/* This function is called when the Cancel button is pressed, it 	*/
/* discards all the buttonpresses and kills the setup window.		*/
/************************************************************************/
void cancelpressed(GtkWidget *widget, struct GlobalParams *params) {
	SetupStartCancel(params);
	destroy(widget, NULL);
}

/************************************************************************/
/* This function is called when the Quit button is pressed, it quits 	*/
/* the main program.							*/
/************************************************************************/
void quitpressed(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

/************************************************************************/
/* This function is called whenever a entry is changed, it checks if	*/
/* the file entered exists, if the stringsearch is used and if so that 	*/
/* the string is longer than zero and it checks if the dumpfile option 	*/
/* is used and its name is longer than zero. If the conditions are met 	*/
/* it enables the OK buttons, else it disables it.			*/
/************************************************************************/
void filechange(GtkWidget *widget, gpointer data) {
	FILE *fp;
	if (strlen(gtk_entry_get_text(GTK_ENTRY (file_entry))) > 0
			&& (!usescol
					|| strlen(gtk_entry_get_text(GTK_ENTRY (scol_entry))) > 0)
			&& (!usedump
					|| strlen(gtk_entry_get_text(GTK_ENTRY (dump_entry))) > 0)) {
		fp = fopen(gtk_entry_get_text(GTK_ENTRY (file_entry)), "r");
		if (fp != NULL) {
			gtk_widget_set_sensitive(okeyb, TRUE);
			fclose(fp);
		} else
			gtk_widget_set_sensitive(okeyb, FALSE);
	} else
		gtk_widget_set_sensitive(okeyb, FALSE);
}

/************************************************************************/
/* This function is called when the XYZ file format radiobutton is 	*/
/* pressed.								*/
/************************************************************************/
void toggle_checkxyz(GtkWidget *widget, gpointer data) {
	newparams.fxyz = TRUE;
	gtk_widget_set_sensitive(usetypescheck, TRUE);
	gtk_widget_set_sensitive(timedel_entry, TRUE);
	gtk_widget_set_sensitive(timedel_label, TRUE);
}

/************************************************************************/
/* This function is called when the arbitrary file format radiobutton 	*/
/* is pressed.								*/
/************************************************************************/
void toggle_checkaff(GtkWidget *widget, gpointer data) {
	newparams.fxyz = FALSE;
	gtk_widget_set_sensitive(usetypescheck, FALSE);
	gtk_widget_set_sensitive(timedel_entry, FALSE);
	gtk_widget_set_sensitive(timedel_label, FALSE);
}

/************************************************************************/
/* This function is called when the drawingmode 0 radiobutton is 	*/
/* pressed.								*/
/************************************************************************/
void toggle_checkm0(GtkWidget *widget, gpointer data) {
	newparams.mode = 0;
}

/************************************************************************/
/* This function is called when the drawingmode 1 radiobutton is 	*/
/* pressed.								*/
/************************************************************************/
void toggle_checkm1(GtkWidget *widget, gpointer data) {
	newparams.mode = 1;
}

/************************************************************************/
/* This function is called when the drawingmode 2 radiobutton is 	*/
/* pressed.								*/
/************************************************************************/
void toggle_checkm2(GtkWidget *widget, gpointer data) {
	newparams.mode = 2;
}

/************************************************************************/
/* This function is called when the varymode 0 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkv0(GtkWidget *widget, gpointer data) {
	newparams.vary = 0;
}

/************************************************************************/
/* This function is called when the varymode 1 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkv1(GtkWidget *widget, gpointer data) {
	newparams.vary = 1;
}

/************************************************************************/
/* This function is called when the varymode 2 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkv2(GtkWidget *widget, gpointer data) {
	newparams.vary = 2;
}

/************************************************************************/
/* This function is called when the colorset 0 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkc0(GtkWidget *widget, gpointer data) {
	newparams.colorset = 0;
}

/************************************************************************/
/* This function is called when the colorset 1 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkc1(GtkWidget *widget, gpointer data) {
	newparams.colorset = 1;
}

/************************************************************************/
/* This function is called when the colorset 2 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkc2(GtkWidget *widget, gpointer data) {
	newparams.colorset = 2;
}

/************************************************************************/
/* This function is called when the colorset 3 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkc3(GtkWidget *widget, gpointer data) {
	newparams.colorset = 3;
}

/************************************************************************/
/* This function is called when the colorset 4 radiobutton is pressed.	*/
/************************************************************************/
void toggle_checkc4(GtkWidget *widget, gpointer data) {
	newparams.colorset = 4;
}

/************************************************************************/
/* This function is called when the dump with time radio button is 	*/
/* pressed.								*/
/************************************************************************/
void toggle_checkdt(GtkWidget *widget, gpointer data) {
	newparams.dumpnum = FALSE;
}

/************************************************************************/
/* This function is called when the dump with framenum radio button is	*/
/* pressed. 								*/
/************************************************************************/
void toggle_checkdn(GtkWidget *widget, gpointer data) {
	newparams.dumpnum = TRUE;
}

/************************************************************************/
/* This function is called when the erase checkbutton is pressed. 	*/
/************************************************************************/
void toggle_erase(GtkToggleButton *widget, gpointer data) {
	newparams.erase = gtk_toggle_button_get_active(widget);
}

void toggle_usetypes(GtkToggleButton *widget, gpointer data) {
	newparams.usetypes = gtk_toggle_button_get_active(widget);
}

/************************************************************************/
/* This function is called when the dumpframe checkbutton is pressed. 	*/
/* It also activates the dumpfilename entry and the other related	*/
/* radiobuttons.							*/
/************************************************************************/
void toggle_checkdump(GtkWidget *widget, gpointer data) {
	usedump = FALSE;
	gtk_widget_set_sensitive(dump_entry, FALSE);
	gtk_widget_set_sensitive(dump_label, FALSE);
	gtk_widget_set_sensitive(dncheck, FALSE);
	gtk_widget_set_sensitive(dtcheck, FALSE);
	filechange(widget, data);
}

void toggle_checkdumptif(GtkWidget *widget, gpointer data) {
	usedump = TRUE;
	gtk_widget_set_sensitive(dump_entry, TRUE);
	gtk_widget_set_sensitive(dump_label, TRUE);
	gtk_widget_set_sensitive(dncheck, TRUE);
	gtk_widget_set_sensitive(dtcheck, TRUE);
	newparams.tifjpg = TRUE;
	filechange(widget, data);
}

void toggle_checkdumpjpg(GtkWidget *widget, gpointer data) {
	usedump = TRUE;
	gtk_widget_set_sensitive(dump_entry, TRUE);
	gtk_widget_set_sensitive(dump_label, TRUE);
	gtk_widget_set_sensitive(dncheck, TRUE);
	gtk_widget_set_sensitive(dtcheck, TRUE);
	newparams.tifjpg = FALSE;
	filechange(widget, data);
}

/************************************************************************/
/* This function is called when the whitebg checkbutton is pressed.	*/
/************************************************************************/
void toggle_white(GtkToggleButton *widget, gpointer data) {
	newparams.whitebg = gtk_toggle_button_get_active(widget);
}

/************************************************************************/
/* This function is called when the sortreverse checkbutton is pressed.	*/
/************************************************************************/
void toggle_sortr(GtkToggleButton *widget, gpointer data) {
	if (gtk_toggle_button_get_active(widget)) {
		newparams.sort = 2;
	} else {
		newparams.sort = 1;
	}
}

/************************************************************************/
/* This function is called when the stringsearch checkbutton is 	*/
/* pressed. It also activates the string entry and the other related 	*/
/* radiobuttons.							*/
/************************************************************************/
void toggle_scol(GtkToggleButton *widget, gpointer data) {
	usescol = gtk_toggle_button_get_active(widget);
	gtk_widget_set_sensitive(scol_label, gtk_toggle_button_get_active(widget));
	gtk_widget_set_sensitive(scol_entry, gtk_toggle_button_get_active(widget));
	gtk_widget_set_sensitive(scolspinner, gtk_toggle_button_get_active(widget));
	filechange(widget, data);
}

/************************************************************************/
/* This function is called whenever the x,y or z spinbutton values are	*/
/* changed, it sets up the limits for them.				*/
/************************************************************************/
void setspinlimits(GtkWidget *widget, gpointer data) {
	double value1, value2;

	value1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (xspinner));
	value2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (x2spinner));
	adjx = (GtkAdjustment *) gtk_adjustment_new(value1, -10000.0, value2, 1.0,
			5.0, 0.0);
	adjx2 = (GtkAdjustment *) gtk_adjustment_new(value2, value1, 10000.0, 1.0,
			5.0, 0.0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (xspinner), adjx);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (x2spinner), adjx2);

	value1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (yspinner));
	value2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (y2spinner));
	adjy = (GtkAdjustment *) gtk_adjustment_new(value1, -10000.0, value2, 1.0,
			5.0, 0.0);
	adjy2 = (GtkAdjustment *) gtk_adjustment_new(value2, value1, 10000.0, 1.0,
			5.0, 0.0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (yspinner), adjy);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (y2spinner), adjy2);

	value1 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (zspinner));
	value2 = gtk_spin_button_get_value(GTK_SPIN_BUTTON (z2spinner));
	adjz = (GtkAdjustment *) gtk_adjustment_new(value1, -10000.0, value2, 1.0,
			5.0, 0.0);
	adjz2 = (GtkAdjustment *) gtk_adjustment_new(value2, value1, 10000.0, 1.0,
			5.0, 0.0);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (zspinner), adjz);
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON (z2spinner), adjz2);

	g_signal_connect(G_OBJECT (adjx), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	g_signal_connect(G_OBJECT (adjx2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	g_signal_connect(G_OBJECT (adjy), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	g_signal_connect(G_OBJECT (adjy2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	g_signal_connect(G_OBJECT (adjz), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	g_signal_connect(G_OBJECT (adjz2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
}

/************************************************************************/
/* This function is called when the setup button is pressed in the main	*/
/* window or if gdpc is started without parameters. It sets up the	*/
/* setup window and all its buttons and entries.			*/
/************************************************************************/
void setupwindow(struct GlobalParams *params) {
	GtkWidget *browseb, *cancelb, *redrawb, *quitb, *check, *erasetoggle,
			*whitetoggle, *dumpcheck, *sortrtoggle;
	GtkWidget *dumptifcheck, *dumpjpgcheck;
	GtkWidget *vbox_main, *hbox_main, *vbox, *hbox1, *hbox2, *hbox3, *vboxright,
			*vboxmostright, *vboxleft, *hboxcube;
	GtkWidget *vboxcubel, *vboxcuber, *vboxcuberm, *hboxxc, *vboxxcl, *vboxxcr,
			*hboxssize, *hboxsleep, *hboxd, *vbox_header;
	GtkWidget *hbox_cube, *hboxsc, *scoltoggle;
	GtkWidget *file_label, *column_label, *ff_label, *cxyz_label, *separator;
	GtkWidget *cube_label, *x_label, *y_label, *z_label, *xcol_label,
			*ycol_label, *zcol_label, *tcol_label, *ssize_label;
	GtkWidget *ssx_label, *ssy_label, *sleep_label, *d_label, *header,
			*col_label, *misc_label, *draw_label, *empty_label;
	GtkWidget *hboxtd;
	GSList *group;

	newparams.mode = params->mode;
	newparams.erase = params->erase;
	newparams.whitebg = params->whitebg;
	newparams.colorset = params->colorset;
	newparams.fxyz = params->fxyz;
	newparams.sort = params->sort;
	newparams.vary = params->vary;
	newparams.dumpnum = params->dumpnum;
	newparams.tifjpg = params->tifjpg;
	newparams.dumpnum = params->dumpnum;
	newparams.usetypes = params->usetypes;

	usedump = FALSE;
	usescol = FALSE;

	setupwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (setupwin), "gdpc Setup");
	gtk_container_set_border_width(GTK_CONTAINER (setupwin), 5);

	g_signal_connect(G_OBJECT (setupwin), "destroy", G_CALLBACK (destroy),
			&setupwin);

	/* Create boxes for layout. */
	vbox_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	vbox_header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	hbox_main = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	hboxssize = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hboxd = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hboxsc = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hbox_cube = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	hboxcube = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	vboxcubel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxcuber = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxcuberm = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	hboxxc = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	vboxxcl = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	vboxxcr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	hboxsleep = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	hboxtd = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	vboxleft = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxright = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	vboxmostright = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_add(GTK_CONTAINER (setupwin), vbox_main);

	gtk_box_pack_start(GTK_BOX (vbox_main), vbox_header, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox_main), hbox_main, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox_main), hbox3, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox_main), vbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox_main), vboxmostright, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (vbox), hbox1, FALSE, FALSE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vbox), separator, FALSE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (vbox), hbox2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox2), vboxleft, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox2), vboxright, FALSE, FALSE, 0);

	header = gtk_label_new(" Setup ");
	gtk_box_pack_start(GTK_BOX (vbox_header), header, FALSE, TRUE, 0);
	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vbox_header), separator, FALSE, TRUE, 3);

	file_label = gtk_label_new("Input file : ");
	file_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(file_entry), 160);
	g_signal_connect(G_OBJECT (file_entry), "changed", G_CALLBACK (filechange),
			G_OBJECT (setupwin));
	if (params->file != NULL) {
		gtk_entry_set_text(GTK_ENTRY (file_entry), params->file);
	}
	gtk_widget_set_tooltip_text(file_entry, inputfilett);

	browseb = gtk_button_new_with_mnemonic("_Browse");
	g_signal_connect(G_OBJECT (browseb), "clicked", G_CALLBACK (filebrowser),
			G_OBJECT (setupwin));

	gtk_box_pack_start(GTK_BOX (hbox1), file_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX (hbox1), file_entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hbox1), browseb, FALSE, FALSE, 0);

	ssize_label = gtk_label_new("Drawingboard size [pixels]");
	ssx_label = gtk_label_new(" X : ");
	ssy_label = gtk_label_new(" Y : ");

	adjssizex = (GtkAdjustment *) gtk_adjustment_new(params->absxsize, 0.0,
			3000.0, 10.0, 5.0, 0.0);
	adjssizey = (GtkAdjustment *) gtk_adjustment_new(params->absysize, 0.0,
			3000.0, 10.0, 5.0, 0.0);

	ssxspinner = gtk_spin_button_new(adjssizex, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (ssxspinner), FALSE);
	ssyspinner = gtk_spin_button_new(adjssizey, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (ssyspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (vboxleft), ssize_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxssize), ssx_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxssize), ssxspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxssize), ssy_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxssize), ssyspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxleft), hboxssize, TRUE, TRUE, 0);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxleft), separator, FALSE, TRUE, 3);

	cxyz_label = gtk_label_new("Simulation box size");
	gtk_box_pack_start(GTK_BOX (vboxleft), cxyz_label, TRUE, TRUE, 3);

	gtk_box_pack_start(GTK_BOX (vboxleft), hbox_cube, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (vboxleft), hboxcube, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (hboxcube), vboxcubel, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (hboxcube), vboxcuber, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (hboxcube), vboxcuberm, TRUE, TRUE, 3);

	cube_label = gtk_label_new("Cube : ");

	adjcube = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10000.0, 1.0, 5.0,
			0.0);

	cubespinner = gtk_spin_button_new(adjcube, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (cubespinner), FALSE);
//    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (cubespinner),
//                                     GTK_SHADOW_OUT);
	gtk_box_pack_start(GTK_BOX (hbox_cube), cube_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hbox_cube), cubespinner, TRUE, TRUE, 0);
	gtk_widget_set_tooltip_text(cubespinner, cubett);

	x_label = gtk_label_new("X : ");

	if (params->xmin == 65535.0) {
		adjx = (GtkAdjustment *) gtk_adjustment_new(0.0, -10000.0, 0.0, 1.0,
				5.0, 0.0);
		adjx2 = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10000.0, 1.0,
				5.0, 0.0);
	} else {
		adjx = (GtkAdjustment *) gtk_adjustment_new(params->xmin, -10000.0,
				params->xmax, 1.0, 5.0, 0.0);
		adjx2 = (GtkAdjustment *) gtk_adjustment_new(params->xmax, params->xmin,
				10000.0, 1.0, 5.0, 0.0);
	}

	xspinner = gtk_spin_button_new(adjx, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (xspinner), FALSE);
	g_signal_connect(G_OBJECT (adjx), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	x2spinner = gtk_spin_button_new(adjx2, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (x2spinner), FALSE);
	g_signal_connect(G_OBJECT (adjx2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxcubel), x_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuber), xspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuberm), x2spinner, TRUE, TRUE, 0);

	y_label = gtk_label_new("Y : ");

	if (params->ymin == 65535.0) {
		adjy = (GtkAdjustment *) gtk_adjustment_new(0.0, -10000.0, 0.0, 1.0,
				5.0, 0.0);
		adjy2 = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10000.0, 1.0,
				5.0, 0.0);
	} else {
		adjy = (GtkAdjustment *) gtk_adjustment_new(params->ymin, -10000.0,
				params->ymax, 1.0, 5.0, 0.0);
		adjy2 = (GtkAdjustment *) gtk_adjustment_new(params->ymax, params->ymin,
				10000.0, 1.0, 5.0, 0.0);
	}

	yspinner = gtk_spin_button_new(adjy, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (yspinner), FALSE);
	g_signal_connect(G_OBJECT (adjy), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));

	y2spinner = gtk_spin_button_new(adjy2, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (y2spinner), FALSE);
	g_signal_connect(G_OBJECT (adjy2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxcubel), y_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuber), yspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuberm), y2spinner, TRUE, TRUE, 0);

	z_label = gtk_label_new("Z : ");

	if (params->zmin == 65535.0) {
		adjz = (GtkAdjustment *) gtk_adjustment_new(0.0, -10000.0, 0.0, 1.0,
				5.0, 0.0);
		adjz2 = (GtkAdjustment *) gtk_adjustment_new(0.0, 0.0, 10000.0, 1.0,
				5.0, 0.0);
	} else {
		adjz = (GtkAdjustment *) gtk_adjustment_new(params->zmin, -10000.0,
				params->zmax, 1.0, 5.0, 0.0);
		adjz2 = (GtkAdjustment *) gtk_adjustment_new(params->zmax, params->zmin,
				10000.0, 1.0, 5.0, 0.0);
	}

	zspinner = gtk_spin_button_new(adjz, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (zspinner), FALSE);
	g_signal_connect(G_OBJECT (adjz), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));

	z2spinner = gtk_spin_button_new(adjz2, 0, 2);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (z2spinner), FALSE);
	g_signal_connect(G_OBJECT (adjz2), "value_changed",
			G_CALLBACK (setspinlimits), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxcubel), z_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuber), zspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxcuberm), z2spinner, TRUE, TRUE, 0);

	gtk_widget_set_tooltip_text(xspinner, mintt);
	gtk_widget_set_tooltip_text(yspinner, mintt);
	gtk_widget_set_tooltip_text(zspinner, mintt);
	gtk_widget_set_tooltip_text(x2spinner, maxtt);
	gtk_widget_set_tooltip_text(y2spinner, maxtt);
	gtk_widget_set_tooltip_text(z2spinner, maxtt);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxleft), separator, FALSE, TRUE, 3);

	column_label = gtk_label_new("Input data column representations");
	gtk_box_pack_start(GTK_BOX (vboxleft), column_label, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX (vboxleft), hboxxc, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (hboxxc), vboxxcl, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX (hboxxc), vboxxcr, TRUE, TRUE, 3);

	xcol_label = gtk_label_new("X column : ");

	adjxc = (GtkAdjustment *) gtk_adjustment_new(params->xcolumn, 1.0, 100.0,
			1.0, 5.0, 0.0);

	xcspinner = gtk_spin_button_new(adjxc, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (xcspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (vboxxcl), xcol_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxxcr), xcspinner, TRUE, TRUE, 0);
	gtk_widget_set_tooltip_text(xcspinner, xcoltt);

	ycol_label = gtk_label_new("Y column : ");

	adjyc = (GtkAdjustment *) gtk_adjustment_new(params->ycolumn, 1.0, 100.0,
			1.0, 5.0, 0.0);

	ycspinner = gtk_spin_button_new(adjyc, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (ycspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (vboxxcl), ycol_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxxcr), ycspinner, TRUE, TRUE, 0);
	gtk_widget_set_tooltip_text(ycspinner, ycoltt);

	zcol_label = gtk_label_new("Z column : ");

	adjzc = (GtkAdjustment *) gtk_adjustment_new(params->zcolumn, 1.0, 100.0,
			1.0, 5.0, 0.0);

	zcspinner = gtk_spin_button_new(adjzc, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (zcspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (vboxxcl), zcol_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxxcr), zcspinner, TRUE, TRUE, 0);
	gtk_widget_set_tooltip_text(zcspinner, zcoltt);

	tcol_label = gtk_label_new("t column : ");

	adjtc = (GtkAdjustment *) gtk_adjustment_new(params->tcolumn, 1.0, 100.0,
			1.0, 5.0, 0.0);

	tcspinner = gtk_spin_button_new(adjtc, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (tcspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (vboxxcl), tcol_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxxcr), tcspinner, TRUE, TRUE, 0);
	gtk_widget_set_tooltip_text(tcspinner, tcoltt);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxleft), separator, FALSE, TRUE, 3);

	timedel_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(timedel_entry), 160);
	gtk_entry_set_text(GTK_ENTRY (timedel_entry), params->timedelim);
	timedel_label = gtk_label_new("Time unit : ");
	gtk_box_pack_start(GTK_BOX (hboxtd), timedel_label, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxtd), timedel_entry, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxleft), hboxtd, FALSE, TRUE, 0);
	if (!params->fxyz) {
		gtk_widget_set_sensitive(timedel_entry, FALSE);
		gtk_widget_set_sensitive(timedel_label, FALSE);
	} else {
		gtk_widget_set_sensitive(timedel_entry, TRUE);
		gtk_widget_set_sensitive(timedel_label, TRUE);
	}
	gtk_widget_set_tooltip_text(timedel_entry, timedeltt);

	ff_label = gtk_label_new("Input file format");
	gtk_box_pack_start(GTK_BOX (vboxright), ff_label, TRUE, TRUE, 0);

	usetypescheck = gtk_check_button_new_with_label(" Use coloring by type");

	check = gtk_radio_button_new_with_label(NULL, "XYZ file format");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "clicked", G_CALLBACK (toggle_checkxyz),
			G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));
	if (params->fxyz) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	}

	check = gtk_radio_button_new_with_label(group, "Arbitrary file format");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "clicked", G_CALLBACK (toggle_checkaff),
			G_OBJECT (setupwin));
	if (!params->fxyz) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	}

	g_signal_connect(G_OBJECT (usetypescheck), "toggled",
			G_CALLBACK (toggle_usetypes), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxright), usetypescheck, TRUE, TRUE, 0);
	if (params->usetypes)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (usetypescheck), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (usetypescheck), FALSE);
	gtk_widget_set_tooltip_text(usetypescheck, coltypett);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxright), separator, FALSE, TRUE, 3);

	draw_label = gtk_label_new(" Drawing options ");
	gtk_box_pack_start(GTK_BOX (vboxright), draw_label, TRUE, TRUE, 0);

	d_label = gtk_label_new("Size of drawn polygons :");

	adjd = (GtkAdjustment *) gtk_adjustment_new(params->radius, 1.0, 100.0, 1.0,
			5.0, 0.0);

	dspinner = gtk_spin_button_new(adjd, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (dspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (hboxd), d_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxd), dspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxright), hboxd, TRUE, TRUE, 0);

	check = gtk_radio_button_new_with_label(NULL, "Draw as rectangles");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkm0),
			G_OBJECT (setupwin));
	if (params->mode == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));

	check = gtk_radio_button_new_with_label(group, "Draw as circles");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkm1),
			G_OBJECT (setupwin));
	if (params->mode == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));

	check = gtk_radio_button_new_with_label(group, "Draw as rendered balls");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkm2),
			G_OBJECT (setupwin));
	if (params->mode == 2)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	empty_label = gtk_label_new("  ");
	gtk_box_pack_start(GTK_BOX (vboxright), empty_label, TRUE, TRUE, 0);

	check = gtk_radio_button_new_with_label(NULL, "Dont vary size with z");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkv0),
			G_OBJECT (setupwin));
	if (params->vary == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));

	check = gtk_radio_button_new_with_label(group,
			"Vary size with z, decreasing");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkv1),
			G_OBJECT (setupwin));
	if (params->vary == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));

	check = gtk_radio_button_new_with_label(group,
			"Vary size with z, increasing");
	gtk_box_pack_start(GTK_BOX (vboxright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkv2),
			G_OBJECT (setupwin));
	if (params->vary == 2)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxright), separator, FALSE, TRUE, 3);

	scoltoggle = gtk_check_button_new_with_label(
			"Only use inputlines with string :");
	g_signal_connect(G_OBJECT (scoltoggle), "toggled", G_CALLBACK (toggle_scol),
			G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxright), scoltoggle, TRUE, TRUE, 0);

	scol_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(scol_entry), 60);
	g_signal_connect(G_OBJECT (scol_entry), "changed", G_CALLBACK (filechange),
			G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxright), scol_entry, TRUE, TRUE, 0);

	scol_label = gtk_label_new(" in column ");
	adjscol = (GtkAdjustment *) gtk_adjustment_new(1.0, 1.0, 100.0, 1.0, 5.0,
			0.0);
	scolspinner = gtk_spin_button_new(adjscol, 0, 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (scolspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (hboxsc), scol_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxsc), scolspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxright), hboxsc, TRUE, TRUE, 0);

	gtk_widget_set_sensitive(scol_label, FALSE);
	gtk_widget_set_sensitive(scol_entry, FALSE);
	gtk_widget_set_sensitive(scolspinner, FALSE);

	col_label = gtk_label_new(" Color settings ");
	gtk_box_pack_start(GTK_BOX (vboxmostright), col_label, TRUE, TRUE, 0);

	check = gtk_radio_button_new_with_label(NULL, "Use default colors");
	gtk_box_pack_start(GTK_BOX (vboxmostright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkc0),
			G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));
	if (params->colorset == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	check = gtk_radio_button_new_with_label(group, "Use inverted colors");
	gtk_box_pack_start(GTK_BOX (vboxmostright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkc1),
			G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));
	if (params->colorset == 1)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	check = gtk_radio_button_new_with_label(group, "Use cold colors");
	gtk_box_pack_start(GTK_BOX (vboxmostright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkc2),
			G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));
	if (params->colorset == 2)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	check = gtk_radio_button_new_with_label(group, "Use cold colors 2");
	gtk_box_pack_start(GTK_BOX (vboxmostright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkc3),
			G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (check));
	if (params->colorset == 3)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	check = gtk_radio_button_new_with_label(group, "Use greyscale colors");
	gtk_box_pack_start(GTK_BOX (vboxmostright), check, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (check), "toggled", G_CALLBACK (toggle_checkc4),
			G_OBJECT (setupwin));
	if (params->colorset == 4)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (check), TRUE);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxmostright), separator, FALSE, TRUE, 3);

	misc_label = gtk_label_new("Misc. settings");
	gtk_box_pack_start(GTK_BOX (vboxmostright), misc_label, TRUE, TRUE, 0);

	erasetoggle = gtk_check_button_new_with_label(
			" Erase before drawing next frame");
	g_signal_connect(G_OBJECT (erasetoggle), "toggled",
			G_CALLBACK (toggle_erase), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), erasetoggle, TRUE, TRUE, 0);
	if (params->erase)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (erasetoggle), TRUE);

	whitetoggle = gtk_check_button_new_with_label(
			" Use white as backgroundcolor");
	g_signal_connect(G_OBJECT (whitetoggle), "toggled",
			G_CALLBACK (toggle_white), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), whitetoggle, TRUE, TRUE, 0);
	if (params->whitebg)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (whitetoggle), TRUE);

	sortrtoggle = gtk_check_button_new_with_label(" Reverse sorting");
	g_signal_connect(G_OBJECT (sortrtoggle), "toggled",
			G_CALLBACK (toggle_sortr), G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), sortrtoggle, TRUE, TRUE, 0);
	if (params->sort == 2)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (sortrtoggle), TRUE);

	sleep_label = gtk_label_new("Delay between frames [s] : ");

	adjsleep = (GtkAdjustment *) gtk_adjustment_new((params->interval / 1000),
			0.0, 100.0, 0.1, 5.0, 0.0);

	sleepspinner = gtk_spin_button_new(adjsleep, 0, 1);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON (sleepspinner), FALSE);
	gtk_box_pack_start(GTK_BOX (hboxsleep), sleep_label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (hboxsleep), sleepspinner, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX (vboxmostright), hboxsleep, TRUE, TRUE, 0);

	separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start(GTK_BOX (vboxmostright), separator, FALSE, TRUE, 3);

	dump_label = gtk_label_new(" Dumped pictures name (no extension) : ");
	dump_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(dump_entry), 60);
	gtk_widget_set_sensitive(dump_label, FALSE);
	gtk_widget_set_sensitive(dump_entry, FALSE);

	dtcheck = gtk_radio_button_new_with_label(NULL, " Add frame time");
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (dtcheck));
	gtk_widget_set_sensitive(dtcheck, FALSE);

	dncheck = gtk_radio_button_new_with_label(group, " Add frame number");
	gtk_widget_set_sensitive(dncheck, FALSE);

	okeyb = gtk_button_new_from_stock(GTK_STOCK_OK);

	dumpcheck = gtk_radio_button_new_with_label(NULL, " Do not dump images");
	gtk_box_pack_start(GTK_BOX (vboxmostright), dumpcheck, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (dumpcheck), "toggled",
			G_CALLBACK (toggle_checkdump), G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (dumpcheck));
	if (strlen(params->dumpname) == 0)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (dumpcheck), TRUE);

	dumptifcheck = gtk_radio_button_new_with_label(group,
			" Dump a .png of each frame");
	gtk_box_pack_start(GTK_BOX (vboxmostright), dumptifcheck, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (dumptifcheck), "toggled",
			G_CALLBACK (toggle_checkdumptif), G_OBJECT (setupwin));
	group = gtk_radio_button_get_group(GTK_RADIO_BUTTON (dumptifcheck));
	if (strlen(params->dumpname) > 0 && params->tifjpg) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (dumptifcheck), TRUE);
		gtk_entry_set_text(GTK_ENTRY (dump_entry), params->dumpname);
	}

	dumpjpgcheck = gtk_radio_button_new_with_label(group,
			" Dump a .jpg of each frame");
	gtk_box_pack_start(GTK_BOX (vboxmostright), dumpjpgcheck, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT (dumpjpgcheck), "toggled",
			G_CALLBACK (toggle_checkdumpjpg), G_OBJECT (setupwin));
	if (strlen(params->dumpname) > 0 && !params->tifjpg) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (dumpjpgcheck), TRUE);
	}

	gtk_box_pack_start(GTK_BOX (vboxmostright), dump_label, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT (dump_entry), "changed", G_CALLBACK (filechange),
			G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), dump_entry, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT (dtcheck), "toggled", G_CALLBACK (toggle_checkdt),
			G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), dtcheck, TRUE, TRUE, 0);

	g_signal_connect(G_OBJECT (dncheck), "toggled", G_CALLBACK (toggle_checkdn),
			G_OBJECT (setupwin));
	gtk_box_pack_start(GTK_BOX (vboxmostright), dncheck, TRUE, TRUE, 0);

	if (newparams.dumpnum)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (dncheck), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (dtcheck), TRUE);

	g_signal_connect(G_OBJECT (okeyb), "clicked", G_CALLBACK (okeypressed),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hbox3), okeyb, TRUE, TRUE, 0);
	if (!params->StartedAlready)
		gtk_widget_set_sensitive(okeyb, FALSE);

	cancelb = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT (cancelb), "clicked", G_CALLBACK (cancelpressed),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hbox3), cancelb, TRUE, TRUE, 0);
	if (!params->StartedAlready)
		gtk_widget_set_sensitive(cancelb, FALSE);

	redrawb = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
	g_signal_connect(G_OBJECT (redrawb), "clicked", G_CALLBACK (redrawpressed),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hbox3), redrawb, TRUE, TRUE, 0);
	if (!params->StartedAlready)
		gtk_widget_set_sensitive(redrawb, FALSE);

	quitb = gtk_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(G_OBJECT (quitb), "clicked", G_CALLBACK (quitpressed),
			(gpointer) params);
	gtk_box_pack_start(GTK_BOX (hbox3), quitb, TRUE, TRUE, 0);

	gtk_widget_show_all(setupwin);
}
