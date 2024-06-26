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

#define NLOOP 64
#define NTHREAD 4

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
            for (; l < count && l < i + (bufsize>>2); l++) {
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

static void io_run_test(int bufsize)
{
    pbm_buffer pb;
    test_state s[NTHREAD], t;
    thrd_t w_tid[NTHREAD], r_tid[NTHREAD];
    int r, res;

    pbm_buffer_init(&pb, bufsize);

    for (size_t i = 0; i < NTHREAD; i++) {
        memset(&s[i], 0, sizeof(test_state));
        s[i].bufsize = bufsize;
        s[i].count = (1<<17) - 11;
        s[i].io = &pb.io;
    }

    for (size_t i = 0; i < NTHREAD; i++) {
        r = thrd_create(&r_tid[i], io_read_thread, &s[i]);
        assert(r == 0);
        r = thrd_create(&w_tid[i], io_write_thread, &s[i]);
        assert(r == 0);
    }

    for (size_t i = 0; i < NTHREAD; i++) {
        r = thrd_join(w_tid[i], &res);
        assert(r == 0);
        r = thrd_join(r_tid[i], &res);
        assert(r == 0);
    }

    pbm_buffer_destroy(&pb);

    memset(&t, 0, sizeof(test_state));
    t.bufsize = bufsize;

    for (size_t i = 0; i < NTHREAD; i++) {
        t.count += s[i].count;
        t.wops += s[i].wops;
        t.rops += s[i].rops;
        t.werrs += s[i].werrs;
        t.rerrs += s[i].rerrs;
        t.wsum += s[i].wsum;
        t.rsum += s[i].rsum;
        if (t.wstart == 0 || t.wstart > s[i].wstart) t.wstart = s[i].wstart;
        if (t.rstart == 0 || t.rstart > s[i].rstart) t.rstart = s[i].rstart;
        if (t.wend == 0 || t.wend < s[i].wend) t.wend = s[i].wend;
        if (t.rend == 0 || t.rend < s[i].rend) t.rend = s[i].rend;
    }

    io_print_results(t, NLOOP);

    assert(t.wsum == t.rsum);
}

int main(int argc, const char **argv)
{
    printf("\n# %s: %d write thread(s) %d read thread(s)\n",
        "test_003_pbm_buffer", NTHREAD, NTHREAD);
    printf("# os: %s cpu: %s\n", get_os_name(), get_cpu_name());

    io_run_test(4);
    io_run_test(16);
    io_run_test(64);
    io_run_test(4096);
    io_run_test(32768);

    printf("\n");
}
