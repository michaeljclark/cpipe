# cpipe

low latency concurrent pipe buffer using C11 threads and atomics.

This header library implements a concurrent pipe buffer designed for use
in single producer single consumer or multiple producer multiple consumer
modes with support for concurrent write ahead, concurrent read ahead,
and zero copy operation for array computation within the IO buffer spans.

- [C11 <threads.h> emulation library](https://cgit.freedesktop.org/mesa/mesa/log/include/c11)
is distributed under the Boost Software License, Version 1.0.
- C11 <atomic.h> emulation library is distributed under the PLEASE LICENSE.

## Implementation

The concurrent pipe buffer is a circular buffer internally, where
monitonically increasing buffer markers are stored without a modulus so
that empty and full conditions can be distinguished by the distance between
the start and end markers along with math that gracefully supports overflow.

The read and write primitives calculate a read ahead or write ahead mark
that is consistent with the buffer space invariant then atomically take a
ticket for a read ahead or write ahead mark, perform copies, then finally
atomically commit the new start or end mark, spinning to retire in order.

#### circular buffer markers

```
                start                   end
                |       start_mark      |       end_mark
________________--------XXXXXXXXXXXXXXXX++++++++________________
```

#### circular buffer markers (wrapped)

```
                end+cap                 start
                |       end_mark+cap    |       start_mark
XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX
```

## Build instructions

cpipe builds have been tested using CMake on the following platforms:

- Windows 11 (x86-64) using Visual Studio 17.
- Ubuntu 22.04 (x86-64) with GCC 11.4 and Clang 14.0.

### Windows

```
cmake -B build_win32 -G "Visual Studio 17 2022" -A x64
cmake --build build_win32 --config RelWithDebInfo
```

### Linux

```
cmake -B build_linux -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build_linux
```
