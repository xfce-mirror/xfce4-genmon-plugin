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

#ifndef _debug_h
#define _debug_h
static char     _debug_h_id[] = "$Id: debug.h,v 1.1 2004/09/09 13:35:51 rogerms Exp $";

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#ifndef DEBUG
#define DEBUG	0
#endif


#define TRACE		if (!DEBUG) {} else genmon_Trace

#define TRACE_LOG	"/tmp/genmon.log"


#ifdef __cplusplus
extern          "C" {
#endif

    int             genmon_Trace (const char *const msg, ...);
    /* Write debug information in TRACE_LOG when DEBUG is set */
    /* Return 0 on success, -1 otherwise */

#ifdef __cplusplus
}				/* extern "C" */
#endif
/*
$Log: debug.h,v $
Revision 1.1  2004/09/09 13:35:51  rogerms
Initial revision

Revision 1.1  2004/08/27 23:17:07  RogerSeguin
Initial revision

*/
#endif				/* _debug_h */
