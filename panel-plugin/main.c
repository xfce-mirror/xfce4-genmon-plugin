/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Copyright (c) 2004 Roger Seguin <roger_seguin@msn.com>
 *  					<http://rmlx.dyndns.org>
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
#include "cmdspawn.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/dialogs.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>


#define PLUGIN_NAME	"GenMon"
#define BORDER          8


typedef GtkWidget *Widget_t;

typedef struct param_t {
    /* Configurable parameters */
    char            acCmd[128];	/* Commandline to spawn */
    int             fTitleDisplayed;
    char            acTitle[16];
    uint32_t        iPeriod_ms;
    char            acFont[128];
} param_t;

typedef struct conf_t {
    Widget_t        wTopLevel;
    struct gui_t    oGUI;	/* Configuration/option dialog */
    struct param_t  oParam;
} conf_t;

typedef struct monitor_t {
    /* Plugin monitor */
    Widget_t        wEventBox;
    Widget_t        wBox;
    Widget_t        wTitle;
    Widget_t        wValue;
} monitor_t;

typedef struct genmon_t {
    XfcePanelPlugin *plugin;
    unsigned int    iTimerId;	/* Cyclic update */
    struct conf_t   oConf;
    struct monitor_t
                    oMonitor;
    char            acValue[32];	/* Commandline resulting string */
} genmon_t;


	/**************************************************************/

static int DisplayCmdOutput (struct genmon_t *p_poPlugin)
 /* Launch the command, get its output and display it in the panel-docked
    text field */
{
    static GtkTooltips *s_poToolTips = 0;

    struct param_t *poConf = &(p_poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(p_poPlugin->oMonitor);
    char            acToolTips[128];
    int             status;

    if (!s_poToolTips)
	s_poToolTips = gtk_tooltips_new ();
    status = genmon_SpawnCmd (poConf->acCmd, p_poPlugin->acValue,
			      sizeof (p_poPlugin->acValue));
    if (status == -1)
	return (-1);
    gtk_label_set_text (GTK_LABEL (poMonitor->wValue),
			p_poPlugin->acValue);
    sprintf (acToolTips, "%s\n"
	     "----------------\n"
	     "%s\n"
	     "Period (s): %d", poConf->acTitle, poConf->acCmd,
	     poConf->iPeriod_ms / 1000);
    gtk_tooltips_set_tip (s_poToolTips, GTK_WIDGET (poMonitor->wEventBox),
			  acToolTips, 0);

    return (0);
}				/* DisplayCmdOutput() */

	/**************************************************************/

static gboolean SetTimer (void *p_pvPlugin)
	/* Recurrently update the panel-docked monitor through a timer */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    DisplayCmdOutput (poPlugin);

    if (poPlugin->iTimerId == 0)
        poPlugin->iTimerId = g_timeout_add (poConf->iPeriod_ms,
	        			    (GSourceFunc) SetTimer, poPlugin);
    return TRUE;
}				/* SetTimer() */

	/**************************************************************/

static genmon_t *genmon_create_control (XfcePanelPlugin *plugin)
	/* Plugin API */
	/* Create the plugin */
{
    struct genmon_t *poPlugin;
    struct param_t *poConf;
    struct monitor_t *poMonitor;

    poPlugin = g_new (genmon_t, 1);
    memset (poPlugin, 0, sizeof (genmon_t));
    poConf = &(poPlugin->oConf.oParam);
    poMonitor = &(poPlugin->oMonitor);

    poPlugin->plugin = plugin;
    
    strcpy (poConf->acCmd, "");
    strcpy (poConf->acTitle, "(genmon)");

    poConf->fTitleDisplayed = 1;

    poConf->iPeriod_ms = 30 * 1000;
    poPlugin->iTimerId = 0;

    strcpy (poConf->acFont, "(default)");

    poMonitor->wEventBox = gtk_event_box_new ();
    gtk_widget_show (poMonitor->wEventBox);

    xfce_panel_plugin_add_action_widget (plugin, poMonitor->wEventBox);
    
    if (xfce_panel_plugin_get_orientation (plugin) == 
            GTK_ORIENTATION_HORIZONTAL)
	poMonitor->wBox = gtk_hbox_new (FALSE, 0);
    else
	poMonitor->wBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (poMonitor->wBox);
    gtk_container_set_border_width (GTK_CONTAINER
				    (poMonitor->wBox), 4);
    gtk_container_add (GTK_CONTAINER (poMonitor->wEventBox),
		       poMonitor->wBox);

    poMonitor->wTitle = gtk_label_new (poConf->acTitle);
    if (poConf->fTitleDisplayed)
	gtk_widget_show (poMonitor->wTitle);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			GTK_WIDGET (poMonitor->wTitle), FALSE, FALSE, 0);

    poMonitor->wValue = gtk_label_new ("");
    gtk_widget_show (poMonitor->wValue);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			GTK_WIDGET (poMonitor->wValue), FALSE, FALSE, 0);

    return poPlugin;
}				/* genmon_create_control() */

	/**************************************************************/

