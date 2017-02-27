/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Main file for the Genmon plugin
 *  Copyright (c) 2004 Roger Seguin <roger_seguin@msn.com>
 *                                  <http://rmlx.dyndns.org>
 *  Copyright (c) 2006 Julien Devemy <jujucece@gmail.com>
 *  Copyright (c) 2012 John Lindgren <john.lindgren@aol.com>
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PLUGIN_NAME    "GenMon"
#define BORDER    2

typedef struct param_t {
    /* Configurable parameters */
    char           *acCmd; /* Commandline to spawn */
    int             fTitleDisplayed;
    char           *acTitle;
    uint32_t        iPeriod_ms;
    char           *acFont;
} param_t;

typedef struct conf_t {
    GtkWidget      *wTopLevel;
    struct gui_t    oGUI; /* Configuration/option dialog */
    struct param_t  oParam;
} conf_t;

typedef struct monitor_t {
    /* Plugin monitor */
    GtkWidget      *wEventBox;
    GtkWidget      *wBox;
    GtkWidget      *wImgBox;
    GtkWidget      *wTitle;
    GtkWidget      *wValue;
    GtkWidget      *wValButton;
    GtkWidget      *wValButtonLabel;
    GtkWidget      *wImage;
    GtkWidget      *wBar;
    GtkWidget      *wButton;
    GtkWidget      *wImgButton;
    char           *onClickCmd;
    char           *onValClickCmd;
} monitor_t;

typedef struct genmon_t {
    XfcePanelPlugin *plugin;
    unsigned int    iTimerId; /* Cyclic update */
    struct conf_t   oConf;
    struct monitor_t    oMonitor;
    char            *acValue; /* Commandline resulting string */
} genmon_t;

/**************************************************************/
static void ExecOnClickCmd (GtkWidget *p_wSc, void *p_pvPlugin)
/* Execute the onClick Command */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    GError *error = NULL;

    xfce_spawn_command_line_on_screen( gdk_screen_get_default(), poMonitor->onClickCmd, 0, 0, &error );
    if (error) {
        char *first = g_strdup_printf (_("Could not run \"%s\""), poMonitor->onClickCmd);
        xfce_message_dialog (NULL, _("Xfce Panel"),
                             "dialog-error", first, error->message,
                             "gtk-close", GTK_RESPONSE_OK, NULL);
        g_error_free (error);
        g_free (first);
    }

}

/**************************************************************/
static void ExecOnValClickCmd (GtkWidget *p_wSc, void *p_pvPlugin)
/* Execute the onClick Command */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    GError *error = NULL;

    xfce_spawn_command_line_on_screen( gdk_screen_get_default(), poMonitor->onValClickCmd, 0, 0, &error );
    if (error) {
        char *first = g_strdup_printf (_("Could not run \"%s\""), poMonitor->onValClickCmd);
        xfce_message_dialog (NULL, _("Xfce Panel"),
                             "dialog-error", first, error->message,
                             "gtk-close", GTK_RESPONSE_OK, NULL);
        g_error_free (error);
        g_free (first);
    }

}

