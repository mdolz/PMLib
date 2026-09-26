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

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <boost/asio.hpp>
#include <boost/optional.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

/// @file server.hpp
/// @brief TCP server, wire protocol opcodes and the raw send()/receive() codec
///        shared by every message exchanged between clients and pmlib_server.

namespace PMLib
{
    /// Wire-protocol opcode sent as the first word of every client request;
    /// dispatched by Server::connection() to either Counter (CREATE) or
    /// Info (the *_DEVICE / LIST_DEVICES / CMD_STATUS ops).
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

    /// Lifecycle state of a Counter: whether it is currently between a
    /// start()/restart() and the matching stop().
    enum class State {
        INACTIVE,
        ACTIVE
    };

    /// Status code returned to the client after each operation.
    /// ERRORF additionally carries the device's maximum frequency (see
    /// Info::read_device()) so the client can retry at a valid rate.
    enum class Retval {
        SUCCESS,
        ERROR = -1,
        ERRORF = -2,
    };

    /// Whether a Device aggregates several physical outlets under one
    /// server-side entry (MULTIPLE, e.g. a PDU-style meter) or represents a
    /// single measurement line (SINGLE).
    enum class Devtype {
        SINGLE,
        MULTIPLE
    };

    class Counter;
    class Device;

    typedef boost::shared_ptr<tcp::socket> socket_ptr;
    typedef shared_ptr<Counter> counter_ptr;
    typedef shared_ptr<Device> device_ptr;

    /// Owns the listening TCP socket and the set of configured devices.
    ///
    /// One Server is created per pmlib_server process from a JSON config file
    /// (see parse_configfile()); it spawns devices' sampling threads
    /// (start_devices()), accepts client connections and hands each one off
    /// to a dedicated thread (connection()) for the lifetime of that client's
    /// Counter or Info request.
    class Server {
        string _IP;
        short _port;

        io_service* _io_service;
        tcp::acceptor _acceptor;
        bool _daemonize;

      public:
        map<string, device_ptr> device_map;
        vector<counter_ptr> counters;

        Server(boost::asio::io_service* ios, string configfile, bool daemonize);
        Server(const Server& s);

        /// Looks up a configured device by name, or nullptr if unknown.
        device_ptr get_device(string name) const;
        /// Parses the JSON config file: server IP/port, logging, computers
        /// and devices/lines, populating device_map.
        void parse_configfile(string configfile);
        /// Starts every configured device's sampling thread and blocks (up to
        /// 15s each) until it reports itself working, or aborts the process.
        void start_devices();
        /// Binds/listens the acceptor, optionally daemonizes, then runs the
        /// io_service event loop until a SIGINT/SIGTERM is received.
        void start_connection();
        /// Full server lifecycle: start devices, accept connections until
        /// shutdown, then stop connection and devices in order.
        void run();
        /// Signals every device to stop sampling and clears device_map,
        /// which destroys the Device objects.
        void stop_devices();
        void stop_connection();
        /// Queues the next asynchronous accept on the acceptor.
        void async_accept();
        void accept_handler(socket_ptr _sock, const boost::system::error_code& e);
        /// Per-connection handler run on its own thread: reads the initial
        /// Operation opcode and dispatches to a new Counter or Info instance.
        void connection(socket_ptr sock);
    };

    // ------------------------------------------------------------------
    // Minimal binary wire codec used by Server/Counter/Info to exchange
    // fixed-size values, vectors and length-prefixed strings with clients.
    // Every send() has a matching receive() overload with the same layout.
    // ------------------------------------------------------------------

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
