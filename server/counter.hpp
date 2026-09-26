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

/// @file counter.hpp
/// @brief A client-owned, named measurement session over a subset of a
///        Device's lines.

namespace PMLib
{
    /// Maps a line id to a sample-buffer offset (Device::get_line_data()
    /// index at the moment a set was opened/closed); used to delimit the
    /// slice of buffered samples that belongs to one start/stop cycle.
    typedef map<int, long long int> set_t;
    class Server;

    /// One client's measurement session against a single Device.
    ///
    /// A Counter is created for the lifetime of a client TCP connection
    /// (see Server::connection()) and owns its own thread of control
    /// (run()), driven by Operation opcodes read from the socket. Each
    /// start()/stop() pair records a "set": the [begin, end) offsets into
    /// the device's per-line sample buffers covering that interval, so a
    /// client can start/stop/restart multiple times and later fetch (get())
    /// the concatenation of all recorded sets in one call.
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
        /// Reads the counter's creation request (device name, frequency,
        /// aggregate flag, line bitmask) off @p sock, registers it with the
        /// requested device, replies with the result, then blocks in run()
        /// for the rest of the connection's lifetime.
        Counter(socket_ptr _sock, Server* server);
        ~Counter();

        int get_id() const { return id; }
        string get_client_ip() const { return _sock->remote_endpoint().address().to_string(); }
        const vector<int>& get_lines() const { return lines; }

        /// Begins a new set: activates the counter's lines on the device and
        /// records the current buffer offsets as the set's start.
        void start();
        /// Like start(), but appends a new set instead of erroring if one is
        /// already open; used by clients that bracket several sub-regions
        /// under the same counter.
        void restart();
        /// Closes the current set: deactivates the lines and records the
        /// current buffer offsets as the set's end.
        void stop();
        /// Replies with the concatenated (optionally line-aggregated)
        /// samples of every completed set, resampled to this counter's
        /// requested frequency.
        void get();
        /// Ends the counter's session, causing run() to return.
        void finalize();
        /// Decodes the line bitmask received from the client into the list
        /// of line ids this counter measures.
        void set_lines(string lines_str);
        /// Validates the requested frequency against the device's maximum
        /// and derives the device-sample stride (interval) needed to
        /// downsample to it.
        void set_frequency(int frequency);
        //bool last_set_completed();
        /// Debug helper: dumps the recorded sets to stdout.
        void show_sets();

        /// Reads Operation opcodes from the socket in a loop and dispatches
        /// to start()/restart()/stop()/get()/finalize() until finalize() (or
        /// an error) ends the session.
        void run();
    };

}

#endif
