cmake_minimum_required(VERSION 3.15)

project(pmlib_server CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build: None Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake.modules)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

message(STATUS "Detected processor is ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.36.0 REQUIRED COMPONENTS system filesystem thread coroutine log log_setup)

set(USE_STXXL ON CACHE BOOL "Use STXXL for on-disk sample storage")
if(USE_STXXL)
    add_subdirectory(third_party/stxxl)
endif()

## Plugins (experimental)
option(USE_DEVICE_APCAPE "Build support for AccelPower CAPE" OFF)

set(PMLIB ${PROJECT_SOURCE_DIR}/server)

set(PMLIBHEADERS
    ${PMLIB}/server.hpp
    ${PMLIB}/counter.hpp
    ${PMLIB}/device.hpp
    ${PMLIB}/info.hpp
    ${PMLIB}/utils/color.hpp
    ${PMLIB}/utils/logger.hpp
    ${PMLIB}/utils/json/json.h
    ${PMLIB}/utils/json/json-forwards.h
)

set(PMLIBSRC
    ${PMLIB}/server.cpp
    ${PMLIB}/counter.cpp
    ${PMLIB}/device.cpp
    ${PMLIB}/info.cpp
    ${PMLIB}/utils/jsoncpp.cpp
)

add_executable(pmlib_server ${PMLIB}/pmlib_server.cpp ${PMLIBSRC} ${PMLIBHEADERS})

target_compile_features(pmlib_server PRIVATE cxx_std_17)

target_include_directories(pmlib_server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(pmlib_server PRIVATE ${Boost_LIBRARIES})
target_compile_definitions(pmlib_server PRIVATE BOOST_LOG_DYN_LINK)

if(USE_STXXL)
    # The vendored STXXL predates CMake's own imported-target support and
    # exposes itself via these cache variables instead (set by
    # stxxl/CMakeLists.txt), so it's linked that way rather than as a
    # regular target dependency.
    target_compile_definitions(pmlib_server PRIVATE USE_STXXL)
    target_include_directories(pmlib_server PRIVATE ${STXXL_INCLUDE_DIRS})
    target_compile_options(pmlib_server PRIVATE ${STXXL_CXX_FLAGS})
    target_link_libraries(pmlib_server PRIVATE ${STXXL_LIBRARIES})
endif()

if(USE_DEVICE_APCAPE)
    target_compile_definitions(pmlib_server PRIVATE USE_DEVICE_APCAPE)
    # Provides the i2c_smbus_* helpers (libi2c-dev) used by the INA219
    # register access in devices/APCape.hpp.
    target_link_libraries(pmlib_server PRIVATE i2c)
endif()

install(FILES ${PMLIBHEADERS} DESTINATION include/pmlib)
install(TARGETS pmlib_server RUNTIME DESTINATION bin)