static void genmon_free (XfcePanelPlugin *plugin, genmon_t *poPlugin)
	/* Plugin API */
{
    TRACE ("genmon_free()\n");

    if (poPlugin->iTimerId)
	g_source_remove (poPlugin->iTimerId);
    g_free (poPlugin);
}				/* genmon_free() */

	/**************************************************************/

static int SetMonitorFont (void *p_pvPlugin)
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcFont = poConf->acFont;
    PangoFontDescription *poFont;

    if (*pcFont == '(')		/* Default font */
	return (1);
    poFont = pango_font_description_from_string (poConf->acFont);
    if (!poFont)
	return (-1);
    gtk_widget_modify_font (poMonitor->wTitle, poFont);
    gtk_widget_modify_font (poMonitor->wValue, poFont);
    return (0);
}				/* SetMonitorFont() */

	/**************************************************************/

	/* Configuration Keywords */
#define CONF_USE_LABEL		"UseLabel"
#define CONF_LABEL_TEXT		"Text"
#define CONF_CMD		"Command"
#define CONF_UPDATE_PERIOD	"UpdatePeriod"
#define CONF_FONT		"Font"

	/**************************************************************/

static void genmon_read_config (XfcePanelPlugin *plugin, genmon_t *poPlugin)
	/* Plugin API */
	/* Executed when the panel is started - Read the configuration
	   previously stored in xml file */
{
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    const char           *pc;
    char           *file;
    XfceRc         *rc;
    
    if (!(file = xfce_panel_plugin_lookup_rc_file (plugin)))
        return;
    
    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (!rc)
        return;
    
    if ((pc = xfce_rc_read_entry (rc, (CONF_CMD), NULL))) {
        memset (poConf->acCmd, 0, sizeof (poConf->acCmd));
        strncpy (poConf->acCmd, pc, sizeof (poConf->acCmd) - 1);
    }

    poConf->fTitleDisplayed = xfce_rc_read_int_entry (rc, (CONF_USE_LABEL), 1);
    if (poConf->fTitleDisplayed)
        gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));
    else
        gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));

    if ((pc = xfce_rc_read_entry (rc, (CONF_LABEL_TEXT), NULL))) {
        memset (poConf->acTitle, 0, sizeof (poConf->acTitle));
        strncpy (poConf->acTitle, pc, sizeof (poConf->acTitle) - 1);
        gtk_label_set_text (GTK_LABEL (poMonitor->wTitle),
                            poConf->acTitle);
    }

    poConf->iPeriod_ms = 
        xfce_rc_read_int_entry (rc, (CONF_UPDATE_PERIOD), 30 * 1000);

    if ((pc = xfce_rc_read_entry (rc, (CONF_FONT), NULL))) {
        memset (poConf->acFont, 0, sizeof (poConf->acFont));
        strncpy (poConf->acFont, pc, sizeof (poConf->acFont) - 1);
    }

    xfce_rc_close (rc);
}				/* genmon_read_config() */

	/**************************************************************/

static void genmon_write_config (XfcePanelPlugin *plugin, genmon_t *poPlugin)
	/* Plugin API */
	/* Write plugin configuration into xml file */
{
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    XfceRc *rc;
    char *file;

    if (!(file = xfce_panel_plugin_save_location (plugin, TRUE)))
        return;
    
    rc = xfce_rc_simple_open (file, FALSE);
    g_free (file);

    if (!rc)
        return;

    TRACE ("genmon_write_config()\n");

    xfce_rc_write_entry (rc, CONF_CMD, poConf->acCmd);

    xfce_rc_write_int_entry (rc, CONF_USE_LABEL, poConf->fTitleDisplayed);

    xfce_rc_write_entry (rc, CONF_LABEL_TEXT, poConf->acTitle);

    xfce_rc_write_int_entry (rc, CONF_UPDATE_PERIOD, poConf->iPeriod_ms);

    xfce_rc_write_entry (rc, CONF_FONT, poConf->acFont);

    xfce_rc_close (rc);
}				/* genmon_write_config() */

	/**************************************************************/

static void SetCmd (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the command to be spawn, whose output will 
	   be displayed using the panel-docked monitor */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcCmd = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    memset (poConf->acCmd, 0, sizeof (poConf->acCmd));
    strncpy (poConf->acCmd, pcCmd, sizeof (poConf->acCmd) - 1);
}				/* SetCmd() */

	/**************************************************************/

static void ToggleTitle (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback turning on/off the monitor bar legend */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    poConf->fTitleDisplayed =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (p_w));
    gtk_widget_set_sensitive (GTK_WIDGET (poGUI->wTF_Title),
			      poConf->fTitleDisplayed);
    if (poConf->fTitleDisplayed)
	gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));
    else
	gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));
}				/* ToggleTitle() */

	/**************************************************************/

