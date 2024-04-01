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
#include "common.h"

#define NLOOP 256
#define NTHREAD 1

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

    io_print_results(s, NLOOP);

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
