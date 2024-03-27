#undef NDEBUG
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <threads.h>

#ifdef _WIN32
#define alloca _alloca
#else
#include <alloca.h>
#endif

#include "buffer.h"

#define NLOOP 256
#define NTHREAD 1

typedef struct test_state test_state;
struct test_state
{
    io_buffer *io;
    size_t bufsize, count, wsum, rsum, wops, rops, werrs, rerrs;
    clock_t wstart, wend, rstart, rend;
};

static int io_write_thread(void* arg)
{
    test_state *s = (test_state*)arg;
    size_t bufsize = s->bufsize, count = s->count;
    size_t sum = 0, ops = 0, errs = 0;
    uint *arr = alloca(count * sizeof(int));
    s->wstart = clock();
    for (size_t j = 0; j < NLOOP; j++) {
        uint seq = 0;
        for (size_t i = 0, l = 0; i < count;) {
            for (; l < count && l < i + bufsize; l++) {
                seq = seq * 793517 + (int)l;
                sum += (arr[l] = seq);
            }
            size_t r = io_buffer_write(s->io, (char*)&arr[i], (count-i)*sizeof(int));
            i += (r>>2);
            if (r) ops++; else errs++;
        }
    }
    s->wend = clock();
    s->wops = ops;
    s->werrs = errs;
    s->wsum = sum;

    return 0;
}

static int io_read_thread(void* arg)
{
    test_state *s = (test_state*)arg;
    size_t count = s->count;
    size_t sum = 0, ops = 0, errs = 0;
    uint *arr = alloca(count * sizeof(int));
    s->rstart = clock();
    for (size_t j = 0; j < NLOOP; j++) {
        for (size_t i = 0; i < count;) {
            size_t r = io_buffer_read(s->io, (char*)&arr[i], (count-i)*sizeof(int));
            i += (r>>2);
            if (r) ops++; else errs++;
        }
        for (size_t i = 0; i < count; i++) {
            sum += arr[i];
        }
    }
    s->rend = clock();
    s->rops = ops;
    s->rerrs = errs;
    s->rsum = sum;

    return 0;
}

void io_print_results(test_state s)
{
    double s1, s2, s3, s4;
    char name[32];

    s1 = (1e9 * (s.wend - s.wstart)) / ((double)CLOCKS_PER_SEC * s.wops);
    s2 = (1e9 * (s.rend - s.rstart)) / ((double)CLOCKS_PER_SEC * s.rops);
    s3 = (1e9 * (s.wend - s.wstart)) / ((double)CLOCKS_PER_SEC * s.count * NLOOP);
    s4 = (1e9 * (s.rend - s.rstart)) / ((double)CLOCKS_PER_SEC * s.count * NLOOP);

    snprintf(name, sizeof(name), "%zu-byte", s.bufsize);

    printf("\nwsum 0x%016zx ( %7.2f sec ) rsum 0x%016zx ( %7.2f sec )\n",
        s.wsum, (double)(s.wend - s.wstart) / CLOCKS_PER_SEC,
        s.rsum, (double)(s.rend - s.rstart) / CLOCKS_PER_SEC);
    printf("\n%12s %12s %15s %12s %12s %12s\n", name, "ops",
        "errors", "time", "message/sec", "MB/sec");
    printf("%12s %12s %15s %12s %12s %12s\n", "","------------",
        "---------------", "------------", "------------", "------------");
    printf("%12s %12zu %15zu %10.2fns %12zu %12.2f\n", "read",
        s.wops, s.werrs, s1, (size_t)(1e9/s1), sizeof(int)*(1e9/s3)/(1024*1024));
    printf("%12s %12zu %15zu %10.2fns %12zu %12.2f\n", "write",
        s.rops, s.rerrs, s2, (size_t)(1e9/s2), sizeof(int)*(1e9/s4)/(1024*1024));
}

void io_run_test(int bufsize)
{
    pipe_buffer pb;
    test_state s;
    thrd_t w_tid, r_tid;
    int r, res;

    pipe_buffer_init(&pb, bufsize);

    memset(&s, 0, sizeof(test_state));
    s.bufsize = bufsize;
    s.count = (1<<17) - 11;
    s.io = &pb.io;

    r = thrd_create(&w_tid, io_write_thread, &s);
    assert(r == 0);
    r = thrd_create(&r_tid, io_read_thread, &s);
    assert(r == 0);

    r = thrd_join(w_tid, &res);
    assert(r == 0);
    r = thrd_join(r_tid, &res);
    assert(r == 0);

    pipe_buffer_destroy(&pb);

    io_print_results(s);

    assert(s.wsum == s.rsum);
}

int main(int argc, const char **argv)
{
    printf("\n%s: %d write thread(s), %d read thread(s)\n",
        "test_002_pipe_buffer", NTHREAD, NTHREAD);

    io_run_test(4);
    io_run_test(16);
    io_run_test(64);
    io_run_test(4096);
    io_run_test(32768);

    printf("\n");
}