/**************************************************************/
static int DisplayCmdOutput (struct genmon_t *p_poPlugin)
 /* Launch the command, get its output and display it in the panel-docked
    text field */
{
    struct param_t *poConf = &(p_poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(p_poPlugin->oMonitor);
    char  *acToolTips;
    char  *begin;
    char  *end;
    int    newVersion=0;

    g_free (p_poPlugin->acValue);
    if (poConf->acCmd[0])
        p_poPlugin->acValue = genmon_SpawnCmd (poConf->acCmd, 1);
    else
        p_poPlugin->acValue = NULL;

    /* If the command fails, display XXX */
    if (!p_poPlugin->acValue)
        p_poPlugin->acValue = g_strdup ("XXX");

    /* Test if the result is an Image */
    begin=strstr(p_poPlugin->acValue, "<img>");
    end=strstr(p_poPlugin->acValue, "</img>");
    if (begin && end && begin < end)
    {
        /* Get the image path */
        char *buf = g_strndup (begin + 5, end - begin - 5);
        gtk_image_set_from_file (GTK_IMAGE (poMonitor->wImage), buf);
        gtk_image_set_from_file (GTK_IMAGE (poMonitor->wImgButton), buf);
        g_free (buf);

        /* Test if the result has a clickable Image (button) */
        begin=strstr(p_poPlugin->acValue, "<click>");
        end=strstr(p_poPlugin->acValue, "</click>");
        if (begin && end && begin < end)
        {
            /* Get the command path */
            g_free (poMonitor->onClickCmd);
            poMonitor->onClickCmd = g_strndup (begin + 7, end - begin - 7);

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
    if (begin && end && begin < end)
    {
        /* Get the text */
        char *buf = g_strndup (begin + 5, end - begin - 5);
        gtk_label_set_markup (GTK_LABEL (poMonitor->wValue), buf);
        
        /* Test if the result has a clickable Value (button) */
        begin=strstr(p_poPlugin->acValue, "<txtclick>");
        end=strstr(p_poPlugin->acValue, "</txtclick>");
        if (begin && end && begin < end)
        {
            /* Add the text to the button label too*/
            gtk_label_set_markup (GTK_LABEL (poMonitor->wValButtonLabel), buf);

            /* Get the command path */
            g_free (poMonitor->onValClickCmd);
            poMonitor->onValClickCmd = g_strndup (begin + 10, end - begin - 10);
            
            gtk_widget_show (poMonitor->wValButton);
            gtk_widget_show (poMonitor->wValButtonLabel);
            gtk_widget_hide (poMonitor->wValue);
            
        }
        else
        {
            gtk_widget_hide (poMonitor->wValButton);
            gtk_widget_hide (poMonitor->wValButtonLabel);
            gtk_widget_show (poMonitor->wValue);
        }

        newVersion=1;
        g_free (buf);
    }
    else
    {
        gtk_widget_hide (poMonitor->wValue);
        gtk_widget_hide (poMonitor->wValButton);
        gtk_widget_hide (poMonitor->wValButtonLabel);
    }

    /* Test if the result is a Bar */
    begin=strstr(p_poPlugin->acValue, "<bar>");
    end=strstr(p_poPlugin->acValue, "</bar>");
    if (begin && end && begin < end)
    {
        char *buf;
        int value;

        /* Get the text */
        buf = g_strndup (begin + 5, end - begin - 5);
        value = atoi (buf);
        g_free (buf);

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
        gtk_label_set_text (GTK_LABEL (poMonitor->wValue),
            p_poPlugin->acValue);
    }

    /* Test if a ToolTip is given */
    begin=strstr(p_poPlugin->acValue, "<tool>");
    end=strstr(p_poPlugin->acValue, "</tool>");
    if (begin && end && begin < end)
        acToolTips = g_strndup (begin + 6, end - begin - 6);
    else
        acToolTips = g_strdup_printf ("%s\n"
            "----------------\n"
            "%s\n"
            "Period (s): %d", poConf->acTitle, poConf->acCmd,
            poConf->iPeriod_ms / 1000);

    gtk_widget_set_tooltip_text (GTK_WIDGET (poMonitor->wEventBox),acToolTips);
    g_free (acToolTips);

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
    GtkSettings *settings;
    gchar *default_font;

    #if GTK_CHECK_VERSION (3, 16, 0)
        GtkCssProvider *css_provider;
        gchar * css;
    #endif
    
    poPlugin = g_new (genmon_t, 1);
    memset (poPlugin, 0, sizeof (genmon_t));
    poConf = &(poPlugin->oConf.oParam);
    poMonitor = &(poPlugin->oMonitor);

    poPlugin->plugin = plugin;

    poConf->acCmd = g_strdup ("");
    poConf->acTitle = g_strdup ("(genmon)");

    poConf->fTitleDisplayed = 1;

    poConf->iPeriod_ms = 30 * 1000;
    poPlugin->iTimerId = 0;

    // PangoFontDescription needs a font and we can't use "(Default)" anymore.
    // Use GtkSettings to get the current default font and use that, or set default to "Sans 10"
    settings = gtk_settings_get_default();
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(settings), "gtk-font-name"))
    {
        g_object_get(settings, "gtk-font-name", &default_font, NULL);
        poConf->acFont = g_strdup (default_font); 
    }
    else
        poConf->acFont = g_strdup ("Sans 10");

    poMonitor->wEventBox = gtk_event_box_new ();
    gtk_event_box_set_visible_window (
            GTK_EVENT_BOX (poMonitor->wEventBox), FALSE);
    gtk_widget_show (poMonitor->wEventBox);

    xfce_panel_plugin_add_action_widget (plugin, poMonitor->wEventBox);

    poMonitor->wBox = gtk_box_new (orientation, 0);
    gtk_widget_show (poMonitor->wBox);
    gtk_container_set_border_width (GTK_CONTAINER
        (poMonitor->wBox), 0);
    gtk_container_add (GTK_CONTAINER (poMonitor->wEventBox),
        poMonitor->wBox);

    poMonitor->wTitle = gtk_label_new (poConf->acTitle);
    if (poConf->fTitleDisplayed)
        gtk_widget_show (poMonitor->wTitle);
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
        GTK_WIDGET (poMonitor->wTitle), FALSE, FALSE, 0);

    /* Create a Box to put image and text */
    poMonitor->wImgBox = gtk_box_new (orientation, 0);
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
    poMonitor->wButton = xfce_create_panel_button ();
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

    /* Add Value Button */
    poMonitor->wValButton = xfce_create_panel_button ();
    xfce_panel_plugin_add_action_widget (plugin, poMonitor->wValButton);
    gtk_box_pack_start (GTK_BOX (poMonitor->wImgBox),
        GTK_WIDGET (poMonitor->wValButton), TRUE, FALSE, 0);

    /* Add Value Button Label */
    poMonitor->wValButtonLabel = gtk_label_new ("");
    gtk_container_add (GTK_CONTAINER (poMonitor->wValButton), poMonitor->wValButtonLabel);
    gtk_container_set_border_width (GTK_CONTAINER (poMonitor->wValButton), 0);

    /* Add Bar */
    poMonitor->wBar = gtk_progress_bar_new();
    gtk_box_pack_start (GTK_BOX (poMonitor->wBox),
        GTK_WIDGET (poMonitor->wBar), FALSE, FALSE, 0);
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wBar), GTK_ORIENTATION_VERTICAL);
        gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(poMonitor->wBar), TRUE);
    }
    else {
        gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wBar), GTK_ORIENTATION_HORIZONTAL);
        gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(poMonitor->wBar), FALSE);
    }


    /* make widget padding consistent */
    #if GTK_CHECK_VERSION (3, 16, 0)
    #if GTK_CHECK_VERSION (3, 20, 0)
        css = g_strdup_printf("\
            progressbar.horizontal trough { min-height: 6px; }\
            progressbar.horizontal progress { min-height: 6px; }\
            progressbar.vertical trough { min-width: 6px; }\
            progressbar.vertical progress { min-width: 6px; }");
    #else
        css = g_strdup_printf("\
            .progressbar.horizontal trough { min-height: 6px; }\
            .progressbar.horizontal progress { min-height: 6px; }\
            .progressbar.vertical trough { min-width: 6px; }\
            .progressbar.vertical progress { min-width: 6px; }");
    #endif
    #endif

    css_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (css_provider, css, strlen(css), NULL);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wTitle))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wImage))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);  
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wButton))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wValue))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);    
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wValButton))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);                
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wBar))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);  
    g_free(css);
    g_free(default_font);

    return poPlugin;
}/* genmon_create_control() */

