cmake_minimum_required(VERSION 3.1)
project (tempCClosed C CXX)

if(POLICY CMP0012)
  CMAKE_POLICY(SET CMP0012 NEW)
endif(POLICY CMP0012)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option (DEBUG "debug mode (0=none, 9=max)" OFF)
option (STATIC "enable static build" OFF)
option (CLANG "enable static build" OFF)

message("Options: (enable with -D<option>=ON & disable with -D<option>=OFF)")
message("STATIC: static build (STATIC)           " ${STATIC} )
message("CLANG: compile with clang (CLANG)      " ${CLANG} )

set (source_files CMakeLists.txt main.cpp)
include_directories(.)

set(EXECUTABLE_OUTPUT_PATH "bin")
add_custom_target ( doc doxygen Doxyfile )


# static linking
if(${STATIC} STREQUAL ON)
  set(Boost_USE_STATIC_LIBS   ON)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  SET(BUILD_SHARED_LIBRARIES OFF)
  SET(CMAKE_EXE_LINKER_FLAGS "-static")
endif(${STATIC} STREQUAL ON)

set(CMAKE_CXX_FLAGS "-g")


# ==================== REQUIRED PACKAGES ===================
FIND_PACKAGE( Boost COMPONENTS program_options graph REQUIRED )
INCLUDE_DIRECTORIES( "${Boost_INCLUDE_DIR}" )

if(NOT Boost_FOUND)
  MESSAGE(FATAL_ERROR "Could not find Boost.")
endif(NOT Boost_FOUND)

ADD_EXECUTABLE(tempo source/tempo/main.cpp source/tempo/Graph.cpp)
ADD_EXECUTABLE(convert source/convert/main.cpp)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
TARGET_LINK_LIBRARIES(tempo ${Boost_LIBRARIES})
