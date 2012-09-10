set(dbglog_SOURCES
    dbglog.cpp
    mask.cpp
)

add_library(dbglog STATIC ${dbglog_SOURCES})
set_target_properties(dbglog PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
  ${CMAKE_SOURCE_DIR}/lib)

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # Boost.Spirit is non-compilable on Clang in C++11 :(
  set_source_files_properties(mask PROPERTIES
    COMPILE_FLAGS -std=c++98)
endif()
