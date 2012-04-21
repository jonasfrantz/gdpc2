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
#include "parameters.h"


/************************************************************************/
/* This function is used to swap to atoms with each other.		*/
/************************************************************************/
void swap3(struct xyzstruc *coords, gint i, gint j) 
{
double tmp;
gint tmp2;

    tmp = coords[i].xcoord; 
    coords[i].xcoord = coords[j].xcoord; 
    coords[j].xcoord = tmp;

    tmp = coords[i].ycoord; 
    coords[i].ycoord = coords[j].ycoord; 
    coords[j].ycoord = tmp;

    tmp = coords[i].zcoord; 
    coords[i].zcoord = coords[j].zcoord; 
    coords[j].zcoord = tmp;

    tmp2 = coords[i].index; 
    coords[i].index = coords[j].index; 
    coords[j].index = tmp2;

    tmp2 = coords[i].atype; 
    coords[i].atype = coords[j].atype; 
    coords[j].atype = tmp2;
}


/************************************************************************/
/* This function is used to compare two atoms coordinates to each other.*/
/************************************************************************/
gint compare3(struct xyzstruc *coords,gint i,gint j)
{
    if (coords[i].zcoord < coords[j].zcoord) return (-1);
    else if (coords[i].zcoord > coords[j].zcoord) return (1);
    else {
	if (coords[i].ycoord < coords[j].ycoord) return (-1);
	else if (coords[i].ycoord > coords[j].ycoord) return (1);
	else {
	    if (coords[i].xcoord < coords[j].xcoord) return (-1);
	    else if (coords[i].xcoord > coords[j].xcoord) return (1);
	    else return (0);
	}
    }
}


/************************************************************************/
/* This sorting function is a copy of the example in K&R C programming	*/
/* 2nd ed. page 120. Some sort of quicksort.				*/
/* It is used to sort the atoms by x,y and z coordinates.		*/
/************************************************************************/
void sortatoms(struct xyzstruc *coords, gint left, gint right, gboolean sort) 
{
gint i,last;

    if (left>=right) return;

    swap3(coords, left, (left+right)/2);
  
    last = left;

    for (i=left+1;i<=right;i++) 
	if (sort==1) {
	    if (compare3(coords,i,left)<0) swap3(coords, ++last, i);
	}
	else {
	    if (compare3(coords,i,left)>0) swap3(coords, ++last, i);
	}
	 
    swap3(coords, left, last);
    sortatoms(coords, left, last-1, sort);
    sortatoms(coords, last+1, right, sort);
}


