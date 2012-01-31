/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Main file for the Genmon plugin
 *  Copyright (c) 2004 Roger Seguin <roger_seguin@msn.com>
 *                                  <http://rmlx.dyndns.org>
 *  Copyright (c) 2006 Julien Devemy <jujucece@gmail.com>
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
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-panel-convenience.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>

#define PLUGIN_NAME    "GenMon"
#define BORDER    2
#define BUFMAX  1024


typedef GtkWidget *Widget_t;

typedef struct param_t {
    /* Configurable parameters */
    char            acCmd[128]; /* Commandline to spawn */
    int             fTitleDisplayed;
    char            acTitle[16];
    uint32_t        iPeriod_ms;
    char            acFont[128];
} param_t;

typedef struct conf_t {
    Widget_t        wTopLevel;
    struct gui_t    oGUI; /* Configuration/option dialog */
    struct param_t  oParam;
} conf_t;

typedef struct monitor_t {
    /* Plugin monitor */
    Widget_t        wEventBox;
    Widget_t        wBox;
    Widget_t        wImgBox;
    Widget_t        wTitle;
    Widget_t        wValue;
    Widget_t        wImage;
    Widget_t        wBar;
    Widget_t        wButton;
    Widget_t        wImgButton;
    char            onClickCmd[256];
} monitor_t;

typedef struct genmon_t {
    XfcePanelPlugin *plugin;
    unsigned int    iTimerId; /* Cyclic update */
    struct conf_t   oConf;
    struct monitor_t    oMonitor;
    char            acValue[BUFMAX]; /* Commandline resulting string */
} genmon_t;

/**************************************************************/
static void ExecOnClickCmd (Widget_t p_wSc, void *p_pvPlugin)
/* Execute the onClick Command */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    GError *error = NULL;

    xfce_spawn_command_line_on_screen( gdk_screen_get_default(), poMonitor->onClickCmd, 0, 0, &error );
    if (error) {
        char first[256];
        g_snprintf (first, sizeof(first), _("Could not run \"%s\""), poMonitor->onClickCmd);
        xfce_message_dialog (NULL, _("Xfce Panel"),
                             GTK_STOCK_DIALOG_ERROR, first, error->message,
                             GTK_STOCK_CLOSE, GTK_RESPONSE_OK, NULL);
        g_error_free (error);
    }

}

/**************************************************************/

