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

static char     RCSid[] =
    "$Id: main.c,v 1.1 2004/09/09 13:35:53 rogerms Exp $";


#define DEBUG	0

#include "config_gui.h"
#include "cmdspawn.h"
#include "debug.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>


#define PLUGIN_NAME	"GenMon"


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

typedef struct plugin_t {
    unsigned int    iTimerId;	/* Cyclic update */
    struct conf_t   oConf;
    struct monitor_t
                    oMonitor;
    char            acValue[32];	/* Commandline resulting string */
} plugin_t;


	/**************************************************************/

static int DisplayCmdOutput (struct plugin_t *p_poPlugin)
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
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    if (poPlugin->iTimerId) {
	g_source_remove (poPlugin->iTimerId);
	poPlugin->iTimerId = 0;
    }
    XFCE_PANEL_LOCK ();
    DisplayCmdOutput (poPlugin);
    XFCE_PANEL_UNLOCK ();
    poPlugin->iTimerId = g_timeout_add (poConf->iPeriod_ms,
					(GtkFunction) SetTimer, poPlugin);
    return (1);
}				/* SetTimer() */

	/**************************************************************/

static plugin_t *NewPlugin ()
	/* New instance of the plugin (constructor) */
{
    struct plugin_t *poPlugin;
    struct param_t *poConf;
    struct monitor_t *poMonitor;

    poPlugin = g_new (plugin_t, 1);
    memset (poPlugin, 0, sizeof (plugin_t));
    poConf = &(poPlugin->oConf.oParam);
    poMonitor = &(poPlugin->oMonitor);

    strcpy (poConf->acCmd, "");
    strcpy (poConf->acTitle, "(genmon)");

    poConf->fTitleDisplayed = 1;

    poConf->iPeriod_ms = 30 * 1000;
    poPlugin->iTimerId = 0;

    strcpy (poConf->acFont, "(default)");

    poMonitor->wEventBox = gtk_event_box_new ();
    gtk_widget_show (poMonitor->wEventBox);

    return (poPlugin);
}				/* NewPlugin() */

	/**************************************************************/

static gboolean plugin_create_control (Control * p_poCtrl)
	/* Plugin API */
	/* Create the plugin */
{
    struct plugin_t *poPlugin;

    TRACE ("plugin_create_control()\n");
    poPlugin = NewPlugin ();
    gtk_container_add (GTK_CONTAINER (p_poCtrl->base),
		       poPlugin->oMonitor.wEventBox);
    p_poCtrl->data = (gpointer) poPlugin;
    p_poCtrl->with_popup = FALSE;
    gtk_widget_set_size_request (p_poCtrl->base, -1, -1);
    return (TRUE);
}				/* plugin_create_control() */

	/**************************************************************/

static void plugin_free (Control * ctrl)
	/* Plugin API */
{
    struct plugin_t *poPlugin;

    TRACE ("plugin_free()\n");
    g_return_if_fail (ctrl != NULL);
    g_return_if_fail (ctrl->data != NULL);
    poPlugin = (plugin_t *) ctrl->data;
    if (poPlugin->iTimerId)
	g_source_remove (poPlugin->iTimerId);
    g_free (poPlugin);
}				/* plugin_free() */

	/**************************************************************/

