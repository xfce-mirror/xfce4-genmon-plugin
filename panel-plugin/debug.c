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

static char     RCSid[] = "$Id: debug.c,v 1.1 2004/09/09 13:35:51 rogerms Exp $";


#include "debug.h"


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <inttypes.h>


int genmon_Trace (const char *const p_pcFormat, ...)
{
    const char     *const pcFile = TRACE_LOG;
    static FILE    *s_pF = 0;
    va_list         ap;

    if (!s_pF)
	s_pF = fopen (pcFile, "w");
    va_start (ap, p_pcFormat);
    vfprintf (s_pF, p_pcFormat, ap);
    va_end (ap);
    fflush (s_pF);
    return (0);
}


/*
$Log: debug.c,v $
Revision 1.1  2004/09/09 13:35:51  rogerms
Initial revision

Revision 1.1  2004/08/27 23:17:38  RogerSeguin
Initial revision

Revision 1.2  2003/09/23 15:18:57  RogerSeguin
Use vfprintf(3)

Revision 1.1  2003/09/22 02:25:33  RogerSeguin
Initial revision

*/
