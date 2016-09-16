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
#include <fstream>
#include <getopt.h>
#include "server.hpp"

void printUsage(const std::string & progName){
    std::cerr << std::endl;
    std::cerr << "Usage: " << progName << " [--verbose] [--configfile]" << std::endl;
    std::cerr << "[--daemonize] | Runs PMLib server as a daemon." << std::endl;
    std::cerr << "[--configfile | JSON PMLib configuration file." << std::endl;
}                                     

int main(int argc, char *argv[]) {

    int all = 0;
    int daemonize = false; //verbose;
    string configfile = "";

    static struct option long_options[] = {
        {"damonize",     no_argument,       0,            'd'},
        {"configfile",   required_argument, 0,            'c'},
        {0,              0,                 0,              0}
    };

    int index = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, argv,"dc:", long_options, &index )) != -1) {
        switch (opt) {
            case 0:
                if (long_options[index].flag != 0)
                    break;
                cerr << "Error: option " << long_options[index].name;
                if (opt)
                    cerr << " with arg " << optarg;
                cerr << endl;
                break;
            case 'd': 
                daemonize = true;
                break;
            case 'c':
                configfile = string(optarg);            
                break;
            default: {
                printUsage(argv[0]);
                return -1;
            }
        }
    }

    if ( configfile != "" ) {
        ifstream file(configfile);
        if( ! file.fail() ) {
            boost::asio::io_service ios{};
            PMLib::Server( &ios, configfile, daemonize ).run();           
        } 
        else {
            cerr << "Configuration file \"" << configfile << "\" not found!" << endl;
            return -1;
        }
    }
    else {
        printUsage(argv[0]);
    }

    return 0;
}

