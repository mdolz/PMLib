/*
 * device.hpp
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

#ifndef DEVICE_HPP
#define DEVICE_HPP

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <utility>
#include <condition_variable>
//#include <boost/thread.hpp>
//#include <boost/coroutine/asymmetric_coroutine.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#ifdef USE_STXXL
    #include <stxxl/vector>
#endif

using namespace std;
//using namespace boost::coroutines;
using namespace boost::lockfree;

/// @file device.hpp
/// @brief Device driver base class, per-line sample storage, and the
///        device-type registry that JSON config "type" strings resolve to.

namespace PMLib
{
    /// Identifies the machine a Line's outlet is wired to; used mainly by
    /// PDU-style devices (LMG, ArduPower) where each line powers a
    /// different monitored computer.
    struct Computer {
        string _name, _ip;

        Computer() : _name{"Unknown"}, _ip{"0.0.0.0"} {};
        Computer(std::string name, std::string ip) : _name{name}, _ip{ip} {};
    };

    /// Storage for a Line's raw sample history. Backed by STXXL's
    /// out-of-core vector when built with USE_STXXL, so long-running servers
    /// can accumulate samples past main-memory limits by spilling to disk;
    /// otherwise a plain in-memory std::vector.
#ifdef USE_STXXL
    typedef stxxl::VECTOR_GENERATOR<double>::result vector_type;
#else
    typedef std::vector<double> vector_type;
#endif

    /// The physical quantity a Line reports; drives how APCape's INA219
    /// driver interprets each of its lines (see devices/APCape.hpp) and is
    /// otherwise informational.
    enum class Metric { voltage, shunt_voltage, current, power, temperature, def };

    /// One measurable outlet/channel of a Device: calibration data, the
    /// computer it powers, enable/active reference counts (shared across
    /// concurrent Counters), and its accumulated sample history.
    struct Line {
        string _name, _description;
        int _id;
        Metric _metric;
        float _voltage, _offset, _slope;
        std::atomic<int> _enabled, _active;
        Computer _computer;
        vector_type data;

        Line() {};
        Line(string name, string desc, Metric metric, int number, float voltage, 
            Computer &comp, float offset = 0, float slope = 0) :
            _name{name}, _description{desc}, _id{number}, _metric{metric},
            _voltage{voltage}, _offset{offset}, _slope{slope},
            _enabled{0}, _active{0}, _computer{comp} {};

        inline int get_id() const { return _id; }
        inline Metric get_metric() const { return _metric; }
        // _enabled/_active are reference counts, not booleans: several
        // Counters can share a line, so the line must stay enabled/active
        // (and keep buffering data) until the last one lets go.
        inline void enable() { atomic_fetch_add(&_enabled, 1); }
        inline void disable() { if (_enabled.load() > 0) atomic_fetch_sub(&_enabled, 1); else _enabled.store(0); }
        inline bool is_enabled() { return _enabled.load() > 0; }
        inline void activate() { atomic_fetch_add(&_active, 1); }
        inline void deactivate() { if (_active.load() > 0) atomic_fetch_sub(&_active, 1); else _active.store(0); }
        inline bool is_active() { return _active.load() > 0; }

        inline void clear_data() { data.clear(); }
        inline void push_back_data( double d ) { data.push_back(d); }
        inline int get_size() { return data.size(); }
        void print_data() { for ( auto &i : data ) cout << i << " "; cout << endl; };
        inline const vector_type& get_data() { return data; }
    };

    class Counter;
    class Device;

    typedef map<int, long long int> set_t;
    typedef map<string, function<shared_ptr<Device>(string name, string url)>> map_type;

   // typedef asymmetric_coroutine<vector<double>&>::pull_type generator_t;
   // typedef asymmetric_coroutine<vector<double>&>::push_type& producer_t;

    /// Base class for every power-meter driver (WattsUp, LMG, ArduPower,
    /// APCape, ...). A concrete Device subclass supplies little more than a
    /// `_readf` sampling loop passed to the constructor: this base class
    /// handles running that loop on its own thread, moving each sample from
    /// the lock-free queue into per-Line storage, and the start/stop
    /// reference-counting that lets multiple Counters share one device.
    ///
    /// New device types register themselves at static-init time via
    /// RegisterDevice, keyed by the string used in the JSON config's
    /// "type" field (see Server::parse_configfile()).
    class Device {
        string _name, _url;
        int _max_frequency, _n_lines;
        bool _pdu;

        thread the_thread;
        std::atomic<bool> _working, _running;
        map<int, shared_ptr<Line>> lines;
        mutex counter_lock;

        //function<void(producer_t)> _readf;
        function<void()> _readf;
        static map_type *device_register;

      protected:
        vector<double> sample;
        // Bridges the driver's sampling thread (_readf, producer) to run()'s
        // consumer loop without locking on the hot path; sized generously so
        // a slow consumer doesn't cause the driver to block mid-read.
        spsc_queue<vector<double>, capacity<10000> > data_queue, avg_queue;
        inline void yield(vector<double> &s){ data_queue.push(s); };
        // Registry is a function-local static (via get_map()) rather than a
        // plain static member so it's guaranteed constructed before the
        // first RegisterDevice static initializer runs, regardless of
        // translation-unit init order.
        static map_type *get_map() {
            if(!device_register) { device_register = new map_type; }
            return device_register;
        }

      public:
        mutex start_mutex, stop_mutex;
        condition_variable start_cv, stop_cv;

        Computer computer;
        map<int, shared_ptr<Counter>> counter_map;

        Device() {};
        Device(string name, std::string url, int max_freq, int n_lines, bool pdu, function<void()> readf );
        ~Device();

        /// Looks up @p type in the RegisterDevice registry and constructs a
        /// new instance of the matching driver, or nullptr if the type
        /// string doesn't match any linked-in device.
        static shared_ptr<Device> create_device(string type, string name, string url);

        string get_name() const { return _name; };
        int get_max_freq() const { return _max_frequency; };
        int get_num_lines() const { return _n_lines; };
        vector<double> get_sample() const { return sample; };
        inline const map<int,shared_ptr<Line>>& get_lines() const { return lines; };
        /// Attaches a copy of Counter @p c to this device and enables the
        /// lines it requested (see Line::enable()).
        void register_counter(const Counter &c);
        /// Detaches Counter @p c, disabling its lines and clearing their
        /// buffered data once no counter references them anymore.
        void deregister_counter(const Counter &c);

        inline bool is_pdu() { return _pdu; };
        inline bool is_working() { return _working.load(); };
        inline bool is_running() { return _running.load(); };
        inline bool has_counters() { return counter_map.size() > 0; };

        /// Declares one of this device's lines, as parsed from the JSON
        /// config's per-device "lines" object.
        void register_line(string name, string description, string metric, int number, Computer &c, float voltage, float offset = 0, float slope = 0);
        /// Fills @p length with each selected line's current buffer size;
        /// used by Counter::start()/stop() to delimit a measurement set.
        void get_lines_sizes(const vector<int> &sel_lines, set_t &length);
        /// Appends one multi-line sample (as produced by the driver) to
        /// every currently-active line's history.
        void push_back_data(vector<double> &sample);
        const vector_type& get_line_data(int i);

        /// Marks the given lines active, so run()'s consumer loop starts
        /// persisting their incoming samples.
        void start_counter(const vector<int> &sel_lines);
        void stop_counter(const vector<int> &sel_lines);

        /// Consumer loop: drains data_queue into per-line storage at close
        /// to real time, spawning the driver's _readf on its own producer
        /// thread first. Runs until stop() clears _running.
        void run();
        /// Launches run() on a new thread (the_thread).
        void start();
        /// Requests the sampling loop to exit; the destructor joins the
        /// thread once it does.
        void stop();
    };

    /// Registers device subclass T under @p device_class_name so
    /// Device::create_device() can instantiate it from a JSON config's
    /// "type" string. Devices self-register via a file-scope static
    /// instance of this template (see devices/*.hpp), so simply linking a
    /// driver's translation unit in is enough to make it available.
    template <typename T>
    class RegisterDevice : public Device {
      public:
        RegisterDevice(string const &device_class_name)
        {
            auto func = [](string name, string url) { return make_shared<T>(name, url); };
            get_map()->insert( make_pair( device_class_name, func ) );
        }
    };
}

#endif