/**************************************************************/

static void genmon_free (XfcePanelPlugin *plugin, genmon_t *poPlugin)
/* Plugin API */
{
    TRACE ("genmon_free()\n");

    if (poPlugin->iTimerId)
        g_source_remove (poPlugin->iTimerId);

    g_free (poPlugin->oConf.oParam.acCmd);
    g_free (poPlugin->oConf.oParam.acTitle);
    g_free (poPlugin->oConf.oParam.acFont);
    g_free (poPlugin->oMonitor.onClickCmd);
    g_free (poPlugin->acValue);
    g_free (poPlugin);
}/* genmon_free() */

/**************************************************************/

static int SetMonitorFont (void *p_pvPlugin)
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    
#if GTK_CHECK_VERSION (3, 16, 0)
    GtkCssProvider *css_provider;
    gchar * css;
#if GTK_CHECK_VERSION (3, 20, 0)
  PangoFontDescription *font;
  font = pango_font_description_from_string(poConf->acFont);
  if (G_LIKELY (font))
  {
    css = g_strdup_printf("label { font-family: %s; font-size: %dpx; font-style: %s; font-weight: %s }",
                          pango_font_description_get_family (font),
                          pango_font_description_get_size (font) / PANGO_SCALE,
                          (pango_font_description_get_style(font) == PANGO_STYLE_ITALIC ||
                           pango_font_description_get_style(font) == PANGO_STYLE_OBLIQUE) ? "italic" : "normal",
                          (pango_font_description_get_weight(font) >= PANGO_WEIGHT_BOLD) ? "bold" : "normal");
    pango_font_description_free (font);
  }
  else
    css = g_strdup_printf("label { font: %s; }", 
#else
    css = g_strdup_printf(".label { font: %s; }", 
#endif
                                    poConf->acFont);                        
    /* Setup Gtk style */
    DBG("css: %s",css);
    
    css_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_data (css_provider, css, strlen(css), NULL);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wTitle))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wValue))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_provider (
        GTK_STYLE_CONTEXT (gtk_widget_get_style_context (GTK_WIDGET (poMonitor->wValButtonLabel))),
        GTK_STYLE_PROVIDER (css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_free(css);
#else
    
    PangoFontDescription *poFont;

    if (!strcmp (poConf->acFont, "(default)")) /* Default font */
        return (1);
    poFont = pango_font_description_from_string (poConf->acFont);
    if (!poFont)
        return (-1);
        
    gtk_widget_override_font (poMonitor->wTitle, poFont);
    gtk_widget_override_font (poMonitor->wValue, poFont);
    gtk_widget_override_font (poMonitor->wValButton, poFont);
    
    pango_font_description_free (poFont);
    
#endif

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
    const char     *pc;
    char           *file;
    XfceRc         *rc;

    if (!(file = xfce_panel_plugin_lookup_rc_file (plugin)))
        return;

    rc = xfce_rc_simple_open (file, TRUE);
    g_free (file);

    if (!rc)
        return;

    if ((pc = xfce_rc_read_entry (rc, (CONF_CMD), NULL))) {
        g_free (poConf->acCmd);
        poConf->acCmd = g_strdup (pc);
    }

    poConf->fTitleDisplayed = xfce_rc_read_int_entry (rc, (CONF_USE_LABEL), 1);
    if (poConf->fTitleDisplayed)
        gtk_widget_show (GTK_WIDGET (poMonitor->wTitle));
    else
        gtk_widget_hide (GTK_WIDGET (poMonitor->wTitle));

    if ((pc = xfce_rc_read_entry (rc, (CONF_LABEL_TEXT), NULL))) {
        g_free (poConf->acTitle);
        poConf->acTitle = g_strdup (pc);
        gtk_label_set_text (GTK_LABEL (poMonitor->wTitle),
                            poConf->acTitle);
    }

    poConf->iPeriod_ms =
        xfce_rc_read_int_entry (rc, (CONF_UPDATE_PERIOD), 30 * 1000);

    if ((pc = xfce_rc_read_entry (rc, (CONF_FONT), NULL))) {
        g_free (poConf->acFont);
        poConf->acFont = g_strdup (pc);
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

static void SetCmd (GtkWidget *p_wTF, void *p_pvPlugin)
/* GUI callback setting the command to be spawn, whose output will
   be displayed using the panel-docked monitor */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    const char     *pcCmd = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    g_free (poConf->acCmd);
    poConf->acCmd = g_strdup (pcCmd);
}/* SetCmd() */

/**************************************************************/

static void ToggleTitle (GtkWidget *p_w, void *p_pvPlugin)
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

static void SetLabel (GtkWidget *p_wTF, void *p_pvPlugin)
/* GUI callback setting the legend of the monitor */
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);
    const char     *acTitle = gtk_entry_get_text (GTK_ENTRY (p_wTF));

    g_free (poConf->acTitle);
    poConf->acTitle = g_strdup (acTitle);
    gtk_label_set_text (GTK_LABEL (poMonitor->wTitle), poConf->acTitle);
}/* SetLabel() */

