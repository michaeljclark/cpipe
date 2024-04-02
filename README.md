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

## Performance

Benchmarks have been run on Windows 11 and Ubuntu 22.04 with Kaby Lake
and Skylake processors. See [Windows benchmarks](BENCH-Windows.md) and
[Linux benchmarks](BENCH-Linux.md) for details.

### Minimum Latency (nanoseconds)

|                      | cpipe win11 | cpipe linux | linux pipes |
|:---------------------|------------:|------------:|------------:|
| Kaby Lake (i7-8550U) |      ~219ns |      ~362ns |     ~7692ns |
| Skylake (i9-7980XE)  |      ~404ns |      ~425ns |     ~9183ns |

### Message Rate (messages per second)

|                      | cpipe win11 | cpipe linux | linux pipes |
|:---------------------|------------:|------------:|------------:|
| Kaby Lake (i7-8550U) |       4.55M |       2.71M |     129.62K |
| Skylake (i9-7980XE)  |       2.47M |       2.35M |     108.89K |

### Bandwidth 32KB buffer (1-thread)

|                      | cpipe win11 | cpipe linux | linux pipes |
|:---------------------|------------:|------------:|------------:|
| Kaby Lake (i7-8550U) |  2.91GB/sec |  1.36GB/sec |  1.72GB/sec |
| Skylake (i9-7980XE)  |  2.98GB/sec |  1.44GB/sec |  1.67GB/sec |

### Bandwidth 32KB buffer (4-threads)

|                      | cpipe win11 | cpipe linux |
|:---------------------|------------:|------------:|
| Kaby Lake (i7-8550U) |  5.56GB/sec |  0.79GB/sec |
| Skylake (i9-7980XE)  |  7.11GB/sec |  0.89GB/sec |

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
