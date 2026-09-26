/*
 * info.hpp
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

#ifndef INFO_HPP
#define INFO_HPP

/// @file info.hpp
/// @brief Read-only, connection-scoped queries against the server's device
///        inventory (as opposed to Counter, which owns a measurement).

namespace PMLib
{
    /// Handles the introspection opcodes (LIST_DEVICES, INFO_DEVICE,
    /// CMD_STATUS, READ_DEVICE). Unlike Counter, an Info instance does not
    /// persist beyond a single request/reply and never registers itself
    /// with a device.
    class Info {
        socket_ptr _sock;
        Server* _server;

      public:
        /// Immediately dispatches to run(op); the constructor is the only
        /// entry point since an Info has no further use once its one
        /// request completes.
        Info(socket_ptr sock, Operation op, Server* server);

        /// Replies with the device count followed by each configured
        /// device's name, per client/pm_get_devices.c's protocol.
        void list_devices();
        /// Reads a device name off the socket and replies with its max
        /// frequency, line count and name.
        void info_device();
        /// Replies with a full dump of every device: its lines, computers
        /// and currently-registered counters. Used by monitoring/status
        /// tools rather than by measurement clients.
        void cmd_status();
        /// Streams live samples for a named device to the client at the
        /// requested frequency until the connection closes; distinct from
        /// Counter, which records samples server-side for later retrieval.
        void read_device();
        /// Dispatches to the handler matching @p op.
        void run(Operation op);
    };
}

#endif