static void SetLabel (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the legend of the monitor */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    const char     *acTitle = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    memset (poConf->acTitle, 0, sizeof (poConf->acTitle));
    strncpy (poConf->acTitle, acTitle, sizeof (poConf->acTitle) - 1);
    gtk_label_set_text (GTK_LABEL (poMonitor->wTitle), poConf->acTitle);
}				/* SetLabel() */

	/**************************************************************/

static void SetPeriod (Widget_t p_wSc, void *p_pvPlugin)
	/* Set the update period - To be used by the timer */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    float           r;

    TRACE ("SetPeriod()\n");
    r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (p_wSc));
    poConf->iPeriod_ms = (r * 1000);
}				/* SetPeriod() */

	/**************************************************************/

static void UpdateConf (void *p_pvPlugin)
	/* Called back when the configuration/options window is closed */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct conf_t  *poConf = &(poPlugin->oConf);
    struct gui_t   *poGUI = &(poConf->oGUI);

    TRACE ("UpdateConf()\n");
    SetCmd (poGUI->wTF_Cmd, poPlugin);
    SetLabel (poGUI->wTF_Title, poPlugin);
    SetMonitorFont (poPlugin);
    SetTimer (poPlugin);
}				/* UpdateConf() */

	/**************************************************************/

static void About (Widget_t w, void *unused)
	/* Called back when the About button in clicked */
{
    xfce_info (_("%s %s - Generic Monitor\n"
	       "Cyclically spawns a script/program, captures its output "
	       "and displays the resulting string in the panel\n\n"
	       "(c) 2004 Roger Seguin <roger_seguin@msn.com>"),
	       PACKAGE, VERSION);
}				/* About() */

	/**************************************************************/

static void ChooseFont (Widget_t p_wPB, void *p_pvPlugin)
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    Widget_t        wDialog;
    const char     *pcFont = poConf->acFont;
    int             iResponse;

    wDialog = gtk_font_selection_dialog_new (_("Font Selection"));
    gtk_window_set_transient_for (GTK_WINDOW (wDialog),
				  GTK_WINDOW (poPlugin->oConf.wTopLevel));
    if (*pcFont != '(')		/* Default font */
	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG
						 (wDialog), pcFont);
    iResponse = gtk_dialog_run (GTK_DIALOG (wDialog));
    if (iResponse == GTK_RESPONSE_OK) {
	pcFont = gtk_font_selection_dialog_get_font_name
	    (GTK_FONT_SELECTION_DIALOG (wDialog));
	if (pcFont && (strlen (pcFont) < sizeof (poConf->acFont) - 1)) {
	    strcpy (poConf->acFont, pcFont);
	    gtk_button_set_label (GTK_BUTTON (p_wPB), poConf->acFont);
	}
    }
    gtk_widget_destroy (wDialog);
}				/* ChooseFont() */

	/**************************************************************/

static void genmon_dialog_response (GtkWidget *dlg, int response, 
                                    genmon_t *genmon)
{
    UpdateConf (genmon);
    gtk_widget_destroy (dlg);
    xfce_panel_plugin_unblock_menu (genmon->plugin);
    genmon_write_config (genmon->plugin, genmon);
}

