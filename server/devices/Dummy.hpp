/*
 * Dummy.hpp
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

#ifndef DUMMY_HPP
#define DUMMY_HPP

/// @file Dummy.hpp
/// @brief A fake, hardware-free device that fabricates increasing per-line
///        values at a fixed rate, so the server/Counter/client pipeline
///        can be exercised (and CI can build+smoke-test a running server)
///        without any physical power meter attached.
///
///        Ported from the useful half of
///        legacy/daemon/devices/DummyDevice.py: that file's read() loop
///        (initialize each line to line_index/100, add 1 every sample) is
///        genuinely a dummy device, but its constructor was a copy-paste
///        of PDUDevice.py's and required a "ssh://user:pass@host" URL it
///        never actually used - dropped here, since a device that fakes
///        its own data has no real endpoint to connect to.

namespace PMLib
{
    template <int n_lines = 24, int max_freq = 1, bool pdu = true>
    class Dummy : public Device {
      public:
        Dummy(string name, string url) :
            Device(name, url, max_freq, n_lines, pdu,
            [&] () {

                for ( int s = 0; s < n_lines; s++ ) sample[s] = (s + 1) / 100.0;

                while ( is_running() ) {
                    for ( int s = 0; s < n_lines; s++ ) sample[s] += 1;

                    yield( sample );
                    this_thread::sleep_for(std::chrono::microseconds((int)(1e6/max_freq)));
                }

            } ) {};
    };

    /// Registers the JSON config "type": "Dummy".
    static RegisterDevice< Dummy<> > Reg_Dummy("Dummy");
}

#endif
