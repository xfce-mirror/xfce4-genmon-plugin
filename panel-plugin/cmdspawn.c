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
static gboolean
ReadChunk (int fd, GString *buf)
/**********************************************************************/
/* Append whatever is available on fd to buf */
/* Return FALSE on end-of-file or error */
{
    char chunk[4096];
    ssize_t n;

    do
        n = read (fd, chunk, sizeof (chunk));
    while (n == -1 && errno == EINTR);

    if (n <= 0)
        return FALSE;

    g_string_append_len (buf, chunk, n);
    return TRUE;
} // ReadChunk()


/**********************************************************************/
char *
genmon_Spawn (char **argv, int wait)
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
    GString *aoBuf[OUT_ERR];
    int nOpen;
    int i, j;
    size_t len;
    char *str = NULL;

    if (!(*argv))
    {
        fprintf (stderr, "Spawn() error: No parameters passed!\n");
        return (NULL);
    }
    for (i = 0; i < OUT_ERR; i++)
    {
        if (pipe (aaiPipe[i]) == -1)
        {
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
                if (dup2 (aaiPipe[i][WR], j) != j)
                {
                    perror ("dup2()");
                    _exit (127);
                }
            /* Don't leak the pipe descriptors to the command or to
               potential long-lived grandchildren. */
            for (i = 0; i < OUT_ERR; i++)
                for (j = 0; j < RD_WR; j++)
                    if (aaiPipe[i][j] > STDERR_FILENO)
                        close (aaiPipe[i][j]);
            }
            /* Execute the given command */
            execvp (argv[0], argv);
            perror (argv[0]);
            _exit (127);
    }

    for (i = 0; i < OUT_ERR; i++)
        close (aaiPipe[i][WR]); /* close write end of pipes in parent */

    /* Wait for child completion */
    if (wait == 1)
    {
        /* Drain both pipes until EOF *before* reaping the child: a child
           writing more than a pipe buffer would otherwise block forever
           while we block in waitpid() */
        for (i = 0; i < OUT_ERR; i++)
        {
            aoBuf[i] = g_string_new (NULL);
            aoPoll[i].fd = aaiPipe[i][RD];
            aoPoll[i].events = POLLIN;
        }
        nOpen = OUT_ERR;
        while (nOpen > 0)
        {
            if (poll (aoPoll, OUT_ERR, -1) == -1)
            {
                if (errno == EINTR)
                    continue;
                perror ("poll()");
                break;
            }
            for (i = 0; i < OUT_ERR; i++)
            {
                if (aoPoll[i].fd < 0 || aoPoll[i].revents == 0)
                    continue;
                if (!ReadChunk (aoPoll[i].fd, aoBuf[i]))
                {
                    aoPoll[i].fd = -1; /* ignored by poll() from now on */
                    nOpen--;
                }
            }
        }
        while (waitpid (pid, NULL, 0) == -1 && errno == EINTR)
            ;

        /* Prefer stdout; fall back to stderr; NULL if both are empty */
        for (i = 0; i < OUT_ERR; i++)
            if (aoBuf[i]->len > 0)
                break;
        if (i < OUT_ERR)
        {
            /* Remove trailing carriage return if any */
            len = aoBuf[i]->len;
            if (aoBuf[i]->str[len - 1] == '\n')
                g_string_truncate (aoBuf[i], len - 1);
            str = g_string_free (aoBuf[i], FALSE);
            aoBuf[i] = NULL;
        }
        for (i = 0; i < OUT_ERR; i++)
            if (aoBuf[i])
                g_string_free (aoBuf[i], TRUE);
    }

    /* Close read end of pipes */
    for (i = 0; i < OUT_ERR; i++)
        close (aaiPipe[i][RD]);

    return (str);
} // Spawn()


/**********************************************************************/
char *
genmon_SpawnCmd (const char *p_pcCmdLine, int wait)
/**********************************************************************/
/* Parse a command line, spawn the command, and capture its output from stdout or stderr */
/* Return allocated string on success, otherwise NULL */
{
    char **argv;
    int argc;
    GError *error = NULL;
    char *str;

    /* Split the commandline into an argv array */
    if (!g_shell_parse_argv (p_pcCmdLine, &argc, &argv, &error))
    {
        char *first = g_strdup_printf (_("Error in command \"%s\""), p_pcCmdLine);

        xfce_message_dialog (NULL, _("Xfce Panel"),
                             "dialog-error", first, error->message,
                             "gtk-close", GTK_RESPONSE_OK, NULL);

        g_error_free (error);
        g_free (first);
        return (NULL);
    }

    /* Spawn the command and free allocated memory */
    str = genmon_Spawn (argv, wait);
    g_strfreev (argv);

    return (str);
} // SpawnCmd()
