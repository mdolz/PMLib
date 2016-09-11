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
    int daemonize; //verbose;
    string configfile;

    static struct option options[] = {
        {"damonize",     required_argument, 0,            'd'},
        {"configfile",   required_argument, 0,            'c'},
        {0,              0,                 0,              0}
    };

    int index = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, argv,"d:c:", options, &index )) != -1) {
        switch (opt) {
            case 0:
                if(options[index].flag != 0){
                    break;
                }else{
                   // throw std::runtime_error("option " + std::string(options[index].name) +
                   //                          " with arg " + optarg + " without flags");
                }
                break;
            case 'd': 
                daemonize = true;
                break;
            case 'v':
                configfile = string(optarg);            
                break;
            default: {
                printUsage(argv[0]);
                return -1;
            }
        }
    }

    boost::asio::io_service ios{};
//    string configfile = "../settings-test.json";
   
    PMLib::Server( &ios, configfile, daemonize ).run();
//    s.run();

    return 0;
}