static int SetMonitorFont (void *p_pvPlugin)
{
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcFont = poConf->acFont;
    const PangoFontDescription *poFont;

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

static void plugin_read_config (Control * p_poCtrl, xmlNodePtr p_poParent)
	/* Plugin API */
	/* Executed when the panel is started - Read the configuration
	   previously stored in xml file */
{
    struct plugin_t *poPlugin = (plugin_t *) p_poCtrl->data;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    xmlNodePtr      poNode;
    char           *pc;

    TRACE ("plugin_read_config()\n");
    if (!p_poParent)
	return;
    for (poNode = p_poParent->children; poNode; poNode = poNode->next) {
	if (!(xmlStrEqual (poNode->name, PLUGIN_NAME)))
	    continue;

	if ((pc = xmlGetProp (poNode, (CONF_CMD)))) {
	    memset (poConf->acCmd, 0, sizeof (poConf->acCmd));
	    strncpy (poConf->acCmd, pc, sizeof (poConf->acCmd) - 1);
	    xmlFree (pc);
	}

	if ((pc = xmlGetProp (poNode, (CONF_USE_LABEL)))) {
	    poConf->fTitleDisplayed = atoi (pc);
	    xmlFree (pc);
	}
	if (poConf->fTitleDisplayed)
	    gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));
	else
	    gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));

	if ((pc = xmlGetProp (poNode, (CONF_LABEL_TEXT)))) {
	    memset (poConf->acTitle, 0, sizeof (poConf->acTitle));
	    strncpy (poConf->acTitle, pc, sizeof (poConf->acTitle) - 1);
	    xmlFree (pc);
	    gtk_label_set_text (GTK_LABEL (poMonitor->wTitle),
				poConf->acTitle);
	}

	if ((pc = xmlGetProp (poNode, (CONF_UPDATE_PERIOD)))) {
	    poConf->iPeriod_ms = atoi (pc);
	    xmlFree (pc);
	}

	if ((pc = xmlGetProp (poNode, (CONF_FONT)))) {
	    memset (poConf->acFont, 0, sizeof (poConf->acFont));
	    strncpy (poConf->acFont, pc, sizeof (poConf->acFont) - 1);
	    xmlFree (pc);
	}

	SetMonitorFont (poPlugin);
    }
    SetTimer (poPlugin);
}				/* plugin_read_config() */

	/**************************************************************/

static void plugin_write_config (Control * p_poCtrl, xmlNodePtr p_poParent)
	/* Plugin API */
	/* Write plugin configuration into xml file */
{
    struct plugin_t *poPlugin = (plugin_t *) p_poCtrl->data;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    xmlNodePtr      poRoot;
    char            acBuffer[16];

    TRACE ("plugin_write_config()\n");

    poRoot = xmlNewTextChild (p_poParent, NULL, PLUGIN_NAME, NULL);

    xmlSetProp (poRoot, CONF_CMD, poConf->acCmd);

    sprintf (acBuffer, "%d", poConf->fTitleDisplayed);
    xmlSetProp (poRoot, CONF_USE_LABEL, acBuffer);

    xmlSetProp (poRoot, CONF_LABEL_TEXT, poConf->acTitle);

    sprintf (acBuffer, "%d", poConf->iPeriod_ms);
    xmlSetProp (poRoot, CONF_UPDATE_PERIOD, acBuffer);

    xmlSetProp (poRoot, CONF_FONT, poConf->acFont);
}				/* plugin_write_config() */

	/**************************************************************/

static void SetCmd (Widget_t p_wTF, void *p_pvPlugin)
	/* GUI callback setting the command to be spawn, whose output will 
	   be displayed using the panel-docked monitor */
{
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcCmd = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    memset (poConf->acCmd, 0, sizeof (poConf->acCmd));
    strncpy (poConf->acCmd, pcCmd, sizeof (poConf->acCmd) - 1);
}				/* SetCmd() */

	/**************************************************************/

static void ToggleTitle (Widget_t p_w, void *p_pvPlugin)
	/* GUI callback turning on/off the monitor bar legend */
{
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
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
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
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
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    float           r;

    TRACE ("SetPeriod()\n");
    r = gtk_spin_button_get_value (GTK_SPIN_BUTTON (p_wSc));
    poConf->iPeriod_ms = (r * 1000);
}				/* SetPeriod() */

	/**************************************************************/

static void UpdateConf (Widget_t w, void *p_pvPlugin)
	/* Called back when the configuration/options window is closed */
{
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
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
    xfce_info ("%s %s - Generic Monitor\n"
	       "Cyclically spawns a script/program, captures its output "
	       "and displays the resulting string in the panel\n\n"
	       "(c) 2004 Roger Seguin <roger_seguin@msn.com>",
	       PACKAGE, VERSION);
}				/* About() */

	/**************************************************************/

