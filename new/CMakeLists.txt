 
# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take
# their definition and dump it in the cache along with proper
# documentation, otherwise set  CMAKE_BUILD_TYPE to Debug prior to
# calling PROJECT() 
#

# IF(DEFINED CMAKE_BUILD_TYPE)
#    SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
# build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
# Release RelWithDebInfo MinSizeRel.")
#    MESSAGE(STATUS "Build Type from command line " ${CMAKE_BUILD_TYPE})
# ELSE()
#    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build,
# options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
# RelWithDebInfo MinSizeRel.")
#    MESSAGE(STATUS "Default Build Type: Release  (change with
#    -DCMAKE_BUILD_TYPE=Debug | Release | RelWithDebInfo | MinSizeRel")
# ENDIF()


project( pmlib_server )

cmake_minimum_required (VERSION 2.8.11)

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake.modules )
# INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
# INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
# INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
# INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
# INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
# INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

#set(CMAKE_CXX_COMPILER "clang++-3.6")
#set(CMAKE_C_COMPILER "clang-3.6")

message( STATUS "Detected processor is " ${CMAKE_SYSTEM_PROCESSOR} )
message( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )
message( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )

if ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
    message( STATUS "C++ Compiler is Clang" )
    set( PPICM_HAS_CLANGXX "TRUE" )
   # set( CMAKE_CXX_FLAGS "-stdlib=libc++ -std=gnu++11" )
    set( CMAKE_CXX_FLAGS "-std=c++11" )
   # set( CMAKE_CXX_FLAGS "-std=c++14" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register" )
   # add_definitions( -stdlib=libc++ -std=gnu++11 )  # Or -std=c++11
elseif ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
    message( STATUS "C++ Compiler is GNU")
    execute_process( COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GXX_VERSION )
    if(NOT(GXX_VERSION VERSION_GREATER 4.1))
        set( GCC_TOO_OLD 1 )
        message( FATAL_ERROR "g++ Version < 4.1, some targets will be skipped" )
        message( WARNING "Use CC/CXX env variable to define a newer compiler, e.g. export CC=/usr/bin/gcc-4.2; export CXX=/usr/bin/g++-4.2" )
    else(NOT(GXX_VERSION VERSION_GREATER 4.1))
        #add_definitions( -std=c++14 ) # or -std=c++0x 
        add_definitions( -std=c++11 ) # or -std=c++0x 
    endif(NOT(GXX_VERSION VERSION_GREATER 4.1))
elseif ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel" )
    message( STATUS "C++ Compiler is Intel" )
    message( WARNING "Compliance check not fully implemented yet" )
endif()

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost 1.36.0) 
find_package(Boost COMPONENTS system filesystem thread coroutine log log_setup REQUIRED)
if(NOT(Boost_FOUND))
    message( FATAL_ERROR "Boost library is not installed in your system" )
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(USE_STXXL ON) 
if ( ${USE_STXXL} STREQUAL "ON" )
    add_definitions(-DUSE_STXXL)
    add_subdirectory(stxxl)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STXXL_CXX_FLAGS}")
    include_directories(${STXXL_INCLUDE_DIRS})
else()
    set(STXXL_LIBRARIES "")
endif()

include_directories(${Boost_INCLUDE_DIRS}) 

set( PMLIB ${PROJECT_SOURCE_DIR}/pmlib )

set( PMLIBHEADERS
     ${PROJECT_SOURCE_DIR}/server.hpp
     ${PROJECT_SOURCE_DIR}/counter.hpp
     ${PROJECT_SOURCE_DIR}/device.hpp
     ${PROJECT_SOURCE_DIR}/info.hpp     
     ${PROJECT_SOURCE_DIR}/pmlib_sever.hpp
     ${PROJECT_SOURCE_DIR}/utils/color.hpp
     ${PROJECT_SOURCE_DIR}/utils/logger.hpp
     ${PROJECT_SOURCE_DIR}/utils/json/json.h
     ${PROJECT_SOURCE_DIR}/utils/json/json-forwards.h
)

set( PMLIBSRC
     ${PMLIB}/server.cpp
     ${PMLIB}/counter.cpp
     ${PMLIB}/device.cpp     
     ${PMLIB}/info.cpp
     ${PMLIB}/utils/jsoncpp.cpp
)

#add_subdirectory( pmlib )

include_directories(${STXXL_INCLUDE_DIRS})


add_executable ( pmlib_server ${PMLIB}/pmlib_server.cpp ${PMLIBSRC} )

target_link_libraries( pmlib_server ${Boost_LIBRARIES} ${STXXL_LIBRARIES})

# option( BUILD_TESTS "Build tests" ON )

# message( STATUS "Configuring >>tests<< directory" )

#configure_file(
	#"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	#"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	#IMMEDIATE @ONLY 
	#)

add_custom_target( uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" )

#install( FILES ${PMLIBHEADERS} DESTINATION /include/ppi/ )

#if( BUILD_EXAMPLES )
#    enable_testing()
#    add_subdirectory( examples )
#endif( BUILD_EXAMPLES )

# if( BUILD_TESTS )
#    enable_testing()
#    add_subdirectory( tests )
#endif( BUILD_TESTS )

## Plugins (experimental)
option( USE_DEVICE_APCAPE "Build support for AccelPower CAPE" OFF)
if ( USE_DEVICE_APCAPE )
    add_definitions(-DUSE_DEVICE_APCAPE)
    #add_subdirectory(pmlib/devices/APCape)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STXXL_CXX_FLAGS}")
    #include_directories(${STXXL_INCLUDE_DIRS})
else()
endif()