static int DisplayCmdOutput (struct genmon_t *p_poPlugin)
 /* Launch the command, get its output and display it in the panel-docked
    text field */
{
    static GtkTooltips *s_poToolTips = 0;

    struct param_t *poConf = &(p_poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(p_poPlugin->oMonitor);
    char   acToolTips[256];
    char   acCompleteCmd[256];
    int    status;
    char  *begin;
    char  *end;
    int    newVersion=0;

    if (!s_poToolTips)
        s_poToolTips = gtk_tooltips_new ();

    /* If the command starts with ~ expand it */
    if (poConf->acCmd[0] == '~')
    {
        uid_t uid;
        struct passwd* spwd;

        uid = getuid();
        spwd = getpwuid(uid);
        sprintf(acCompleteCmd, "%s%s", spwd->pw_dir, &poConf->acCmd[1]);
        status = genmon_SpawnCmd (acCompleteCmd, p_poPlugin->acValue,
            sizeof (p_poPlugin->acValue), 1);
    }
    else
        status = genmon_SpawnCmd (poConf->acCmd, p_poPlugin->acValue,
            sizeof (p_poPlugin->acValue), 1);

    /* If the command fails, display XXX */
    if (status == -1)
        strcpy(p_poPlugin->acValue, "XXX");

    /* Normally it's impossible to overflow the buffer because p_poPlugin->acValue is < BUFMAX */

    /* Test if the result is an Image or a Text */
    begin=strstr(p_poPlugin->acValue, "<img>");
    end=strstr(p_poPlugin->acValue, "</img>");
    if ((begin != NULL) && (end != NULL) && (begin < end) && (end-begin < BUFMAX*sizeof(char)))
    {
        char  buf[BUFMAX];
        /* Get the image path */
        strncpy(buf, begin+5*sizeof(char), end-begin-5*sizeof(char));
        buf[end-begin-5*sizeof(char)]='\0';

        gtk_image_set_from_file (GTK_IMAGE (poMonitor->wImage), buf);
        gtk_image_set_from_file (GTK_IMAGE (poMonitor->wImgButton), buf);

        /* Test if the result has a clickable Image (button) */
        begin=strstr(p_poPlugin->acValue, "<click>");
        end=strstr(p_poPlugin->acValue, "</click>");
        if ((begin != NULL) && (end != NULL) && (begin < end) && (end-begin < BUFMAX*sizeof(char)))
        {
            char  buf[BUFMAX];
            /* Get the command path */
            strncpy(buf, begin+7*sizeof(char), end-begin-7*sizeof(char));
            buf[end-begin-7*sizeof(char)]='\0';

            strcpy(poMonitor->onClickCmd, buf);

            gtk_widget_show (poMonitor->wButton);
            gtk_widget_show (poMonitor->wImgButton);
            gtk_widget_hide (poMonitor->wImage);
        }
        else
        {
            gtk_widget_hide (poMonitor->wButton);
            gtk_widget_hide (poMonitor->wImgButton);
            gtk_widget_show (poMonitor->wImage);
        }

        newVersion=1;
    }
    else
    {
        gtk_widget_hide (poMonitor->wButton);
        gtk_widget_hide (poMonitor->wImgButton);
        gtk_widget_hide (poMonitor->wImage);
    }

    /* Test if the result is a Text */
    begin=strstr(p_poPlugin->acValue, "<txt>");
    end=strstr(p_poPlugin->acValue, "</txt>");
    if ((begin != NULL) && (end != NULL) && (begin < end) && (end-begin < BUFMAX*sizeof(char)))
    {
        char  buf[BUFMAX];
        /* Get the text */
        strncpy(buf, begin+5*sizeof(char), end-begin-5*sizeof(char));
        buf[end-begin-5*sizeof(char)]='\0';
        gtk_label_set_markup (GTK_LABEL (poMonitor->wValue), buf);
        gtk_widget_show (poMonitor->wValue);

        newVersion=1;
    }
    else
        gtk_widget_hide (poMonitor->wValue);

    /* Test if the result is a Bar */
    begin=strstr(p_poPlugin->acValue, "<bar>");
    end=strstr(p_poPlugin->acValue, "</bar>");
    if ((begin != NULL) && (end != NULL) && (begin < end) && (end-begin < BUFMAX*sizeof(char)))
    {
        char  buf[BUFMAX];
        int value;
        /* Get the text */
        strncpy(buf, begin+5*sizeof(char), end-begin-5*sizeof(char));
        buf[end-begin-5*sizeof(char)]='\0';
        value=atoi(buf);
        if (value<0)
            value=0;
        if (value>100)
            value=100;
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(poMonitor->wBar), (float)value/100.0);
        gtk_widget_show (poMonitor->wBar);

        newVersion=1;
    }
    else
        gtk_widget_hide (poMonitor->wBar);

    if (newVersion == 0)
    {
        gtk_widget_show (poMonitor->wValue);
        gtk_label_set_markup (GTK_LABEL (poMonitor->wValue),
            p_poPlugin->acValue);
    }

    /* Test if a ToolTip is given */
    begin=strstr(p_poPlugin->acValue, "<tool>");
    end=strstr(p_poPlugin->acValue, "</tool>");
    if ((begin != NULL) && (end != NULL) && (begin < end) && (end-begin < BUFMAX*sizeof(char)))
    {
        strncpy(acToolTips, begin+6, end-begin-6);
        acToolTips[end-begin-6]='\0';
    }
    else
        sprintf (acToolTips, "%s\n"
            "----------------\n"
            "%s\n"
            "Period (s): %d", poConf->acTitle, poConf->acCmd,
            poConf->iPeriod_ms / 1000);

    gtk_tooltips_set_tip (s_poToolTips, GTK_WIDGET (poMonitor->wEventBox),
        acToolTips, 0);

    return (0);

}/* DisplayCmdOutput() */

/**************************************************************/

static gboolean SetTimer (void *p_pvPlugin)
/* Recurrently update the panel-docked monitor through a timer */
/* Warning : should not be called directly (except the 1st time) */
/* To avoid multiple timers */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);

    DisplayCmdOutput (poPlugin);

    if (poPlugin->iTimerId == 0)
    {
        poPlugin->iTimerId = g_timeout_add (poConf->iPeriod_ms,
            (GSourceFunc) SetTimer, poPlugin);
        return FALSE;
    }

    return TRUE;
}/* SetTimer() */

/**************************************************************/

