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

using namespace boost::asio;

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 20, bool pdu = true>
    class LMG : public Device {
      public:
        LMG(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {
                /*
                // Experimental code
                io_service io;
                serial_port port( io, url );

                port.set_option( serial_port_base::baud_rate( 115200 ) );
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
                sendstr("ACTN;P1?;P2?;P3?;P4?\n");
                sendstr("CONT ON\n");

                boost::asio::streambuf buff;
                istream is(&buff);
                string line;
                vector<string> vsample;

                while ( is_running() ) { 

                    read_until(port, buff, "\n");
                    getline(is, line);

                    for ( int s = 0; s < n_lines; s++ ) {
                        // std::reverse(..., ...); // Needed if data comes little-endian
                        sample[s] = *reinterpret_cast<double*>( line[7+s] );
                    }

                    yield(sample);
                }

                sendstr("CONT OFF\n");
                sendstr("FRMT ASCII\n");
                sendstr("GTL\n");

                port.close();
                io.stop();
                */

                while ( is_running() ) { 
                    sample[0] += 0.01;
                    sample[1] += 0.02;
                    sample[2] += 0.01;
                    sample[3] += 0.03;

                    yield( sample );
                    this_thread::sleep_for(chrono::microseconds((int)(1e6/max_freq)));
                }    
        } ) {};   
    };

    static RegisterDevice< LMG< 4 > > Reg_LMG450("LMG450");
    static RegisterDevice< LMG< 8 > > Reg_LMG500("LMG500");
}

#endif