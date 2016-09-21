/*
 * counter.cpp
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

#include <ctime>
#include <iostream>
#include <string>

#include "server.hpp"
#include "device.hpp"
#include "counter.hpp"
#include "info.hpp"
#include "utils/logger.hpp"

namespace PMLib {

Info::Info(socket_ptr sock, Operation op, Server* server) :
    _sock{sock}, _server{server} {
    run(op);
}

void Info::list_devices() {
    for (auto &dev : _server->device_map)
        send(_sock, dev.second->get_name());
}

void Info::info_device() {
    string device_name;
    receive(_sock, device_name);
    device_ptr dev = _server->get_device(device_name);
    if ( !dev ) {
        CLOG_ERROR << "Device " << device_name << " could not be found!" << endl;
        send<int>(_sock, (int)Retval::ERROR);
        return;
    }
    send<int>(_sock, dev->get_max_freq() );
    send<int>(_sock, dev->get_num_lines() );       
    send(_sock, dev->get_name() );
}

void Info::cmd_status() {
    send<int>(_sock, _server->device_map.size());
    for (auto &dev : _server->device_map) {
        send(_sock, dev.second->get_name());
        send<int>(_sock, dev.second->get_max_freq());
        send<int>(_sock, dev.second->get_num_lines());

        if ( dev.second->is_pdu() ) {
            send<int>(_sock, (int)Devtype::MULTIPLE);
        } else {
            send<int>(_sock, (int)Devtype::SINGLE);
            send(_sock, dev.second->computer._name);
        }

        for (auto &lin : dev.second->get_lines()) {
            send<int>(_sock, lin.second->_id);
            send(_sock, lin.second->_name);
            send(_sock, lin.second->_voltage);
            send(_sock, lin.second->_description);

            if ( dev.second->is_pdu() )
                send(_sock, lin.second->_computer._name);
        }

        send<int>(_sock, dev.second->counter_map.size());
        for (auto &cnt : dev.second->counter_map ) {    
            send<int>(_sock, cnt.second->get_id());
            send(_sock, cnt.second->get_client_ip());
            send(_sock, cnt.second->get_lines() );
        }
    }
}

void Info::read_device() {
    int frequency, max_frequency; //, ack;
    string device_name;
    receive(_sock, device_name);
    receive(_sock, frequency);
 
    device_ptr dev = _server->get_device(device_name);
    if ( !dev ) {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Device " << device_name << " could not be found!" << endl;        
        return;
    }

    max_frequency = dev->get_max_freq();
    if ( frequency > max_frequency ) {
        send<int>(_sock, (int)Retval::ERRORF);
        send(_sock, max_frequency);
        CLOG_ERROR << "Frequency requested is too high for the device!" << endl;
        return;
    }

    send<int>(_sock, (int)Retval::SUCCESS);
    float sleep_time = 1e6 / ((frequency > 0) ? frequency : max_frequency);

    try {    
        while ( true ) {
            send(_sock, dev->get_sample() );
            this_thread::sleep_for(chrono::microseconds((int)sleep_time));
        }
    } 
    catch(exception& e) {}
}

void Info::run(Operation op) {
    try {
        switch ( op ) {
            case Operation::LIST_DEVICES: list_devices(); break;
            case Operation::INFO_DEVICE:  info_device();  break;
            case Operation::CMD_STATUS:   cmd_status();   break;   
            case Operation::READ_DEVICE:  read_device();  break;
            default:
                send<int>(_sock, (int)Retval::ERROR);
                CLOG_ERROR << "Message type could not be recognized!" << endl;                
                break;
        }
    } 
    catch(exception& e) {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Error in run() of Info class: "  << e.what() << endl;
    }
}

// end namespace PMLib
}
