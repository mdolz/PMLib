/*
 * LMG.hpp
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

#ifndef LMG_HPP
#define LMG_HPP

#include <boost/algorithm/string.hpp>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace boost::asio;

/// @file LMG.hpp
/// @brief Driver for the ZES Zimmer LMG450/LMG500 power analyzer, used as a
///        multi-outlet (PDU-style) device.
///
///        Ported from legacy/daemon/devices/LMG450Device.py, which is the
///        only one of the two device types (LMG450, n_lines=4) that Python
///        actually implements and was presumably run against real
///        hardware; LMG500 (n_lines=8) is a same-protocol extrapolation
///        with no Python reference to check it against, so please validate
///        it specifically before trusting its readings.
///
///        The Python driver's baud rate (57600, hardware flow control) and
///        binary reply format (a fixed-size packed struct, not a text
///        line) are load-bearing: an earlier draft of this file used
///        115200 and a text-line read here, neither of which match the
///        actual LMG wire protocol.

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 20, bool pdu = true>
    class LMG : public Device {
      public:
        LMG(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {
                io_service io;
                serial_port port( io, url );

                port.set_option( serial_port_base::baud_rate( 57600 ) );
                port.set_option( serial_port_base::character_size( 8 ) );
                port.set_option( serial_port_base::flow_control( serial_port_base::flow_control::hardware ) );
                port.set_option( serial_port_base::parity( serial_port_base::parity::none ) );
                port.set_option( serial_port_base::stop_bits( serial_port_base::stop_bits::one ) );

                function<void(string)> sendstr = [&](string s)
                    {  write(port, buffer(s.c_str(),  sizeof(char)*s.size()));  };

                sendstr(":SYSTem:LANGuage SHORT\n");
                sendstr("FRMT PACKED\n");
                std::stringstream ss;
                ss << "CYCL " << std::fixed << std::setprecision(6) << (double)(1.0/max_freq) << "\n";
                sendstr(ss.str());

                // Requests one power reading channel per line ("P1?;P2?;...").
                std::stringstream actn;
                actn << "ACTN";
                for ( int s = 0; s < n_lines; s++ ) actn << ";P" << (s+1) << "?";
                actn << "\n";
                sendstr(actn.str());

                sendstr("CONT ON\n");

                // Each reply is a fixed-size packed frame: a 7-byte header,
                // then one little-endian 32-bit float per requested
                // channel, then a 1-byte trailer - never text/line-based,
                // so it's read as a raw byte count rather than delimited.
                const size_t header_bytes = 7, trailer_bytes = 1;
                const size_t frame_bytes = header_bytes + n_lines * sizeof(float) + trailer_bytes;
                vector<char> frame(frame_bytes);

                while ( is_running() ) {

                    boost::asio::read(port, buffer(frame, frame_bytes));

                    for ( int s = 0; s < n_lines; s++ ) {
                        float v;
                        // memcpy avoids the misaligned-pointer UB of
                        // reinterpret_cast'ing straight into the buffer;
                        // assumes a little-endian host, which the LMG's
                        // own packed format is defined in terms of.
                        std::memcpy( &v, frame.data() + header_bytes + s * sizeof(float), sizeof(float) );
                        sample[s] = v;
                    }

                    yield(sample);
                }

                sendstr("CONT OFF\n");
                sendstr("FRMT ASCII\n");
                sendstr("GTL\n");

                port.close();
                io.stop();
        } ) {};
    };

    /// Registers the JSON config "type": "LMG450" (4 lines, validated
    /// against the Python reference implementation's protocol) and
    /// "LMG500" (8 lines, same protocol extrapolated to more channels -
    /// unverified, see the file-level note above).
    static RegisterDevice< LMG< 4 > > Reg_LMG450("LMG450");
    static RegisterDevice< LMG< 8 > > Reg_LMG500("LMG500");
}

#endif