/**************************************************************/

static void SetPeriod (GtkWidget *p_wSc, void *p_pvPlugin)
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

static void About (XfcePanelPlugin *plugin)
{ 
  GdkPixbuf *icon;

  const gchar *auth[] =
    {
      "Roger Seguin <roger_seguin@msn.com>",
      "Julien Devemy <jujucece@gmail.com>",
      "Tony Paulic <tony.paulic@gmail.com>",
      NULL
    };

  icon = xfce_panel_pixbuf_from_source ("utilities-system-monitor", NULL, 32);
  gtk_show_about_dialog (NULL,
                         "logo",         icon,
                         "license",      xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
                         "version",      VERSION,
                         "program-name", PACKAGE,
                         "comments",     _("Cyclically spawns a script/program, captures its output and displays the resulting string in the panel"),
                         "website",      "http://goodies.xfce.org/projects/panel-plugins/xfce4-genmon-plugin",
                         "copyright",    _("Copyright \xc2\xa9 2004 Roger Seguin\nCopyright \xc2\xa9 2006 Julien Devemy\nCopyright \xc2\xa9 2016 Tony Paulic\n"),
                         "authors",      auth,
                         NULL);

  if (icon)
    g_object_unref (G_OBJECT (icon));
}

/**************************************************************/

static void ChooseFont (GtkWidget *p_wPB, void *p_pvPlugin)
{
    struct genmon_t *poPlugin = (genmon_t *) p_pvPlugin;
    struct param_t *poConf = &(poPlugin->oConf.oParam);
    GtkWidget      *wDialog;
    const char     *pcFont;
    int             iResponse;

    wDialog = gtk_font_chooser_dialog_new (_("Font Selection"),
        GTK_WINDOW(gtk_widget_get_toplevel(p_wPB)));
    gtk_window_set_transient_for (GTK_WINDOW (wDialog),
        GTK_WINDOW (poPlugin->oConf.wTopLevel));
    if (strcmp (poConf->acFont, "(default)")) /* Default font */
        gtk_font_chooser_set_font (GTK_FONT_CHOOSER (wDialog), poConf->acFont);
    iResponse = gtk_dialog_run (GTK_DIALOG (wDialog));
    if (iResponse == GTK_RESPONSE_OK) {
        pcFont = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (wDialog));
        if (pcFont) {
            g_free (poConf->acFont);
            poConf->acFont = g_strdup (pcFont);
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

    TRACE ("genmon_create_options()\n");

    xfce_panel_plugin_block_menu (plugin);

    dlg = xfce_titled_dialog_new_with_buttons (_("Configuration"),
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "gtk-close", GTK_RESPONSE_OK,
        NULL);

    g_signal_connect (dlg, "response", G_CALLBACK (genmon_dialog_response),
        poPlugin);

    xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG (dlg), _("Generic Monitor"));

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, BORDER + 6);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BORDER + 4);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(dlg))), vbox, 
        TRUE, TRUE, 0);

    poPlugin->oConf.wTopLevel = dlg;

    (void) genmon_CreateConfigGUI (GTK_WIDGET (vbox), poGUI);

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

    if (strcmp (poConf->acFont, "(default)")) /* Default font */
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
    struct monitor_t *poMonitor = &(poPlugin->oMonitor);

    gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wBox), p_iOrientation);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wImgBox), p_iOrientation);

    if (p_iOrientation == GTK_ORIENTATION_HORIZONTAL) {
        gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wBar), GTK_ORIENTATION_VERTICAL);
        gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(poMonitor->wBar), TRUE);
    }
    else {
        gtk_orientable_set_orientation(GTK_ORIENTABLE(poMonitor->wBar), GTK_ORIENTATION_HORIZONTAL);
        gtk_progress_bar_set_inverted(GTK_PROGRESS_BAR(poMonitor->wBar), FALSE);
    }

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
// call: xfce4-panel --plugin-event=genmon-X:refresh:bool:true
//    where genmon-X is the genmon widget id (e.g. genmon-7)

