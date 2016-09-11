/*
 * daemonize.hpp
 *
 * Created on: 27/05/2016
 *
 * =========================================================================
 *  Copyright (C) 2016-, Manuel F. Dolz (maneldz@gmail.com)
 *
 *  This file is part of PMLib.
 *
 *  PMLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  PMLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this PMLib.  If not, see <http://www.gnu.org/licenses/>.
 *
 * =========================================================================
*/

#include <ostream>

#ifndef DAEMONIZE_HPP
#define DAEMONIZE_HPP

namespace PMLib {

    void signal_handler(int sig) {
        CLOG_WARN << "Unhandled signal: " << sig;
         
        //switch(sig)
        //{
         //   case SIGHUP:
         //       CLOG_WARN << "Received SIGHUP signal";
         //       break;
         //   case SIGINT:
         //   case SIGTERM:
         //       exit(EXIT_SUCCESS);
         //       break;
        //    default:
        //}
    }
 
    void daemonize(char *rundir, char *pidfile)
    {
        int pid, sid, i;
        char str[10];
        struct sigaction newSigAction;
        sigset_t newSigSet;
         
        /* Check if parent process id is set */
        if (getppid() == 1) return;
 
        /* Set signal mask - signals we want to block */
        sigemptyset(&newSigSet);
        sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
        sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
        sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
        sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
        sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */
        
        /* Set up a signal handler */
        newSigAction.sa_handler = signal_handler;
        sigemptyset(&newSigAction.sa_mask);
        newSigAction.sa_flags = 0;

        sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
        //sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
        sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */
 
        pid = fork();
        if ( pid < 0 ) exit(-1);
        if ( pid > 0 ) exit(0);
         
        umask(027); /* Set file permissions 750 */
         
        if (setsid() < 0) exit(EXIT_FAILURE);
         
        for (i = getdtablesize(); i >= 0; --i)
            close(i);
         
        i = open("/dev/null", O_RDWR);
        dup(i);
        dup(i);
         
        chdir(rundir);
         
        pidFilehandle = open(pidfile, O_RDWR|O_CREAT, 0600);
         
        if (pidFilehandle == -1 ) {
            CLOG_INFO << "Could not open PID lock file " << pidfile << ", exiting" << endl;
            exit(EXIT_FAILURE);
        }
         
        if (lockf(pidFilehandle,F_TLOCK,0) == -1) {
            CLOG_INFO << "Could not open PID lock file " << pidfile << ", exiting" << endl;
            exit(EXIT_FAILURE);
        }
        sprintf(str,"%d\n",getpid());
        write(pidFilehandle, str, strlen(str));
        close(pidFilehandle);
    }

}

#endif