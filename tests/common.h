#pragma once

#if defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#define HAS_X86_CPUID 1
#include <cpuid.h>
static inline void __x86_cpuidex(int reg[], int level, int count)
{ __cpuid_count(level, count, reg[0], reg[1], reg[2], reg[3]); }
#elif defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
#define HAS_X86_CPUID 1
#define __x86_cpuidex __cpuidex
#endif

#if defined __APPLE__
static inline const char* get_os_name() { return "macOS"; }
#elif defined __linux__
static inline const char* get_os_name() { return "Linux"; }
#elif defined __FreeBSD__
static inline const char* get_os_name() { return "FreeBSD"; }
#elif defined _WIN32
static inline const char* get_os_name() { return "Windows"; }
#else
static inline const char* get_os_name() { return "unknown"; }
#endif

typedef struct test_state test_state;
struct test_state
{
    io_buffer *io;
    size_t bufsize, count, wsum, rsum, wops, rops, werrs, rerrs;
    clock_t wstart, wend, rstart, rend;
};

#if HAS_X86_CPUID
static inline const char* get_cpu_name()
{
    int leaf_0[4], leaf_2[4], leaf_3[4], leaf_4[4];
    static char cpu_name[64] = "unknown";

    __x86_cpuidex(leaf_0, 0x80000000, 0);

    if (leaf_0[0] >= 0x80000004)
    {
        __x86_cpuidex(leaf_2, 0x80000002, 0);
        __x86_cpuidex(leaf_3, 0x80000003, 0);
        __x86_cpuidex(leaf_4, 0x80000004, 0);
        memcpy(cpu_name + 0x00, leaf_2, 0x10);
        memcpy(cpu_name + 0x10, leaf_3, 0x10);
        memcpy(cpu_name + 0x20, leaf_4, 0x10);
    }

    return cpu_name;
}
#else
static inline const char* get_cpu_name() { return "unknown"; }
#endif

static void io_print_results(test_state s, int nloop)
{
    double s1, s2, s3, s4;
    char name[32];

    s1 = (1e9 * (s.wend - s.wstart)) / ((double)CLOCKS_PER_SEC * s.wops);
    s2 = (1e9 * (s.rend - s.rstart)) / ((double)CLOCKS_PER_SEC * s.rops);
    s3 = (1e9 * (s.wend - s.wstart)) / ((double)CLOCKS_PER_SEC * s.count * nloop);
    s4 = (1e9 * (s.rend - s.rstart)) / ((double)CLOCKS_PER_SEC * s.count * nloop);

    snprintf(name, sizeof(name), "%zu-byte", s.bufsize);

    printf("\nwsum:%016zx (%6.2f sec) rsum:%016zx (%6.2f sec)\n",
        s.wsum, (double)(s.wend - s.wstart) / CLOCKS_PER_SEC,
        s.rsum, (double)(s.rend - s.rstart) / CLOCKS_PER_SEC);
    printf("\n%10s %10s %13s %12s %10s %10s\n", name, "ops",
        "errors", "time", "msg/sec", "MB/sec");
    printf("%10s %10s %13s %12s %10s %10s\n", "","----------",
        "-------------", "------------", "----------", "----------");
    printf("%10s %10zu %13zu %10.2fns %10zu %10.2f\n", "read",
        s.wops, s.werrs, s1, (size_t)(1e9/s1), sizeof(int)*(1e9/s3)/(1024*1024));
    printf("%10s %10zu %13zu %10.2fns %10zu %10.2f\n", "write",
        s.rops, s.rerrs, s2, (size_t)(1e9/s2), sizeof(int)*(1e9/s4)/(1024*1024));
}