static genmon_t *genmon_create_control (XfcePanelPlugin *plugin)
/* Plugin API */
/* Create the plugin */
{
    struct genmon_t *poPlugin;
    struct param_t *poConf;
    struct monitor_t *poMonitor;
    GtkOrientation orientation = xfce_panel_plugin_get_orientation (plugin);
    int size = xfce_panel_plugin_get_size (plugin);

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

    poMonitor->wBox = xfce_hvbox_new (orientation, FALSE, 0);
    gtk_widget_show (poMonitor->wBox);
    gtk_container_set_border_width (GTK_CONTAINER
        (poMonitor->wBox), BORDER);
    gtk_container_add (GTK_CONTAINER (poMonitor->wEventBox),
        poMonitor->wBox);

    poMonitor->wTitle = gtk_label_new (poConf->acTitle);
    if (poConf->fTitleDisplayed)
        gtk_widget_show (poMonitor->wTitle);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
        GTK_WIDGET (poMonitor->wTitle), FALSE, FALSE, 0);

    /* Create a Box to put image and text */
    poMonitor->wImgBox = xfce_hvbox_new (orientation, FALSE, 0);
    gtk_widget_show (poMonitor->wImgBox);
    gtk_container_set_border_width (GTK_CONTAINER
        (poMonitor->wImgBox), 0);
    gtk_container_add (GTK_CONTAINER (poMonitor->wBox),
        poMonitor->wImgBox);

    /* Add Image */
    poMonitor->wImage = gtk_image_new ();
    gtk_box_pack_start (GTK_BOX (poMonitor->wImgBox),
        GTK_WIDGET (poMonitor->wImage), TRUE, FALSE, 0);

    /* Add Button */
    poMonitor->wButton = (Widget_t) xfce_create_panel_button ();
    xfce_panel_plugin_add_action_widget (plugin, poMonitor->wButton);
    gtk_box_pack_start (GTK_BOX (poMonitor->wImgBox),
        GTK_WIDGET (poMonitor->wButton), TRUE, FALSE, 0);

    /* Add Image Button*/
    poMonitor->wImgButton = gtk_image_new ();
    gtk_container_add (GTK_CONTAINER (poMonitor->wButton), poMonitor->wImgButton);
    gtk_container_set_border_width (GTK_CONTAINER (poMonitor->wButton), 0);

    /* Add Value */
    poMonitor->wValue = gtk_label_new ("");
    gtk_widget_show (poMonitor->wValue);
    gtk_box_pack_start (GTK_BOX (poMonitor->wImgBox),
        GTK_WIDGET (poMonitor->wValue), TRUE, FALSE, 0);

    /* Add Bar */
    poMonitor->wBar = gtk_progress_bar_new();
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
        GTK_WIDGET (poMonitor->wBar), FALSE, FALSE, 0);
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(poMonitor->wBar), GTK_PROGRESS_BOTTOM_TO_TOP);
    else
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(poMonitor->wBar), GTK_PROGRESS_LEFT_TO_RIGHT);

    return poPlugin;
}/* genmon_create_control() */

/**************************************************************/

static void genmon_free (XfcePanelPlugin *plugin, genmon_t *poPlugin)
/* Plugin API */
{
    TRACE ("genmon_free()\n");

    if (poPlugin->iTimerId)
        g_source_remove (poPlugin->iTimerId);
    g_free (poPlugin);
}/* genmon_free() */

/**************************************************************/

static int SetMonitorFont (void *p_pvPlugin)
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcFont = poConf->acFont;
    PangoFontDescription *poFont;

    if (*pcFont == '(') /* Default font */
        return (1);
    poFont = pango_font_description_from_string (poConf->acFont);
    if (!poFont)
        return (-1);
    gtk_widget_modify_font (poMonitor->wTitle, poFont);
    gtk_widget_modify_font (poMonitor->wValue, poFont);
    return (0);
}/* SetMonitorFont() */

/**************************************************************/

/* Configuration Keywords */
#define CONF_USE_LABEL    "UseLabel"
#define CONF_LABEL_TEXT    "Text"
#define CONF_CMD    "Command"
#define CONF_UPDATE_PERIOD    "UpdatePeriod"
#define CONF_FONT    "Font"

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
}/* genmon_read_config() */

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
}/* genmon_write_config() */

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
}/* SetCmd() */

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
}/* ToggleTitle() */

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
}/* SetLabel() */

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
}/* SetPeriod() */

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
    /* Restart timer */
    if (poPlugin->iTimerId) {
        g_source_remove (poPlugin->iTimerId);
        poPlugin->iTimerId = 0;
    }
    SetTimer(p_pvPlugin);
}/* UpdateConf() */

