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
#include <boost/coroutine/asymmetric_coroutine.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#ifdef USE_STXXL
    #include <stxxl/vector>
#endif

using namespace std;
using namespace boost::coroutines;
using namespace boost::lockfree;

namespace PMLib
{
    struct Computer {
        string _name, _ip;

        Computer() : _name{"Unkwon"}, _ip{"0.0.0.0"} {};
        Computer(std::string name, std::string ip) : _name{name}, _ip{ip} {};
    };

#ifdef USE_STXXL
    typedef stxxl::VECTOR_GENERATOR<double>::result vector_type;
#else
    typedef std::vector<double> vector_type;
#endif

    struct Line {
        string _name, _description;
        int _id;
        float _voltage, _offset, _slope;
        std::atomic<int> _enabled, _active;
        Computer _computer;
        vector_type data;

        Line() {};
        Line(string name, string desc, int number, float voltage, 
            Computer &comp, float offset = 0, float slope = 0) :
            _name{name}, _description{desc}, _id{number}, 
            _voltage{voltage}, _offset{offset}, _slope{slope}, 
            _enabled{0}, _active{0}, _computer{comp} {};

        inline int get_id() const { return _id; }
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
        spsc_queue<vector<double>, capacity<10000> > spsc_queue;
        inline void yield(vector<double> &s){ spsc_queue.push(s); };
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

        static shared_ptr<Device> create_device(string type, string name, string url);
        
        string get_name() const { return _name; };
        int get_max_freq() const { return _max_frequency; };
        int get_num_lines() const { return _n_lines; };
        vector<double> get_sample() const { return sample; };
        inline const map<int,shared_ptr<Line>>& get_lines() const { return lines; };
        void register_counter(const Counter &c);
        void deregister_counter(const Counter &c);
        
        inline bool is_pdu() { return _pdu; };
        inline bool is_working() { return _working.load(); };
        inline bool is_running() { return _running.load(); };
        inline bool has_counters() { return counter_map.size() > 0; };

        void register_line(string name, string description, int number, Computer &c, float voltage, float offset = 0, float slope = 0);
        void get_lines_sizes(const vector<int> &sel_lines, set_t &length);
        void push_back_data(vector<double> &sample);
        const vector_type& get_line_data(int i);

        void start_counter(const vector<int> &sel_lines);
        void stop_counter(const vector<int> &sel_lines);

        void run();
        void start();        
        void stop();
    };

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