static void ChooseFont (Widget_t p_wPB, void *p_pvPlugin)
{
    struct plugin_t *poPlugin = (plugin_t *) p_pvPlugin;
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

static void plugin_create_options (Control * p_poCtrl,
				   GtkContainer * p_pxContainer,
				   Widget_t p_wDone)
	/* Plugin API */
	/* Create/pop up the configuration/options GUI */
{
    struct plugin_t *poPlugin = (plugin_t *) p_poCtrl->data;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    const char     *pcFont = poConf->acFont;

    TRACE ("plugin_create_options()\n");

    poPlugin->oConf.wTopLevel = gtk_widget_get_toplevel (p_wDone);

    (void) genmon_CreateConfigGUI (GTK_WIDGET (p_pxContainer), poGUI);

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

    g_signal_connect (GTK_WIDGET (p_wDone), "clicked",
		      G_CALLBACK (UpdateConf), poPlugin);

}				/* plugin_create_options() */

	/**************************************************************/

static void plugin_attach_callback (Control * ctrl, const gchar * signal,
				    GCallback cb, gpointer data)
	/* Plugin API */
	/* This function has to be defined, even when not being used at
	   all */
{
    TRACE ("plugin_attach_callback()\n");
}				/* plugin_attach_callback() */

	/**************************************************************/

static void plugin_set_size (Control * p_poCtrl, int p_size)
	/* Plugin API */
	/* Set the size of the panel-docked monitor */
{
    TRACE ("plugin_set_size()\n");
}				/* plugin_set_size() */

	/**************************************************************/

static void plugin_set_orientation (Control * p_poCtrl, int p_iOrientation)
	/* Plugin API */
	/* Invoked when the panel changes orientation */
{
    struct plugin_t *poPlugin = (plugin_t *) p_poCtrl->data;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    TRACE ("plugin_set_orientation()\n");

    if (poPlugin->iTimerId) {
	g_source_remove (poPlugin->iTimerId);
	poPlugin->iTimerId = 0;
    }

    gtk_container_remove (GTK_CONTAINER (poMonitor->wEventBox),
			  GTK_WIDGET (poMonitor->wBox));
    if (p_iOrientation == HORIZONTAL)
	poMonitor->wBox = gtk_hbox_new (FALSE, 0);
    else
	poMonitor->wBox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (poMonitor->wBox);
    gtk_container_set_border_width (GTK_CONTAINER
				    (poMonitor->wBox), border_width);
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
}				/* plugin_set_orientation() */

	/**************************************************************/

G_MODULE_EXPORT void xfce_control_class_init (ControlClass * cc)
	/* Plugin API */
	/* Main */
{
    TRACE ("xfce_control_class_init()\n");

    cc->name = PLUGIN_NAME;
    cc->caption = _("Generic Monitor");
    cc->create_control = (CreateControlFunc) plugin_create_control;
    cc->free = plugin_free;
    cc->attach_callback = plugin_attach_callback;
    cc->read_config = plugin_read_config;
    cc->write_config = plugin_write_config;
    cc->create_options = plugin_create_options;
    cc->set_size = plugin_set_size;
    cc->set_orientation = plugin_set_orientation;
    cc->set_theme = 0;
}				/* xfce_control_class_init() */

	/* Plugin API */
XFCE_PLUGIN_CHECK_INIT
	/**************************************************************/
/*
$Log: main.c,v $
Revision 1.1  2004/09/09 13:35:53  rogerms
Initial revision

Revision 1.4  2004/09/09 12:58:14  RogerSeguin
Increased commandline resulting string maximum size from 16 characters to 32

Revision 1.3  2004/08/31 20:27:48  RogerSeguin
Reset font when changing panel orientation

Revision 1.2  2004/08/28 09:58:19  RogerSeguin
Added font selector

Revision 1.1  2004/08/27 23:17:44  RogerSeguin
Initial revision

*/
