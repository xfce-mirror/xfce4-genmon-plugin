/*
 *  Generic Monitor plugin for the Xfce4 panel
 *  Spawn - Spawn a process and capture its output
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

/* Posix-compliance to make sure that only the calling thread is
   duplicated, not the whole process (e.g Solaris) */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include "cmdspawn.h"

#include <errno.h>
#include <libxfce4ui/libxfce4ui.h>
#include <memory.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>


/**********************************************************************/
char *
genmon_Spawn (char **argv, int *wait_status, GError **error)
/**********************************************************************/
/* Spawn a command and capture its output from stdout or stderr */
/* Return allocated string on success, otherwise NULL */
{
    enum
    {
        OUT,
        ERR,
        OUT_ERR
    };
    enum
    {
        RD,
        WR,
        RD_WR
    };

    int aaiPipe[OUT_ERR][RD_WR];
    pid_t pid;
    struct pollfd aoPoll[OUT_ERR];
    int i, j, k;
    char *str = NULL;

    if (!(*argv))
    {
        g_set_error_literal (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED, g_strerror (EINVAL));
        fprintf (stderr, "Spawn() error: No parameters passed!\n");
        return (NULL);
    }
    for (i = 0; i < OUT_ERR; i++)
    {
        if (pipe (aaiPipe[i]) == -1)
        {
            g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
                         _("Failed to create pipe for communicating with child process (%s)"),
                         g_strerror (errno));
            perror ("pipe()");
            for (j = 0; j < i; j++)
            {
                close (aaiPipe[j][WR]); /* close write end of pipes in parent */
                close (aaiPipe[j][RD]); /* Close read end of pipes */
            }
            return NULL;
        }
    }

    switch (pid = fork ())
    {
        case -1:
            g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FORK,
                         _("Failed to fork (%s)"), g_strerror (errno));
            perror ("fork()");
            for (i = 0; i < OUT_ERR; i++)
                for (j = 0; j < RD_WR; j++)
                    close (aaiPipe[i][j]);
            return (NULL);
        case 0:
            /* Redirect stdout/stderr to associated pipe's write-ends */
            for (i = 0; i < OUT_ERR; i++)
            {
                j = i + 1; // stdout/stderr file descriptor
                k = dup2 (aaiPipe[i][WR], j);
                if (k != j)
                {
                    perror ("dup2()");
                    exit (-1);
                }
                /* Don't leak the open descriptor to potential long-lived grandchildren. */
                close (aaiPipe[i][WR]);
            }
            /* Execute the given command */
            execvp (argv[0], argv);
            perror (argv[0]);
            exit (-1);
    }

    for (i = 0; i < OUT_ERR; i++)
        close (aaiPipe[i][WR]); /* close write end of pipes in parent */

    /* Wait for child completion */
    if (waitpid (pid, wait_status, 0) == -1)
    {
        g_set_error (error, G_SPAWN_ERROR, G_SPAWN_ERROR_FAILED,
                     _("Unexpected error in waitpid() (%s)"), g_strerror (errno));
        perror ("waitpid()");
        goto End;
    }

    /* Read stdout/stderr pipes' read-ends */
    for (i = 0; i < OUT_ERR; i++)
    {
        aoPoll[i].fd = aaiPipe[i][RD];
        aoPoll[i].events = POLLIN;
        aoPoll[i].revents = 0;
    }
    poll (aoPoll, OUT_ERR, ~0);
    for (i = 0; i < OUT_ERR; i++)
    {
        if (aoPoll[i].revents & POLLIN)
            break;
    }
    if (i == OUT_ERR)
        goto End;

    j = 0;
    while (1)
    {
        str = g_realloc (str, j + 256);
        if ((k = read (aaiPipe[i][RD], str + j, 255)) > 0)
            j += k;
        else
            break;
    }
    str[j] = 0;

    /* Remove trailing carriage return if any */
    i = strlen (str) - 1;
    if (i >= 0 && str[i] == '\n')
        str[i] = 0;

End:
    /* Close read end of pipes */
    for (i = 0; i < OUT_ERR; i++)
        close (aaiPipe[i][RD]);

    return (str);
} // Spawn()


/**********************************************************************/
char *
genmon_SpawnCmd (const char *p_pcCmdLine, int *wait_status, GError **error)
/**********************************************************************/
/* Parse a command line, spawn the command, and capture its output from stdout or stderr */
/* Return allocated string on success, otherwise NULL */
{
    char **argv;
    int argc;
    GError *parse_error = NULL;
    char *str;

    /* Split the commandline into an argv array */
    if (!g_shell_parse_argv (p_pcCmdLine, &argc, &argv, &parse_error))
    {
        char *first = g_strdup_printf (_("Error in command \"%s\""), p_pcCmdLine);

        xfce_message_dialog (NULL, _("Xfce Panel"),
                             "dialog-error", first, parse_error->message,
                             "gtk-close", GTK_RESPONSE_OK, NULL);

        g_propagate_error (error, parse_error);
        g_free (first);
        return (NULL);
    }

    /* Spawn the command and free allocated memory */
    str = genmon_Spawn (argv, wait_status, error);
    g_strfreev (argv);

    return (str);
} // SpawnCmd()
