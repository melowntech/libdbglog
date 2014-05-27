define_module(LIBRARY dbglog DEPENDS Boost_THREAD Boost_SYSTEM)

set(dbglog_SOURCES
    dbglog.cpp
    mask.cpp
)

add_library(dbglog STATIC ${dbglog_SOURCES})
buildsys_library(dbglog)

target_link_libraries(dbglog ${MODULE_LIBRARIES})
target_compile_definitions(dbglog PRIVATE ${MODULE_DEFINITIONS})

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # Boost.Spirit is non-compilable on Clang in C++11 :(
  set_source_files_properties(mask.cpp PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()
