<p align="center">
  <img src="docs/img/pmlib-logo.png" alt="PMLib logo" width="140">
</p>

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
 ┌──────────────┐ JSON config ┌─────────────────────────┐
 │ pmlib_server │────────────▶│  devices (WattsUp, LMG, │
 │    (src/)    │             │ ArduPower, APCape, ...) │
 └───────┬──────┘             └─────────────────────────┘
         │ TCP
         ▼
 ┌─────────────┐
 │  client app │  pm_set_server / pm_create_counter / pm_start_counter
 │ (your code) │  pm_stop_counter / pm_get_counter_data / pm_print_data_csv
 └─────────────┘
```

## Repository layout

```
PMLib/
├── CMakeLists.txt
├── src/                Server, counter, device and device-driver sources
│                       (the actively maintained C++17 implementation)
├── third_party/stxxl/  Vendored STXXL, used for on-disk sample storage
├── examples/           Example JSON server configurations
├── legacy/             Legacy Python2/C client-server implementation
│   ├── client/         C client API (pmlib.h) predating the C++ rewrite
│   └── server/         Python daemon with additional device backends
│                       (IPMI, National Instruments, PDU, DC/DC2)
└── docs/               Doxygen output and static assets (logo, ...)
```

The `legacy/` tree is kept for reference and predates the C++ rewrite in
`src/`; it is not actively developed but is left untouched.

## Supported devices

| Device | Status | Implementation |
|---|---|---|
| WattsUp? Pro | Supported | `src/devices/WattsUp.hpp` |
| ZES Zimmer LMG450 | Supported | `src/devices/LMG.hpp` |
| ArduPower (Arduino-based PDU) | Supported | `src/devices/ArduPower.hpp` |
| APCape / AccelPower CAPE | Experimental (`USE_DEVICE_APCAPE`) | `src/devices/APCape.hpp` |
| IPMI, National Instruments, generic PDU, DC/DC2 | Legacy only | `legacy/server/daemon/devices/` |

## Building

### Requirements

- CMake ≥ 3.15
- A C++17 compiler (GCC ≥ 7 or Clang ≥ 5)
- Boost ≥ 1.36 (`system`, `filesystem`, `thread`, `coroutine`, `log`, `log_setup`)

```bash
cmake -S . -B build
cmake --build build -j
```

This produces `build/pmlib_server`. To build with experimental AccelPower
CAPE support:

```bash
cmake -S . -B build -DUSE_DEVICE_APCAPE=ON
```

### Configuring

The server reads a JSON configuration file describing the machine IP/port to
listen on, the computers being monitored, and the devices/lines attached to
them. See [`examples/settings.json`](examples/settings.json) for a complete
example covering WattsUp, LMG450 and ArduPower devices, and
[`examples/settings-APCape.json`](examples/settings-APCape.json) for the
experimental APCape device.

```bash
./pmlib_server --configfile /path/to/settings.json
# or, to run as a daemon:
./pmlib_server --daemonize --configfile /path/to/settings.json
```

## Client usage example

Clients talk to `pmlib_server` over TCP using the C API declared in
[`legacy/client/pmlib.h`](legacy/client/pmlib.h). A minimal client looks
like this (see [`legacy/client/test/example1.c`](legacy/client/test/example1.c)
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

API reference for the `src/` C++ sources is generated with
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
device drivers under `src/devices/` or `legacy/server/daemon/devices/`,
please note in the PR description which physical device you tested against,
since these cannot be exercised in CI.

## License

PMLib is distributed under the [GNU Lesser General Public License v3.0](LICENSE).

## Author

[Manuel F. Dolz](https://sites.google.com/uji.es/manuel-f-dolz/) — Universitat Jaume I
