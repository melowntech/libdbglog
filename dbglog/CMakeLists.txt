# bump version here
set(dbglog_VERSION 1.4)

define_module(LIBRARY dbglog=${dbglog_VERSION}
  DEPENDS Boost_THREAD Boost_SYSTEM)

set(dbglog_SOURCES
    dbglog.cpp
    mask.cpp
    logfile.cpp
)

add_library(dbglog STATIC ${dbglog_SOURCES})
buildsys_library(dbglog)

target_link_libraries(dbglog ${MODULE_LIBRARIES})
buildsys_target_compile_definitions(dbglog ${MODULE_DEFINITIONS})

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # Boost.Spirit is non-compilable on Clang in C++11 :(
  set_source_files_properties(mask.cpp PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()
