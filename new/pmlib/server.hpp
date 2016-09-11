/*
 * server.hpp
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

#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace PMLib
{
    enum class Operation { 
        CREATE,
        START,
        CONTINUE,
        STOP,
        GET,
        FINALIZE,
        INFO_DEVICE,
        LIST_DEVICES,
        CMD_STATUS,
        READ_DEVICE,
        ERROR = -1
    };

    enum class State {
        INACTIVE,
        ACTIVE
    };

    enum class Retval {
        SUCCESS,
        ERROR = -1,
        ERRORF = -2,
    };
    
    enum class Devtype {
        SINGLE,
        MULTIPLE
    };

    class Counter;
    class Device;

    typedef boost::shared_ptr<tcp::socket> socket_ptr;
    typedef shared_ptr<Counter> counter_ptr;
    typedef shared_ptr<Device> device_ptr;

    class Server {
        string _IP;
        short _port; 

        io_service* _io_service;
        tcp::acceptor _acceptor;

      public:
        map<string, device_ptr> device_map;
        vector<counter_ptr> counters;

        Server(boost::asio::io_service* ios, string configfile);
        Server(const Server& s);

        device_ptr get_device(string name) const;
        void parse_configfile(string configfile);
        void start_devices();
        void start_connection();
        void run();
        void stop_devices();
        void stop_connection();
        void async_accept();
        void accept_handler(socket_ptr _sock, const boost::system::error_code& e);
        void connection(socket_ptr sock);
    };

    template<typename T>
    inline size_t send(socket_ptr _sock, T const &data) {
        return write(*_sock, buffer((char*)(&data), sizeof(data))); 
    }

    template <typename T>
    inline size_t send(socket_ptr _sock, vector<T> const &data){
        size_t retval = send<int>(_sock, data.size());
        retval+= write(*_sock, buffer((char*)(&data.front()), sizeof(T)*data.size()));
        return retval;
    }

    inline size_t send(socket_ptr _sock, string const &str) {
        int len = str.length();
        size_t retval = write(*_sock, buffer(reinterpret_cast<char*>(&len), sizeof(int))); 
        retval+= write(*_sock, buffer(str.c_str(), sizeof(char)*str.size()));
        return retval;
    }

    template<typename T, typename R = T>
    inline size_t receive(socket_ptr _sock, T &data) {
        return read(*_sock, buffer(reinterpret_cast<char*>(&data), sizeof(R))); 
    }

    inline size_t receive(socket_ptr _sock, string &data) {
        int length;
        size_t retval = 0;
        receive(_sock, length);
        if ( length > 0 ) { 
            vector<char> str_(length);
            retval = read(*_sock, buffer(str_, sizeof(char)*length));
            data= string(str_.begin(), str_.end());
        }
        return retval;
    }
}

#endif
