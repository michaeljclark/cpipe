cmake_minimum_required (VERSION 3.12)

project(cpipe)

include(CheckCCompilerFlag)
include(CheckCSourceCompiles)

set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ENABLE_ASAN "Enable ASAN" OFF)
option(ENABLE_MSAN "Enable MSAN" OFF)
option(ENABLE_TSAN "Enable TSAN" OFF)
option(ENABLE_UBSAN "Enable UBSAN" OFF)

#
# macros for compiler and linker flags
#
macro(add_compiler_flag)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARGN}")
   set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${ARGN}")
endmacro(add_compiler_flag)

macro(add_linker_flag)
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${ARGN}")
endmacro(add_linker_flag)

#
# address, memory, thread and undefined sanitizers
#
if (ENABLE_ASAN)
  add_compiler_flag("-fsanitize=address")
  add_linker_flag("-fsanitize=address")
endif()

if(ENABLE_MSAN)
  add_compiler_flag("-fsanitize=memory")
  add_linker_flag("-fsanitize=memory")
endif()

if(ENABLE_TSAN)
  add_compiler_flag("-fPIE -fsanitize=thread")
  add_linker_flag("-fPIE -fsanitize=thread")
endif()

if(ENABLE_UBSAN)
  add_compiler_flag("-fsanitize=undefined")
  add_linker_flag("-fsanitize=undefined")
endif()

#
# helper macro for logging variables
#
macro(log_boolean)
  set(spaces "                                                  ")
  string(LENGTH ${ARGV0} slen)
  math(EXPR plen "50 - ${slen}")
  string(SUBSTRING ${spaces} 0 ${plen} spaces)
  if (${${ARGV0}})
    message("${ARGV0}${spaces}: true")
  else()
    message("${ARGV0}${spaces}: false")
  endif()
endmacro()

#
# check for MSVC compiler flags and intrinsics
#

# check for MSVC /WX flag and enable for flag tests
check_c_compiler_flag(/WX has_cc_flag_WX)
if (has_cc_flag_WX)
  # MSVC will treat warnings as errors for flag tests
  list(APPEND CMAKE_REQUIRED_FLAGS "/WX")
endif()

# check for MSVC /volatile:iso then enable
check_c_compiler_flag(/volatile:iso has_cc_flag_volatile_iso)
if (has_cc_flag_volatile_iso)
  # MSVC will define _ISO_VOLATILE 1
  add_compiler_flag(/volatile:iso)
  list(APPEND CMAKE_REQUIRED_FLAGS "/volatile:iso")
endif()

# check for MSVC interlocked intrinsics
check_c_source_compiles("#include <intrin.h>
int main() { long v = 0; _InterlockedExchange(&v, 0); }" has_fn_intrin_h_interlocked_exchange)
if(has_fn_intrin_h_interlocked_exchange)
  add_definitions(-DHAVE_INTRIN_INTERLOCKED_EXCHANGE)
endif()

# check for C11 standard atomics
check_c_source_compiles("#include <stdatomic.h>
int main() { _Atomic int v; return atomic_exchange(&v,0); }" has_fn_std_atomic_h_atomic_exchange)

# check for C11 thread support library
find_package(Threads REQUIRED)
check_include_file("threads.h" have_threads_h)
list(APPEND CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})
check_c_source_compiles("#include <threads.h>
int main() { thrd_t t; thrd_create(&t, &main, NULL); }" has_fn_threads_h_thrd_create)

#
# check time related C Library functions
#

# check for timespec_get
check_c_source_compiles("#include <time.h>
int main() { struct timespec ts; timespec_get(&ts, 0); }" has_fn_time_h_timespec_get)
# check for gmtime_r
check_c_source_compiles("#include <time.h>
int main() { time_t ts = 0; struct tm tr; return gmtime_r(&ts, &tr) == &tr; }" has_fn_time_h_gmtime_r)
# check for gmtime_s
check_c_source_compiles("#include <time.h>
int main() { time_t ts = 0; struct tm tr; return gmtime_s(&ts, &tr) == &tr; }" has_fn_time_h_gmtime_s)
# check for gmtime_s with reversed operands
check_c_source_compiles("#include <time.h>
int main() { time_t ts = 0; struct tm tr; return gmtime_s(&tr, &ts) == 0; }" has_fn_time_h_gmtime_s_reversed)

# set macros for gmtime_r, gmtime_s
if (has_fn_time_h_gmtime_r)
  add_definitions(-DHAVE_GMTIME_R)
elseif(has_fn_time_h_gmtime_s)
  add_definitions(-DHAVE_GMTIME_S)
elseif(has_fn_time_h_gmtime_s_reversed)
  add_definitions(-DHAVE_GMTIME_S_REVERSED)
endif()

# enable C11 threads compat
if(NOT has_fn_threads_h_thrd_create)
  set(requires_compat_threads_h 1)
  include_directories(include/compat-threads)
endif()
if(CMAKE_USE_PTHREADS_INIT)
  add_definitions(-DHAVE_PTHREAD)
  if(has_fn_time_h_timespec_get)
    add_definitions(-DHAVE_TIMESPEC_GET)
  endif()
endif()

# enable C11 atomic compat
if(NOT has_fn_std_atomic_h_atomic_exchange)
  set(requires_compat_stdatomic_h 1)
  include_directories(include/compat-atomic)
endif()

# log configuration results
log_boolean(has_cc_flag_volatile_iso)
log_boolean(has_fn_intrin_h_interlocked_exchange)
log_boolean(has_fn_std_atomic_h_atomic_exchange)
log_boolean(has_fn_threads_h_thrd_create)
log_boolean(has_fn_time_h_timespec_get)
log_boolean(has_fn_time_h_gmtime_r)
log_boolean(has_fn_time_h_gmtime_s)
log_boolean(has_fn_time_h_gmtime_s_reversed)
log_boolean(requires_compat_threads_h)
log_boolean(requires_compat_stdatomic_h)

#
# c11atomic tests
#
include_directories(include)

add_executable(test_001 tests/test_001.c)
add_executable(test_002 tests/test_002.c)
add_executable(test_003 tests/test_003.c)
add_executable(test_004 tests/test_004.c)
add_executable(test_005 tests/test_005.c)

target_link_libraries(test_001 ${EXTRA_LIBS})
target_link_libraries(test_002 ${EXTRA_LIBS})
target_link_libraries(test_003 ${EXTRA_LIBS})
target_link_libraries(test_004 ${EXTRA_LIBS})
target_link_libraries(test_005 ${EXTRA_LIBS})
