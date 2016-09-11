/*
 * server.cpp
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

#include <iostream>
#include <string>
#include "server.hpp"
#include "device.hpp"
//#include "utils/logger.hpp"

//using namespace PMLib;

void printUsage(const std::string & progName){
    std::cerr << std::endl;
    std::cerr << "Usage: " << progName << " [--verbose] [--configfile]" << std::endl;
    std::cerr << "[--verbose] | Activates verbose logging, levels available." << std::endl;
    std::cerr << "[--configfile | TCP port used by the server to wait for remote requests." << std::endl;
}                                     \

int main(int argc, char** argv) {

    short int port = 6526;
    int all = 0;
    bool verbose; 
/*
    static struct option long_options[] = {
        {"verbose",   required_argument, &verbose,       'v'},
        {"tcpport",   required_argument, 0,              'p'},
        {0,           0,                 0,              0}
    };

    int long_index = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, argv,"v:p:",
                   long_options, &long_index )) != -1) {
        switch (opt) {
            case 0: {
                // If this option set a flag, do nothing else now. 
                if(long_options[long_index].flag != 0){
                    break;
                }else{
                    throw std::runtime_error("option " + std::string(long_options[long_index].name) +
                                             " with arg " + optarg + " without flags");
                }
            }
            break;
            case 'p': {
                tcpport = atoi(optarg);
            }
            break;
            case 'v': {
                verbose = atoi(optarg);
            }
            break;
            default: {
                printUsage(argv[0]);
                return -1;
            }
        }
    }
*/

    boost::asio::io_service ios;

    
    

    
    string configfile = "../settings-test.json";
    
    PMLib::Server s( &ios, configfile );

   // shared_ptr<PMLib::Device> dev1(new PMLib::Device("test", "/dev/usb0", 1, 1));
//    shared_ptr<PMLib::Device> dev2 = make_shared(new PMLib::Device("test", "/dev/usb0", 1));

    //s.register_device( dev1 );
    //s.register_device( make_shared<PMLib::Device>("prueba", "/dev/usb0", 1, 1) );    

    s.run();

/*
  try {

    if (pid_t pid = fork()) {
        if (pid > 0) {
            exit(0);
        }
        else {
            syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
            return 1;
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
            syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
            return 1;
        }
    }

    // Close the standard streams. This decouples the daemon from the terminal
    // that started it.
    close(0);
    close(1);
    close(2);

    if (open("/dev/null", O_RDONLY) < 0)
    {
        syslog(LOG_ERR | LOG_USER, "Unable to open /dev/null: %m");
        return 1;
    }

    // Send standard output to a log file.
    const char* output = "/tmp/asio.daemon.out";
    const int flags = O_WRONLY | O_CREAT | O_APPEND;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (open(output, flags, mode) < 0) {
        syslog(LOG_ERR | LOG_USER, "Unable to open output file %s: %m", output);
        return 1;
    }

    if (dup(1) < 0)
    {
        syslog(LOG_ERR | LOG_USER, "Unable to dup output descriptor: %m");
        return 1;
    }

    io_service.notify_fork(boost::asio::io_service::fork_child);

//    syslog(LOG_INFO | LOG_USER, "Daemon started");
    io_service.run();
//    syslog(LOG_INFO | LOG_USER, "Daemon stopped");
  }
  catch (std::exception& e)
  {
    syslog(LOG_ERR | LOG_USER, "Exception: %s", e.what());
    std::cerr << "Exception: " << e.what() << std::endl;
  }

*/


    return 0;
}

