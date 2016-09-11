/*
 * counter.hpp
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

#ifndef COUNTER_HPP
#define COUNTER_HPP

namespace PMLib
{
    typedef map<int, long long int> set_t;
    class Server;

    class Counter {

        socket_ptr _sock;
        bool running, aggregate;
        int interval;
        State status;
        unsigned int frequency;
        vector<int> lines;
        vector<pair<set_t, boost::optional<set_t>>> sets;
        set_t sizes;
        Server* _server;
        device_ptr dev;
        static int nextid;
        int id;
        bool registered;

      public:
        Counter(socket_ptr _sock, Server* server);
        ~Counter();

        int get_id() const { return id; }
        string get_client_ip() const { return _sock->remote_endpoint().address().to_string(); }
        const vector<int>& get_lines() const { return lines; }

        void start();
        void restart();
        void stop();
        void get();
        void finalize();
        void set_lines(string lines_str);
        void set_frequency(int frequency);
        //bool last_set_completed();
        void show_sets();

        void run();
    };
    
}

#endif
