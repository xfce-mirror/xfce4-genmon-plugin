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

#ifndef _config_gui_h
#define _config_gui_h
static char     _config_gui_h_id[] = "$Id: config_gui.h,v 1.1.1.2 2004/11/01 00:22:46 rogerms Exp $";

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/i18n.h>
#include <libxfcegui4/dialogs.h>
#include <panel/plugins.h>
#include <panel/xfce.h>


typedef struct gui_t {
    /* Configuration GUI widgets */
    GtkWidget      *wPB_About;
    GtkWidget      *wTF_Cmd;
    GtkWidget      *wTB_Title;
    GtkWidget      *wTF_Title;
    GtkWidget      *wSc_Period;
    GtkWidget      *wPB_Font;
} gui_t;


#ifdef __cplusplus
extern          "C" {
#endif

    int             genmon_CreateConfigGUI (GtkWidget * ParentWindow,
					    struct gui_t *gui);
    /* Create configuration/Option GUI */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}				/* extern "C" */
#endif
/*
$Log: config_gui.h,v $
Revision 1.1.1.2  2004/11/01 00:22:46  rogerms
*** empty log message ***

Revision 1.1.1.1  2004/09/09 13:35:51  rogerms
V1.0

Revision 1.2  2004/08/28 09:52:17  RogerSeguin
Added font selector

Revision 1.1  2004/08/27 23:16:58  RogerSeguin
Initial revision

*/
#endif				/* _config_gui_h */
