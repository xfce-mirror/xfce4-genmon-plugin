/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Spawn - Spawn a process and capture its output
 *  Copyright (c) 2004 Roger Seguin <roger_seguin@msn.com>
 *  					<http://rmlx.dyndns.org>
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

#ifndef _cmdspawn_h
#define _cmdspawn_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>



#ifdef __cplusplus
extern          "C" {
#endif
    int             genmon_Spawn (char *const argv[],
				  char *const StdOutCapture,
				  const size_t StdOutSize, const int wait);
    int             genmon_SpawnCmd (const char *const cmdline,
				     char *const StdOutCapture,
				     const size_t StdOutSize, const int wait);
    /* Spawn the given command and capture its output (stdout) */
    /* Return 0 on success, otherwise copy stderr into the output string
       and return -1 */
#ifdef __cplusplus
}				/* extern "C" */
#endif
/*
$Log: cmdspawn.h,v $
Revision 1.1.1.2  2004/11/01 00:22:46  rogerms
*** empty log message ***

Revision 1.1.1.1  2004/09/09 13:35:51  rogerms
V1.0

Revision 1.1  2004/08/27 23:16:48  RogerSeguin
Initial revision

*/
#endif				/* _cmdspawn_h */
