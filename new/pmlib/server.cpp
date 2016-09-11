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

#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <csignal>
#include <chrono> 
#include <fstream>
#include <iomanip>
 
#ifdef USE_STXXL  
    #include <stxxl/mng>
#endif

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

#include "server.hpp"
#include "device.hpp"
#include "counter.hpp"
#include "info.hpp" 
#include "utils/color.hpp"
#include "utils/logger.hpp"
#include "utils/json/json.h"
#include "devices/devices.hpp"

using namespace boost::asio::ip;
using namespace boost::log;
using namespace std;

namespace PMLib {

typedef sources::severity_logger_mt<trivial::severity_level> logger_t;  
BOOST_LOG_GLOBAL_LOGGER_INIT(lg, logger_t) {
    return logger_type();
}

Server::Server(boost::asio::io_service* ios, string configfile) : 
    _io_service{ios}, _acceptor{*ios} {
    parse_configfile(configfile);
}

device_ptr Server::get_device(string name) const {
    auto dev = device_map.find(name);
    return ( dev == device_map.end() ) ? nullptr : dev->second;
}

void Server::connection(socket_ptr _sock) {
    try {
        CLOG_INFO << "Connection, peer IP: " << _sock->remote_endpoint().address().to_string() << endl;

        Operation op;
        receive(_sock, op);

        if ( op == Operation::CREATE ) {
            Counter(_sock, this);
        }
        else if ( op == Operation::INFO_DEVICE || 
                  op == Operation::LIST_DEVICES || 
                  op == Operation::CMD_STATUS ||
                  op == Operation::READ_DEVICE ) {
            Info(_sock, op, this);
        }
        else {
            send<int>(_sock, (int)Retval::ERROR);
        }                
        _sock->close();
    } 
    catch (exception& e) {
        CLOG_ERROR << "Exception in thread: " << e.what() << endl;
    }
}

void Server::async_accept() {
    try {
        socket_ptr _sock = boost::make_shared<tcp::socket>(*_io_service);
        //_sock.reset(new tcp::socket(*_io_service));
        _acceptor.async_accept(*_sock, boost::bind(&Server::accept_handler, this, _sock, boost::asio::placeholders::error));
    } catch(exception& e) {
        CLOG_ERROR << "Error while accepting connection: " << e.what() << endl;
    }
}

void Server::accept_handler(socket_ptr _sock, const boost::system::error_code& e) {
    if ( !e ) {
        try {
            boost::thread(boost::bind( &Server::connection, this, _sock));
            //thread( &Server::connection, this, _sock);
        }
        catch(exception& e) {
            CLOG_ERROR << "Exception: " << e.what() << endl;
        }
    }
    async_accept();
}

void Server::start_devices() {
    cout << "Starting devices ... " << endl << flush; 
    for (auto &d : device_map) {
        cout << "Starting device " << blue << d.first << def << " ... " << setw(15-d.first.length()) << flush;
        try {
            d.second->start();

            unique_lock<mutex> lk(d.second->start_mutex);
            d.second->start_cv.wait_for(lk, chrono::seconds(15) );

            if ( d.second->is_working() )
                cout << green << "[  OK  ]" << def << endl << flush;
            else {
                d.second->stop();
                throw std::runtime_error("Device " + d.second->get_name() + " could not be started");
            }
        }        
        catch (std::exception& e) {
            cout << red << "[FAILED]" << def << endl;
            cerr << "Exception: " << e.what() << endl;
            exit(0);
        }
    }            
}

void Server::stop_devices() {
    cout << "Stopping devices ... " << endl << flush; 
    for (auto &d : device_map) {
        if ( d.second->is_working() ) {
            cout << "Stopping device " << blue << d.first << def << " ... " << setw(15-d.first.length()) << flush;
            try {
                d.second->stop();

                unique_lock<mutex> lk(d.second->stop_mutex);
                d.second->stop_cv.wait_for(lk, chrono::seconds(15) );

                if ( !d.second->is_running() )
                    cout << green << "[  OK  ]" << def << endl << flush;
                else
                    throw std::runtime_error("Device " + d.second->get_name() + " could not be stopped");
            }
            catch (std::exception& e) {
                cout << red << "[FAILED]" << def << endl;
                cerr << "Exception: " << e.what() << endl;
            }
        }
    }
    device_map.clear(); // Forces destruction of devices
}

void Server::start_connection() {
    cout << "Starting server ...  " << setw(15) << flush;
    try {
        tcp::resolver resolver(*_io_service);
        tcp::endpoint ep(address::from_string(_IP), _port); 

        boost::asio::signal_set signals(*_io_service, SIGINT, SIGTERM);
        signals.async_wait(boost::bind(&boost::asio::io_service::stop, _io_service));
        //_io_service->notify_fork(boost::asio::io_service::fork_prepare);

        _acceptor.open(ep.protocol());
        _acceptor.set_option(tcp::acceptor::reuse_address(true));
        _acceptor.set_option(boost::asio::socket_base::enable_connection_aborted(true));
        _acceptor.bind(ep);
        _acceptor.listen(5);

        async_accept();        
        cout << green << "[  OK  ]" << def << endl << flush;
        cout << "Server listening at (" << ep.address().to_string() << ", " << _port << ")" << endl;
        _io_service->run();
    } 
    catch(std::exception& e) {
        cout << red << "[FAILED]" << def << endl << flush;
        cerr << "Exception wile creating TCP socket on port "<< _port << ": " << e.what() << std::endl;
        stop_devices();
        exit(0);
    }
}

void Server::stop_connection() {
    cout << "Stopping server ...  " << setw(15);
    try {
        _acceptor.cancel();
        _acceptor.close();
        cout << green << "[  OK  ]" << def << endl;
    } 
    catch(exception& e) {
        cout << red << "[FAILED]" << def << endl;
        cerr << "Exception: " << e.what() << endl;
    }
}

void Server::parse_configfile(string config_filename) {
    try {
        ifstream configuration(config_filename, std::ifstream::binary);
        Json::Value conf;
        configuration >> conf;

#ifdef USE_STXXL        
        /* STXXL configuration */
        stxxl::config::get_instance()->add_disk(
            stxxl::disk_config(
                conf.get("stxxl_disk_name", "data.tmp").asString(), 
                conf.get("stxxl_disk_mb_size", 1024).asInt() * 1024 * 1024,
                conf.get("stxxl_disk_properties", "syscall delete autogrow direct=try").asString()
            )
        );
#endif
        /* Set IP and port server */
        _IP   = conf.get("server_ip", "0.0.0.0").asString();
        _port = conf.get("server_port", 6526).asInt();

        /* Set logging configuration */
        add_file_log
        (
            keywords::file_name = conf.get("server_logfile", "pmlib_%N.log").asString(),
            keywords::rotation_size = 10 * 1024 * 1024, // rotate files every 10 MiB
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), // rotation at midnight
            keywords::format =
            (
                expressions::stream << "["
                    << expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
                    << "] [" << trivial::severity
                    << "] " << expressions::smessage
            )
        );
        core::get()->set_filter
        (
            trivial::severity >= trivial::info
        );
        add_common_attributes();

        /* Set computers */
        map<string, Computer> computers_map;
        for( auto &c : conf["computers"].getMemberNames() ) {
            computers_map[c] = Computer(c, conf["computers"][c]["ip"].asString());
        }

        /* Set devices and lines */
        for( auto &dn : conf["devices"].getMemberNames() ) {
            auto &d = conf["devices"][dn];

            auto dev = Device::create_device( d["type"].asString(), 
                                              dn, 
                                              d["url"].asString() );

            auto &lines = d["lines"];
            if ( !dev ) {
                throw std::runtime_error("Error: Module of type " + d["type"].asString() + "for device " + dn + " is not currently supported!");
            }
            else if ( lines.isNull() ) {
                throw std::runtime_error("Error: Device " + dn + " has no lines defined!");
            }
            else if ( lines.getMemberNames().size() != dev->get_num_lines() ) {
                throw std::runtime_error("Error: Device " + dn +" requires exactly " + to_string(dev->get_num_lines()) + " lines, but " + to_string(lines.getMemberNames().size()) + " are defined!");
            } 
            /* No need to check for an already present device in device_map, JSON controls this! 
            else if ( !device_map.insert( make_pair( d, dev ) ).second ) {
                cerr << "Device " << d << " was defined more than once!" << endl;
                continue;
            }
            */
            dev->computer = ( !dev->is_pdu() && 
                            ( d["computer"].isNull() || 
                              computers_map.find( d["computer"].asString() ) == computers_map.end() ) ) ? 
                              Computer() : computers_map[ d["computer"].asString() ];

            vector<bool> check_lines(lines.getMemberNames().size(), false);

            for( auto &ln : lines.getMemberNames() ) {
                auto &l = conf["devices"][dn]["lines"][ln];

                if ( l["number"].asInt() >= 0 && 
                     l["number"].asInt() < lines.getMemberNames().size() && 
                     !check_lines[l["number"].asInt()] )
                    check_lines[l["number"].asInt()] = true;
                else if ( l["number"].asInt() >= 0 && 
                     l["number"].asInt() < lines.getMemberNames().size() && 
                     check_lines[l["number"].asInt()] )
                    throw std::runtime_error("Error: Line number " + to_string(l["number"].asInt()) + " of device " + dn + " is defined more than once!");
                else
                    throw std::runtime_error("Error: Line number " + to_string(l["number"].asInt()) + " of device " + dn + " should be between 0 and " + to_string(lines.getMemberNames().size()-1) + "!");

                Computer c = ( dev->is_pdu() && 
                             ( l["computer"].isNull() || 
                               computers_map.find( l["computer"].asString() ) == computers_map.end() ) ) ? 
                               Computer() : computers_map[ l["computer"].asString() ];

                dev->register_line( ln, 
                                    l["description"].asString(),
                                    l["number"].asInt(),
                                    c,
                                    l["voltage"].asFloat(), 
                                    l.get("offset", 0.0 ).asFloat(),
                                    l.get("slope",  0.0 ).asFloat() );
            }

            device_map[ dn ] = dev;
        } 
        cout << "Reading configuration file ...  " << setw(4) << flush;

        if ( device_map.empty() ) {
            throw std::runtime_error("Error: PMLib server could not start, no devices found!");
        }
        cout << green << "[  OK  ]" << def << endl;
    } 
    catch (exception& e) {
        cout << red << "[FAILED]" << def << endl;
        cerr << "Exception: " << e.what() << endl;
    }
}

void Server::run() {
    if ( device_map.empty() ) return;

    start_devices();    
    start_connection();

    stop_connection();
    stop_devices();
}

// end namespace PMLib
}
