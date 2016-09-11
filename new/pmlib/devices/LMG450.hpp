/*
 * LMG450.hpp
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

#ifndef LMG450_HPP
#define LMG450_HPP

namespace PMLib
{
    template <int n_lines = 4, int max_freq = 20, bool pdu = true>
    class LMG450 : public Device {
      public:
        LMG450(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu, 
            [&] () {

            while ( is_running() ){ 
                sample[0]+=0.01;
                sample[1]+=0.02;
                sample[2]+=0.01;
                sample[3]+=0.03;

                yield(sample);
                this_thread::sleep_for(chrono::microseconds((int)(1e6/max_freq)));
            }
                        
        } ) {};   
    };

    static RegisterDevice< LMG450<> > Reg_LMG450("LMG450");
}

#endif