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
#include <cmath> 
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm> 

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

#include "server.hpp"
#include "device.hpp"
#include "counter.hpp"
#include "utils/logger.hpp"

namespace PMLib {

int Counter::nextid = 0;

Counter::Counter(socket_ptr sock, Server* server) : 
    _sock{sock}, running{true}, status{State::INACTIVE}, sets{}, _server{server}, registered{false} {
    try {
        id = ++nextid;
        string device_name, lines_mask;

        receive(_sock, device_name);
        receive(_sock, frequency);
        receive<bool,int>(_sock, aggregate);
        receive(_sock, lines_mask); 

        auto d = server->device_map.find(device_name);
        if ( d == server->device_map.end() )
            throw std::runtime_error("device " + device_name + " not found!");
        dev = server->device_map.at(device_name);

        set_frequency(frequency);
        set_lines(lines_mask);

        dev->register_counter(*this);
        registered = true;

        send<int>(_sock, (int)Retval::SUCCESS);

        CLOG_INFO << "Created new counter ID: " << id << " from " 
                  << _sock->remote_endpoint().address().to_string() 
                  << " requesting device " << device_name << endl;
    }
    catch (exception& e) {
        send<int>(_sock, (int)Retval::ERROR);        
        CLOG_ERROR << "Error creating new counter: " << e.what() << endl;
        return;
    }
    run();     
}

Counter::~Counter() {
    if ( registered )
        dev->deregister_counter(*this);
}

void Counter::show_sets() {
    int i = 0;
    for ( auto &ss : sets) {
        cout << "SET: " << i++ << " [ ";
        for (auto &s : ss.first )
            cout << s.first << ":" << s.second << " ";
        cout << "] - [ ";
        if ( ss.second.is_initialized() ) {
            for (auto &s : ss.second.get() )
                cout << s.first << ":" << s.second << " ";
            cout << "]" << endl;
        }
        else cout << "NULL ]" << endl; 
    }
}

void Counter::start() {  
    if ( status == State::INACTIVE ) {
        dev->start_counter(lines);
        status = State::ACTIVE;
        sizes.clear();
        dev->get_lines_sizes(lines, sizes);
        sets.clear();
        sets.push_back( make_pair( sizes, boost::optional<set_t>{} ) );
        //show_sets();
        send<int>(_sock, (int)Retval::SUCCESS);
    } 
    else {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Received <start> of an active counter with ID: " << id << endl;
    }
}

void Counter::restart() {
    if ( status == State::INACTIVE ) {
        dev->start_counter(lines);
        status = State::ACTIVE;
        sizes.clear();
        dev->get_lines_sizes(lines, sizes);
        sets.push_back( make_pair( sizes, boost::optional<set_t>{} ) );
        //show_sets();
        send<int>(_sock, (int)Retval::SUCCESS);
    } 
    else {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Received <restart> of an active counter with ID: " << id << endl;
    }
}

void Counter::stop() {
    if ( status == State::ACTIVE ) {    
        dev->stop_counter(lines);
        status = State::INACTIVE;        
        sizes.clear();
        dev->get_lines_sizes(lines, sizes);
        sets.back().second = sizes;
        //show_sets();
        send<int>(_sock, (int)Retval::SUCCESS);
    } 
    else {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Received <stop> of an inactive counter with ID: " << id << endl;
    }
}

void Counter::get() {
    if ( status == State::INACTIVE ) {
        
        vector<int> watts_sets(1, 0);
        vector<long long int> min_len(sets.size(), 0);
        vector<double> watts;
        int se_idx = 0;
        
        for ( auto &se : sets ) {
            for ( auto &l : lines ) {
                min_len[ se_idx ] = ( l == lines.front() ) ? 
                                    ceil( ( se.second.get()[l] - se.first[l] ) / interval) :
                                    min( (long long)ceil( ( ( se.second.get()[l] - se.first[l] ) / interval ) ), min_len[ se_idx ] );
            }
        
            watts_sets.push_back( min_len[se_idx] + watts_sets.back() );
            se_idx++;
        }
       // cout << "MIN_LEN: [ "; for(auto &i:min_len) cout << i << " "; cout << "]" << endl;
       // cout << "WATTS_SETS: [ "; for(auto &i:watts_sets) cout << i << " "; cout << "]" << endl;

        if ( aggregate ) {
            se_idx = 0;
            for ( auto &se : sets ) {
                vector<double> sum_line(min_len[se_idx], 0);
                for ( auto &l : lines ) {
                    const auto &line = dev->get_line_data(l);
                    int i = 0;
                    for( auto idx = line.begin() + se.first[l] ;
                              idx!= line.begin() + min( min(se.second.get()[l], se.first[l]+min_len[se_idx] ), (long long) line.size()) && i < min_len[se_idx];
                              idx+= interval ) {
                        sum_line [ i++ ] += *idx;
                    }
                }
                copy(sum_line.begin(), sum_line.end(), back_inserter(watts));
                se_idx++;
            }   
        } 
        else {
           // cout << "GET DATA" << endl;
            se_idx = 0;
            for ( auto &se : sets ) {
               // cout << "PROCESSING SET: " << se_idx << endl;
                for ( auto &l : lines ) {
                    const auto &line = dev->get_line_data(l);
                  //  cout << "  LINE " << l << " DATA: "; cout << line.size() << endl; //for ( auto &d:line) cout << d << " "; cout << endl;
                  //  cout << "    RANGE: [ " << se.first[l] << ", " << 
                  //  min( min(se.second.get()[l], se.first[l]+min_len[se_idx]), (long long) line.size()) << " ] "<<endl;

                    int before= watts.size();
                    int i = 0;
                    copy_if( line.begin() + se.first[l],
                             line.begin() + min( min(se.second.get()[l], se.first[l]+min_len[se_idx]), (long long) line.size()),
                             back_inserter(watts), 
                             [&i, this](int value){ return (i++ % interval) == 0; } );
                  //  cout << "  WATTS DATA: "; cout << watts.size()-before << endl; for ( auto &d:watts) cout << d << " "; cout << endl;
                }
             //   cout << endl;
                se_idx++;
            } 
        }
        send<int>(_sock, watts.size());
        send<int>(_sock, watts_sets.size());
        send<int>(_sock, lines.size());
        write(*_sock, buffer((char*)(&watts_sets.front()), sizeof(int)*watts_sets.size()));
        write(*_sock, buffer((char*)(&watts.front()), sizeof(double)*watts.size()));
    } 
    else {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Received <get> of an active counter with ID: " << id << endl;
    }        
}

void Counter::finalize() {
    if ( status == State::INACTIVE ) {
        running= false;
        send<int>(_sock, (int)Retval::SUCCESS);
    } 
    else {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Rewived <finalize> of an active counter or with an incomplete set!" << endl;
    }
}

/*
bool Counter::last_set_completed() {
    return sets.empty() ? true : sets.back().second.is_initialized();
}
*/

void Counter::set_lines(string lines_str) {
    lines.clear();
    for (int i = 0; i < lines_str.length(); i++) {
        for ( int b = 0; b < 8; b++ ) { 
            int l = i*b + b;
            if ( lines_str[i] & (1 << b) && 
                 l >= 0 && l < dev->get_num_lines() ) 
                lines.push_back( l );
        }
    }
}

void Counter::set_frequency(int frequency) {
    int max_frequency = dev->get_max_freq();
    if ( frequency == 0 ) 
        frequency= max_frequency;
    else if ( frequency > max_frequency )
        throw std::runtime_error("Device only works at " + to_string(max_frequency) + " Hz!");
    interval= round( (float) max_frequency / frequency );
}

void Counter::run() {
    Operation op;
    try {
        while ( running ) {
            receive(_sock, op);
            switch ( op ) {
                case Operation::START:     start();    break;
                case Operation::CONTINUE:  restart();  break;
                case Operation::STOP:      stop();     break;   
                case Operation::GET:       get();      break;
                case Operation::FINALIZE:  finalize(); break;
                default:
                    send<int>(_sock, (int)Retval::ERROR);
                    CLOG_ERROR << "Error in Counter::run(): message ID " << static_cast<int>(op) 
                               << " not recognized" << endl;
                    break;
            }
        }        
    } 
    catch(exception& e) {
        send<int>(_sock, (int)Retval::ERROR);
        CLOG_ERROR << "Error in Counter::run(): "  << e.what() << endl;
    }
}

// end namespace PMLib
}