static gboolean genmon_remote_event (XfcePanelPlugin *plugin,
                                    const gchar *name,
                                    const GValue *value,
                                    genmon_t *genmon)
{
    g_return_val_if_fail (value == NULL || G_IS_VALUE (value), FALSE);
    if (strcmp (name, "refresh") == 0)
        {
        if (value != NULL
            && G_VALUE_HOLDS_BOOLEAN (value)
            && g_value_get_boolean (value))
            {
            /* update the display */
            DisplayCmdOutput (genmon);
            }
        return TRUE;
        }

    return FALSE;
}/* genmon_remote_event() */


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
    
    g_signal_connect (plugin, "about", G_CALLBACK (About), plugin);
    
    xfce_panel_plugin_menu_show_configure (plugin);
    g_signal_connect (plugin, "configure-plugin",
        G_CALLBACK (genmon_create_options), genmon);

    g_signal_connect (plugin, "remote-event", G_CALLBACK (genmon_remote_event), genmon);

    g_signal_connect (G_OBJECT (genmon->oMonitor.wButton), "clicked",
        G_CALLBACK (ExecOnClickCmd), genmon);

    g_signal_connect (G_OBJECT (genmon->oMonitor.wValButton), "clicked",
        G_CALLBACK (ExecOnValClickCmd), genmon);        
}

XFCE_PANEL_PLUGIN_REGISTER (genmon_construct)
