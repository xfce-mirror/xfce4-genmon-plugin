/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Construct the configure GUI
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

#include "config_gui.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>


#define COPYVAL(var, field)    ((var)->field = field)


/**** GUI initially created using glade-2 ****/

/* Use the gtk_button_new_with_mnemonic() function for text-based
   push buttons */
/* Use "#define gtk_button_new_with_mnemonic(x) gtk_button_new()"
   for color-filled buttons */

#define gtk_button_new_with_mnemonic(x) gtk_button_new()

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
  gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

int genmon_CreateConfigGUI (GtkWidget *vbox1,
    struct gui_t *p_poGUI)
{
    GtkWidget      *table1;
    GtkWidget      *label1;
    GtkWidget      *wTF_Cmd;
    GtkWidget      *wPB_File;
    GtkWidget      *eventbox1;    
    GtkAdjustment  *wSc_Period_adj;
    GtkWidget      *wSc_Period;
    GtkWidget      *label2;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget	   *wTB_SingleRow;
    GtkWidget	   *label3;
    GtkWidget      *hseparator10;
    GtkWidget      *wPB_Font;

    table1 = gtk_grid_new ();
    gtk_grid_set_column_spacing(GTK_GRID (table1), 12);
    gtk_grid_set_row_spacing(GTK_GRID (table1), 6);
    gtk_grid_set_row_homogeneous(GTK_GRID (table1), TRUE);
    gtk_widget_show (table1);
    gtk_box_pack_start (GTK_BOX (vbox1), table1, FALSE, TRUE, 0);

    label1 = gtk_label_new (_("Command"));
    gtk_widget_show (label1);
    gtk_grid_attach (GTK_GRID (table1), label1, 0, 0, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign (GTK_LABEL (label1), 0.0f);
    gtk_widget_set_valign (label1, GTK_ALIGN_CENTER);

    wTF_Cmd = gtk_entry_new ();
    gtk_widget_show (wTF_Cmd);
    gtk_grid_attach (GTK_GRID (table1), wTF_Cmd, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (wTF_Cmd, _("Input the shell command to spawn, then press <Enter>"));
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Cmd), 128);

    wPB_File = gtk_button_new_with_label ("...");
    gtk_widget_set_margin_top (GTK_WIDGET (wPB_File), 6);
    gtk_widget_show (wPB_File);
    gtk_grid_attach (GTK_GRID (table1), wPB_File, 2, 0, 1, 1);
    gtk_widget_set_tooltip_text (wPB_File, _("Press to select file..."));

    wTB_Title = gtk_check_button_new_with_mnemonic (_("Label"));
    gtk_widget_show (wTB_Title);
    gtk_grid_attach (GTK_GRID (table1), wTB_Title, 0, 1, 1, 1);
    gtk_widget_set_tooltip_text (wTB_Title, _("Tick to display label"));

    wTF_Title = gtk_entry_new ();
    gtk_widget_show (wTF_Title);
    gtk_grid_attach (GTK_GRID (table1), wTF_Title, 1, 1, 2, 1);
 
    gtk_widget_set_tooltip_text (wTF_Title, _("Input the plugin label, then press <Enter>"));
    gtk_entry_set_max_length (GTK_ENTRY (wTF_Title), 16);
    gtk_entry_set_text (GTK_ENTRY (wTF_Title), _("(genmon)"));

    label2 = gtk_label_new (_("Period (s)"));
    gtk_widget_show (label2);
    gtk_grid_attach (GTK_GRID (table1), label2, 0, 2, 1, 1);
    gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign (GTK_LABEL (label2), 0.0f);
    gtk_widget_set_valign (label2, GTK_ALIGN_CENTER);

    eventbox1 = gtk_event_box_new ();
    gtk_widget_show (eventbox1);
    gtk_grid_attach (GTK_GRID (table1), eventbox1, 1, 2, 2, 1);
    gtk_widget_set_valign(GTK_WIDGET(eventbox1), GTK_ALIGN_CENTER);
    gtk_widget_set_halign(GTK_WIDGET(eventbox1), GTK_ALIGN_START);
    gtk_widget_set_hexpand(GTK_WIDGET(eventbox1), TRUE);

    wSc_Period_adj = gtk_adjustment_new (15, .25, 60*60*24, .25, 1, 0);
    wSc_Period = gtk_spin_button_new (GTK_ADJUSTMENT (wSc_Period_adj), .25, 2);
    gtk_widget_show (wSc_Period);
    gtk_container_add (GTK_CONTAINER (eventbox1), wSc_Period);
    gtk_widget_set_tooltip_text (wSc_Period, _("Interval between 2 consecutive spawns"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (wSc_Period), TRUE);

    wTB_SingleRow = gtk_check_button_new ();
    gtk_widget_show (wTB_SingleRow);
    gtk_grid_attach (GTK_GRID (table1), wTB_SingleRow, 0, 3, 1, 1);
    gtk_widget_set_tooltip_text (wTB_SingleRow, _("Tick to force plugin to use only one row"));

    label3 = gtk_label_new (_("Use a single panel row"));
    gtk_widget_show (label3);
    gtk_grid_attach (GTK_GRID (table1), label3, 1, 3, 2, 1);
    gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign (GTK_LABEL (label3), 0.0f);
    gtk_widget_set_valign (label3, GTK_ALIGN_CENTER);

    wPB_Font = gtk_button_new_with_label (_("Select the display font..."));
    gtk_widget_set_margin_top (GTK_WIDGET (wPB_Font), 6);
    gtk_widget_show (wPB_Font);
    gtk_grid_attach (GTK_GRID (table1), wPB_Font, 0, 4, 3, 1);
    gtk_widget_set_tooltip_text (wPB_Font, _("Press to change font..."));
    
    hseparator10 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top (GTK_WIDGET (hseparator10), 6);
    gtk_widget_set_margin_bottom (GTK_WIDGET (hseparator10), 0);
    gtk_widget_show (hseparator10);
    gtk_grid_attach (GTK_GRID (table1), hseparator10, 0, 4, 3, 1);
            
    if (p_poGUI) 
    {
        COPYVAL (p_poGUI, wTF_Cmd);
        COPYVAL (p_poGUI, wPB_File);
        COPYVAL (p_poGUI, wTB_Title);
        COPYVAL (p_poGUI, wTF_Title);
        COPYVAL (p_poGUI, wSc_Period);
        COPYVAL (p_poGUI, wTB_SingleRow);
        COPYVAL (p_poGUI, wPB_Font);
    }
    return (0);
}/* CreateConfigGUI() */

