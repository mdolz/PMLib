/*
 * logger.hpp
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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp>

#define CLOG_INFO  BOOST_LOG_SEV(lg::get(), trivial::info)
#define CLOG_WARN  BOOST_LOG_SEV(lg::get(), trivial::warning)
#define CLOG_ERROR BOOST_LOG_SEV(lg::get(), trivial::error)

using namespace boost::log;

namespace PMLib {

    typedef sources::severity_logger_mt<trivial::severity_level> logger_t;
    BOOST_LOG_GLOBAL_LOGGER(lg, logger_t);
   
}

#endif