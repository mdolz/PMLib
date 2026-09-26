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
///        The serial-port protocol implementation below is commented out
///        pending hardware validation; the active sampling loop is a stub
///        that fabricates increasing values so the device/Counter pipeline
///        can be exercised without physical hardware attached.

namespace PMLib
{
    template <int n_lines = 1, int max_freq = 1, bool pdu = false>
    class WattsUp : public Device {
      public:
        WattsUp(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {
                /*
                // Experimental code: real WattsUp serial protocol (untested
                // on hardware, kept for reference/completion by whoever has
                // a device to validate against).
                io_service io;
                serial_port port( io, url );

                port.set_option( serial_port_base::baud_rate( 115200 ) );
                port.set_option( serial_port_base::character_size( 8 ) );
                port.set_option( serial_port_base::flow_control( serial_port_base::flow_control::none ) );
                port.set_option( serial_port_base::parity( serial_port_base::parity::none ) );
                port.set_option( serial_port_base::stop_bits( serial_port_base::stop_bits::one ) );

                function<void(string)> sendstr = [&](string s) 
                    {  write(port, buffer(s.c_str(),  sizeof(char)*s.size()));  };

                sendstr("#L,R,0;"); // stop;
                sendstr("#R,W,0;"); // reset;
                sendstr("#L,W,3,E,1,1;"); // start;

                boost::asio::streambuf buff;
                istream is(&buff);
                string line;
                vector<string> vsample;

                while ( is_running() ) { 

                    read_until(port, buff, ";\r\n");
                    getline(is, line);
                    boost::split(vsample, line, boost::is_any_of(","));

                    if ( vsample.size() == 21 ) {
                        sample[0] = stod( vsample[3] ) * 1e-1;
                    }

                    yield(sample);
                }

                port.close();
                io.stop();
                */

                // Stub sampling loop (no serial hardware read yet): see
                // file-level comment above.
                while ( is_running() ) {
                    sample[0] += 1;

                    yield( sample );
                    this_thread::sleep_for(std::chrono::microseconds((int)(1e6/max_freq)));
                }

            } ) {};
    };

    /// Registers the JSON config "type": "WattsUp".
    static RegisterDevice< WattsUp<> > Reg_WattsUp("WattsUp");
}

#endif