static void genmon_create_options (XfcePanelPlugin *plugin,
                                   genmon_t *poPlugin)
	/* Plugin API */
	/* Create/pop up the configuration/options GUI */
{
    GtkWidget *dlg, *header, *vbox;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    const char     *pcFont = poConf->acFont;

    TRACE ("genmon_create_options()\n");

    xfce_panel_plugin_block_menu (plugin);
    
    dlg = gtk_dialog_new_with_buttons (_("Configuration"), 
                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                GTK_DIALOG_DESTROY_WITH_PARENT |
                GTK_DIALOG_NO_SEPARATOR,
                GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
                NULL);
    
    g_signal_connect (dlg, "response", G_CALLBACK (genmon_dialog_response),
                      poPlugin);

    gtk_container_set_border_width (GTK_CONTAINER (dlg), 2);
    
    header = xfce_create_header (NULL, _("Generic Monitor"));
    gtk_widget_set_size_request (GTK_BIN (header)->child, -1, 32);
    gtk_container_set_border_width (GTK_CONTAINER (header), BORDER - 2);
    gtk_widget_show (header);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), header,
                        FALSE, TRUE, 0);
    
    vbox = gtk_vbox_new(FALSE, BORDER);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BORDER - 2);
    gtk_widget_show(vbox);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->vbox), vbox,
                        TRUE, TRUE, 0);
    
    poPlugin->oConf.wTopLevel = dlg;

    (void) genmon_CreateConfigGUI (GTK_WIDGET (vbox), poGUI);

    g_signal_connect (GTK_WIDGET (poGUI->wPB_About), "clicked",
		      G_CALLBACK (About), 0);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poGUI->wTB_Title),
				  poConf->fTitleDisplayed);
    gtk_widget_set_sensitive (GTK_WIDGET (poGUI->wTF_Title),
			      poConf->fTitleDisplayed);
    g_signal_connect (GTK_WIDGET (poGUI->wTB_Title), "toggled",
		      G_CALLBACK (ToggleTitle), poPlugin);

    gtk_entry_set_text (GTK_ENTRY (poGUI->wTF_Cmd), poConf->acCmd);
    g_signal_connect (GTK_WIDGET (poGUI->wTF_Cmd), "activate",
		      G_CALLBACK (SetCmd), poPlugin);

    gtk_entry_set_text (GTK_ENTRY (poGUI->wTF_Title), poConf->acTitle);
    g_signal_connect (GTK_WIDGET (poGUI->wTF_Title), "activate",
		      G_CALLBACK (SetLabel), poPlugin);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (poGUI->wSc_Period),
			       ((double) poConf->iPeriod_ms / 1000));
    g_signal_connect (GTK_WIDGET (poGUI->wSc_Period), "value_changed",
		      G_CALLBACK (SetPeriod), poPlugin);

    if (*pcFont != '(')		/* Default font */
	gtk_button_set_label (GTK_BUTTON (poGUI->wPB_Font),
			      poConf->acFont);
    g_signal_connect (G_OBJECT (poGUI->wPB_Font), "clicked",
		      G_CALLBACK (ChooseFont), poPlugin);

    gtk_widget_show (dlg);
}				/* genmon_create_options() */

	/**************************************************************/

static void genmon_set_orientation (XfcePanelPlugin *plugin, 
                                    GtkOrientation p_iOrientation,
                                    genmon_t *poPlugin)
	/* Plugin API */
	/* Invoked when the panel changes orientation */
{
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    TRACE ("genmon_set_orientation()\n");

    gtk_container_remove (GTK_CONTAINER (poMonitor->wEventBox),
			  GTK_WIDGET (poMonitor->wBox));
    if (p_iOrientation == GTK_ORIENTATION_HORIZONTAL)
	poMonitor->wBox = gtk_hbox_new (FALSE, 0);
    else
	poMonitor->wBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (poMonitor->wBox);
    gtk_container_set_border_width (GTK_CONTAINER
				    (poMonitor->wBox), 4);
    gtk_container_add (GTK_CONTAINER (poMonitor->wEventBox),
		       poMonitor->wBox);

    poMonitor->wTitle = gtk_label_new (poConf->acTitle);
    if (poConf->fTitleDisplayed)
	gtk_widget_show (poMonitor->wTitle);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			GTK_WIDGET (poMonitor->wTitle), FALSE, FALSE, 0);

    poMonitor->wValue = gtk_label_new ("");
    gtk_widget_show (poMonitor->wValue);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
			GTK_WIDGET (poMonitor->wValue), FALSE, FALSE, 0);

    SetMonitorFont (poPlugin);

    SetTimer (poPlugin);
}				/* genmon_set_orientation() */

	/**************************************************************/

static void genmon_construct (XfcePanelPlugin *plugin)
{
    genmon_t *genmon;

    genmon = genmon_create_control (plugin);

    genmon_read_config (plugin, genmon);

    gtk_container_add (GTK_CONTAINER (plugin), genmon->oMonitor.wEventBox);

    SetMonitorFont (genmon);

    SetTimer (genmon);

    g_signal_connect (plugin, "free-data", G_CALLBACK (genmon_free), genmon);

    g_signal_connect (plugin, "save", G_CALLBACK (genmon_write_config), 
                      genmon);

    g_signal_connect (plugin, "orientation-changed", 
                      G_CALLBACK (genmon_set_orientation), genmon);

    xfce_panel_plugin_menu_show_about (plugin);
    g_signal_connect (plugin, "about", G_CALLBACK (About), genmon);

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin", 
                      G_CALLBACK (genmon_create_options), genmon);
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (genmon_construct)

	/**************************************************************/
/*
$Log: main.c,v $
Revision 1.1.1.2  2004/11/01 00:22:48  rogerms
*** empty log message ***

Revision 1.1.1.1  2004/09/09 13:35:53  rogerms
V1.0

Revision 1.4  2004/09/09 12:58:14  RogerSeguin
Increased commandline resulting string maximum size from 16 characters to 32

Revision 1.3  2004/08/31 20:27:48  RogerSeguin
Reset font when changing panel orientation

Revision 1.2  2004/08/28 09:58:19  RogerSeguin
Added font selector

Revision 1.1  2004/08/27 23:17:44  RogerSeguin
Initial revision

*/
