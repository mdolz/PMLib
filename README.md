<p align="center">
  <img src="docs/img/pmlib-logo.svg" alt="PMLib logo" width="140">
</p>

<h1 align="center">PMLib</h1>
<p align="center"><b>Power Measurement Library</b></p>

<p align="center">
  <a href="LICENSE"><img alt="License: LGPL v3" src="https://img.shields.io/badge/license-LGPL--3.0-blue.svg"></a>
  <img alt="Language: C++17" src="https://img.shields.io/badge/C%2B%2B-17-00599C.svg">
  <img alt="Language: Python" src="https://img.shields.io/badge/legacy-Python-3776AB.svg">
  <img alt="Platform: Linux" src="https://img.shields.io/badge/platform-Linux-lightgrey.svg">
</p>

PMLib is a modular, open-source power measurement library to investigate the
power and energy usage of high-performance computing applications. It runs a
lightweight server next to one or more power meters, and exposes a simple
client API so applications (or external monitoring tools) can start/stop
timed measurement "counters" and pull back per-line power samples over the
network.

## How it works

PMLib follows a **client/server** model over TCP:

- A **server** process owns the actual power-measurement hardware. It is
  configured from a JSON file describing the available *devices* (e.g. a
  WattsUp meter, an LMG450 power analyzer, an ArduPower board) and the
  *lines* each device exposes (one line per monitored computer/outlet, with
  its nominal voltage and calibration data).
- One or more **clients** connect to the server and open *counters*: named,
  independent measurement sessions over a chosen subset of lines. A counter
  can be started, stopped, restarted and read back mid-run, so applications
  can bracket a region of code with `pm_start_counter`/`pm_stop_counter` and
  later fetch the energy/power samples for that region.
- The server keeps sampling every registered line continuously and only
  buffers/serves data for the lines a client has actually requested,
  supporting several concurrent counters against the same device.

```
 ┌──────────────┐        JSON config         ┌────────────────────────┐
 │ pmlib_server │ ─────────────────────────▶ │ devices (WattsUp, LMG,  │
 │  (new/pmlib) │                             │ ArduPower, APCape, ...) │
 └──────┬───────┘                             └────────────────────────┘
        │ TCP
        ▼
 ┌──────────────┐
 │ client app   │  pm_set_server / pm_create_counter / pm_start_counter
 │ (your code)  │  pm_stop_counter / pm_get_counter_data / pm_print_data_csv
 └──────────────┘
```

## Repository layout

```
PMLib/
├── new/            Modern C++17 rewrite of the server (actively maintained)
│   ├── pmlib/      Server, counter, device and device-driver sources
│   ├── stxxl/       Vendored STXXL, used for on-disk sample storage
│   └── CMakeLists.txt
└── Python/         Legacy Python2/C client-server implementation
    ├── client/      C client API (pmlib.h) predating the C++ rewrite
    └── server/      Python daemon with additional device backends
                      (IPMI, National Instruments, PDU, DC/DC2)
```

The `Python/` tree is kept for reference and predates the `new/` C++
rewrite; it is not actively developed but is left untouched.

## Supported devices

| Device | Status | Implementation |
|---|---|---|
| WattsUp? Pro | Supported | `new/pmlib/devices/WattsUp.hpp` |
| ZES Zimmer LMG450 | Supported | `new/pmlib/devices/LMG.hpp` |
| ArduPower (Arduino-based PDU) | Supported | `new/pmlib/devices/ArduPower.hpp` |
| APCape / AccelPower CAPE | Experimental (`USE_DEVICE_APCAPE`) | `new/pmlib/devices/APCape.hpp` |
| IPMI, National Instruments, generic PDU, DC/DC2 | Legacy only | `Python/server/daemon/devices/` |

## Building (new/ C++ server)

### Requirements

- CMake ≥ 3.15
- A C++17 compiler (GCC ≥ 7 or Clang ≥ 5)
- Boost ≥ 1.36 (`system`, `filesystem`, `thread`, `coroutine`, `log`, `log_setup`)

```bash
cd new
mkdir build && cd build
cmake ..
make -j
```

This produces the `pmlib_server` binary. To build with experimental
AccelPower CAPE support:

```bash
cmake .. -DUSE_DEVICE_APCAPE=ON
```

### Configuring

The server reads a JSON configuration file describing the machine IP/port to
listen on, the computers being monitored, and the devices/lines attached to
them. See [`new/settings.json`](new/settings.json) for a complete example
covering WattsUp, LMG450 and ArduPower devices, and
[`new/settings-APCape.json`](new/settings-APCape.json) for the experimental
APCape device.

```bash
./pmlib_server --configfile /path/to/settings.json
# or, to run as a daemon:
./pmlib_server --daemonize --configfile /path/to/settings.json
```

## Client usage example

Clients talk to `pmlib_server` over TCP using the C API declared in
[`Python/client/pmlib.h`](Python/client/pmlib.h). A minimal client looks
like this (see [`Python/client/test/example1.c`](Python/client/test/example1.c)
for a runnable version):

```c
#include "pmlib.h"

server_t server;
counter_t counter;
line_t lines;

pm_set_server("127.0.0.1", 6526, &server);

pm_set_lines("0", &lines);          /* measure line 0 */
pm_create_counter("region1", lines, /*aggregate=*/1, /*interval=*/0,
                   server, &counter);

pm_start_counter(&counter);
/* ... region of code to measure ... */
pm_stop_counter(&counter);

pm_get_counter_data(&counter);
pm_print_data_csv("region1.csv", counter, lines, /*set=*/0);
```

## Documentation

API reference for the `new/pmlib` C++ sources is generated with
[Doxygen](https://www.doxygen.nl/):

```bash
doxygen Doxyfile
```

HTML output is written to `docs/html/index.html`.

## Citation

If you use PMLib in academic work, please cite:

> S. Barrachina, M. Barreda, S. Catalán, M. F. Dolz, G. Fabregat, R. Mayo,
> E. S. Quintana-Ortí. *"An integrated framework for power-performance
> analysis of parallel scientific workloads."* 3rd International Conference
> on Smart Grids, Green Communications and IT Energy-aware Technologies
> (ENERGY 2013).

## Contributing

Issues and pull requests are welcome. When contributing to the hardware
device drivers under `new/pmlib/devices/` or `Python/server/daemon/devices/`,
please note in the PR description which physical device you tested against,
since these cannot be exercised in CI.

## License

PMLib is distributed under the [GNU Lesser General Public License v3.0](LICENSE).

## Author

[Manuel F. Dolz](https://sites.google.com/uji.es/manuel-f-dolz/) — Universitat Jaume I