/**************************************************************/

static void About (Widget_t w, void *unused)
/* Called back when the About button in clicked */
{
    xfce_dialog_show_info (NULL, NULL,
        _("%s %s - Generic Monitor\n"
        "Cyclically spawns a script/program, captures its output "
        "and displays the resulting string in the panel\n\n"
        "(c) 2004 Roger Seguin <roger_seguin@msn.com>\n"
        "(c) 2006 Julien Devemy <jujucece@gmail.com>"),
        PACKAGE, VERSION);
}/* About() */

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
    if (*pcFont != '(') /* Default font */
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
}/* ChooseFont() */

/**************************************************************/

static void genmon_dialog_response (GtkWidget *dlg, int response,
    genmon_t *genmon)
{
    UpdateConf (genmon);
    gtk_widget_destroy (dlg);
    xfce_panel_plugin_unblock_menu (genmon->plugin);
    genmon_write_config (genmon->plugin, genmon);
    /* Do not wait the next timer to update display */
    DisplayCmdOutput (genmon);
}

static void genmon_create_options (XfcePanelPlugin *plugin,
    genmon_t *poPlugin)
/* Plugin API */
/* Create/pop up the configuration/options GUI */
{
    GtkWidget *dlg, *vbox;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct gui_t   *poGUI = &(poPlugin->oConf.oGUI);
    const char     *pcFont = poConf->acFont;

    TRACE ("genmon_create_options()\n");

    xfce_panel_plugin_block_menu (plugin);

    dlg = xfce_titled_dialog_new_with_buttons (_("Configuration"),
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT |
        GTK_DIALOG_NO_SEPARATOR,
        GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
        NULL);

    g_signal_connect (dlg, "response", G_CALLBACK (genmon_dialog_response),
        poPlugin);

    gtk_container_set_border_width (GTK_CONTAINER (dlg), 2);

    xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dlg), _("Generic Monitor"));

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

    if (*pcFont != '(') /* Default font */
        gtk_button_set_label (GTK_BUTTON (poGUI->wPB_Font),
        poConf->acFont);
    g_signal_connect (G_OBJECT (poGUI->wPB_Font), "clicked",
        G_CALLBACK (ChooseFont), poPlugin);

    gtk_widget_show (dlg);
}/* genmon_create_options() */

/**************************************************************/

static void genmon_set_orientation (XfcePanelPlugin *plugin,
    GtkOrientation p_iOrientation,
    genmon_t *poPlugin)
/* Plugin API */
/* Invoked when the panel changes orientation */
{
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    xfce_hvbox_set_orientation (XFCE_HVBOX (poMonitor->wBox), p_iOrientation);
    xfce_hvbox_set_orientation (XFCE_HVBOX (poMonitor->wImgBox), p_iOrientation);

    if (p_iOrientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(poMonitor->wBar), GTK_PROGRESS_BOTTOM_TO_TOP);
    else
        gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(poMonitor->wBar), GTK_PROGRESS_LEFT_TO_RIGHT);

    SetMonitorFont (poPlugin);

}/* genmon_set_orientation() */

/**************************************************************/
static gboolean genmon_set_size (XfcePanelPlugin *plugin, int size, genmon_t *poPlugin)
/* Plugin API */
/* Set the size of the panel-docked monitor */
{

    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    if (xfce_panel_plugin_get_orientation (plugin) == GTK_ORIENTATION_HORIZONTAL)
    {
        if (size>BORDER)
            gtk_widget_set_size_request(GTK_WIDGET(poMonitor->wBar),8, size-BORDER*2);
    }
    else
    {
        if (size>BORDER)
            gtk_widget_set_size_request(GTK_WIDGET(poMonitor->wBar), size-BORDER*2, 8);
    }

    return TRUE;
}/* genmon_set_size() */

/**************************************************************/

static void genmon_construct (XfcePanelPlugin *plugin)
{
    genmon_t *genmon;

    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

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

    g_signal_connect (plugin, "size-changed", G_CALLBACK (genmon_set_size), genmon);

    xfce_panel_plugin_menu_show_about (plugin);
    g_signal_connect (plugin, "about", G_CALLBACK (About), genmon);

    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin",
        G_CALLBACK (genmon_create_options), genmon);

    g_signal_connect (G_OBJECT (genmon->oMonitor.wButton), "clicked",
        G_CALLBACK (ExecOnClickCmd), genmon);
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (genmon_construct)

