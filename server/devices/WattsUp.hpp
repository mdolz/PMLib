/*
 * WattsUp.hpp
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

#ifndef WATTSUP_HPP
#define WATTSUP_HPP

#include <boost/algorithm/string.hpp>

using namespace boost::asio;

/// @file WattsUp.hpp
/// @brief Driver for the WattsUp? Pro power meter (single line, 1 Hz).
///        Ported from the wire protocol in the legacy Python server
///        (legacy/daemon/devices/WattsUpDevice.py): open the serial port
///        raw at 115200 8N1, send the meter's stop/reset/start command
///        sequence, then read comma-separated status lines and pull the
///        watts field out of each one. Not validated against a physical
///        meter; please confirm against real hardware before trusting the
///        readings.

namespace PMLib
{
    template <int n_lines = 1, int max_freq = 1, bool pdu = false>
    class WattsUp : public Device {
      public:
        WattsUp(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {
                io_service io;
                serial_port port( io, url );

                port.set_option( serial_port_base::baud_rate( 115200 ) );
                port.set_option( serial_port_base::character_size( 8 ) );
                port.set_option( serial_port_base::flow_control( serial_port_base::flow_control::none ) );
                port.set_option( serial_port_base::parity( serial_port_base::parity::none ) );
                port.set_option( serial_port_base::stop_bits( serial_port_base::stop_bits::one ) );

                function<void(string)> sendstr = [&](string s)
                    {  write(port, buffer(s.c_str(),  sizeof(char)*s.size()));  };

                sendstr("#L,R,0;"); // stop
                sendstr("#R,W,0;"); // reset
                sendstr("#L,W,3,E,1,1;"); // start logging, external mode

                boost::asio::streambuf buff;
                istream is(&buff);
                string line;
                vector<string> vsample;

                while ( is_running() ) {

                    // The meter terminates each status line with '\n'; a
                    // trailing ';' and any stray whitespace are trimmed
                    // below, mirroring the Python driver's readline()+strip().
                    read_until(port, buff, "\n");
                    getline(is, line);
                    boost::trim_if(line, boost::is_any_of(" \t\r\n;"));
                    boost::split(vsample, line, boost::is_any_of(","));

                    // Field 3 of a full status line ("#d,...,<watts*10>,...;")
                    // is instantaneous watts x10; a short/malformed line
                    // (fewer than the meter's normal 21 fields) is skipped
                    // rather than sampled, matching the Python driver, which
                    // only yields when it sees the expected field count.
                    if ( vsample.size() == 21 ) {
                        sample[0] = stod( vsample[3] ) * 1e-1;
                        yield(sample);
                    }
                }

                port.close();
                io.stop();
            } ) {};
    };

    /// Registers the JSON config "type": "WattsUp".
    static RegisterDevice< WattsUp<> > Reg_WattsUp("WattsUp");
}

#endif