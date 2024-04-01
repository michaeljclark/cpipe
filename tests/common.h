#pragma once

typedef struct test_state test_state;
struct test_state
{
    io_buffer *io;
    size_t bufsize, count, wsum, rsum, wops, rops, werrs, rerrs;
    clock_t wstart, wend, rstart, rend;
};

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
