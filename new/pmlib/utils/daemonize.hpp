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

#ifndef DAEMONIZE_HPP
#define DAEMONIZE_HPP

#include <iostream>
#include <ostream>
#include <fstream> 
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include "logger.hpp"

using namespace std;
using namespace PMLib;

namespace PMLib {

    void signal_handler(int sig) {
        CLOG_WARN << "Unhandled signal: " << sig << endl;
    }

    void write_pidfile(string pid_filename) {
        ofstream pidfile;
        pidfile.open(pid_filename, std::ofstream::out | std::ofstream::trunc);
        
        if ( !pidfile.is_open() ) {
            CLOG_ERROR << "Could not open PID lock file " << pid_filename << ", exiting" << endl;
            cerr << "Could not open PID lock file " << pid_filename << ", exiting" << endl;
            exit(-1);
        }
        else {
            pidfile << getpid() << endl;
            pidfile.close();
        }
    }    
 
    int daemonize(string pidfile)
    {
        try {

            if (getppid() == 1) return -1;

            struct sigaction newSigAction;
            sigset_t newSigSet;

            /* Set signal mask - signals we want to block */
            sigemptyset(&newSigSet);
            sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
            sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
            sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
            sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
            sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

             /* Set up a signal handler */
            newSigAction.sa_handler = PMLib::signal_handler;
            sigemptyset(&newSigAction.sa_mask);
            newSigAction.sa_flags = 0;

            sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
            //sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
            //sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */

            if (pid_t pid = fork()) {
                if (pid > 0) {
                    exit(0);
                }
                else {
                    CLOG_ERROR << "Daemonize: first fork failed" << endl;
                    return -1;
                }
            }

            setsid();
            chdir("/");
            umask(0);

            if (pid_t pid = fork()) {
                if (pid > 0) {
                    exit(0);
                }
                else {
                    CLOG_ERROR << "Daemonize: second fork failed" << endl;
                    return -1;
                }
            }

            // Close the standard streams. This decouples the daemon from the terminal
            // that started it.
            close(0);
            close(1);
            close(2);

            if (open("/dev/null", O_RDONLY) < 0) {
                CLOG_ERROR << "Daemonize: unable to open /dev/null" << endl;
                return -1;
            }

            // Send standard output to a log file.
            const char *output = "/var/log/pmlib.out";
            const int flags = O_WRONLY | O_CREAT | O_APPEND;
            const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

            if (open(output, flags, mode) < 0) {
                CLOG_ERROR << "Daemonize: unable to open output file" << endl;
                return -1;
            }

            if (dup(1) < 0) {
               CLOG_ERROR << "Daemonize: unable to dup output descriptor" << endl;
                return -1;
            }

            write_pidfile(pidfile);

            CLOG_INFO << "PMLib daemon started" << endl;

        }
        catch (std::exception& e) {
            CLOG_ERROR << "Exception: %s" << e.what() << endl;;
            cerr << "Exception: " << e.what() << endl;
        } 
        return 0;      
    }
}

#endif