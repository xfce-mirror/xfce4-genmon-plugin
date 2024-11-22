/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Header file to construct the configure GUI
 *  Copyright (c) 2004 Roger Seguin <roger_seguin@msn.com>
 *                                  <http://rmlx.dyndns.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.

 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _config_gui_h
#define _config_gui_h

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>


typedef struct gui_t {
    /* Configuration GUI widgets */
    GtkWidget      *wTF_Cmd;
    GtkWidget      *wPB_File;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget      *wSc_Period;
    GtkWidget      *wTB_SingleRow;
    GtkWidget      *wRB_AlignLeft;
    GtkWidget      *wRB_AlignCenter;
    GtkWidget      *wRB_AlignRight;
    GtkWidget      *wRB_AlignFill;
    GtkWidget      *wPB_Font;
} gui_t;


#ifdef __cplusplus
extern          "C" {
#endif

    int genmon_CreateConfigGUI (GtkWidget * ParentWindow,
        struct gui_t *gui);
    /* Create configuration/Option GUI */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif/* _config_gui_h */
