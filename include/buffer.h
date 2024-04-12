/*
 * concurrent pipe buffer
 *
 * PLEASE LICENSE, (C) 2024, Michael Clark <michaeljclark@mac.com>
 *
 * All rights to this work are granted for all purposes, with exception of
 * author's implied right of copyright to defend the free use of this work.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "bits.h"

typedef unsigned long long ullong;

/*
 * io buffer
 */

typedef struct io_span io_span;
typedef struct io_buffer_ops io_buffer_ops;
typedef struct io_buffer io_buffer;

struct io_span
{
    char* buf;
    size_t length;
    size_t sequence;
};

typedef size_t (io_read_fn)(io_buffer *io, char *buf, size_t len);
typedef size_t (io_write_fn)(io_buffer *io, char *buf, size_t len);
typedef io_span (io_read_lock_fn)(io_buffer *io, size_t len);
typedef io_span (io_write_lock_fn)(io_buffer *io, size_t len);
typedef int (io_read_commit_fn)(io_buffer *io, io_span ticket);
typedef int (io_write_commit_fn)(io_buffer *io, io_span ticket);

struct io_buffer_ops
{
    io_read_fn *read;
    io_write_fn *write;
    io_read_lock_fn *read_lock;
    io_write_lock_fn *write_lock;
    io_read_commit_fn *read_commit;
    io_write_commit_fn *write_commit;
};

struct io_buffer
{
    io_buffer_ops *ops;
};

static size_t io_buffer_read(io_buffer *io, char *buf, size_t len)
{
    return io->ops->read(io, buf, len); 
}

static size_t io_buffer_write(io_buffer *io, char *buf, size_t len)
{
    return io->ops->write(io, buf, len); 
}

static io_span io_buffer_read_lock(io_buffer *io, size_t len)
{
    return io->ops->read_lock(io, len); 
}

static io_span io_buffer_write_lock(io_buffer *io, size_t len)
{
    return io->ops->write_lock(io, len); 
}

static int io_buffer_read_commit(io_buffer *io, io_span ticket)
{
    return io->ops->read_commit(io, ticket); 
}

static int io_buffer_write_commit(io_buffer *io, io_span ticket)
{
    return io->ops->write_commit(io, ticket); 
}

/*
 * pipe buffer debug
 */

//#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#define VA_ARGS(...) , ##__VA_ARGS__
#define pb_debug(fmt,...) printf(fmt "\n" VA_ARGS(__VA_ARGS__))
#define pb_debugf(fmt,...) printf("%s: " fmt "\n", __func__ VA_ARGS(__VA_ARGS__))
#else
#define pb_debugf(fmt,...)
#endif

/*
 * single producer single consumer pipe buffer
 *
 * pipe buffer is a power of two sized circular buffer that provides
 * single-threaded read/write using read ahead and write ahead pointers.
 */

typedef ullong pbs_uoffset;
typedef atomic_ullong atomic_pbs_uoffset;
typedef struct pbs_buffer pbs_buffer;

struct pbs_buffer
{
    io_buffer io;
    atomic_size_t capacity;
    char *data;
    atomic_pbs_uoffset start;
    atomic_pbs_uoffset end;
};

static io_buffer_ops pbs_ops;

static void pbs_buffer_init(pbs_buffer *pb, size_t capacity)
{
    assert(ispow2(capacity));
    pb->io.ops = &pbs_ops;
    pb->start = 0;
    pb->end = 0;
    pb->capacity = capacity;
    pb->data = (char*)malloc(capacity);
    memset(pb->data, 0, capacity);
}

static void pbs_buffer_destroy(pbs_buffer *pb)
{
    free(pb->data);
    pb->data = NULL;
}

static size_t pbs_buffer_capacity(pbs_buffer *pb)
{
    return pb->capacity;
}

static size_t pbs_buffer_read(pbs_buffer *pb, char *buf, size_t len)
{
    pbs_uoffset cap, mask, csz, io_len, start, new_start, end;

    /*                  start                   end                       *
     *                  |       start           |       end               *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end+cap         |       start             *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pbs_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

    /* fetch buffer markers */
    start = atomic_load_explicit(&pb->start, memory_order_acquire);
    end = atomic_load_explicit(&pb->end, memory_order_relaxed);

    /* ensure buffer marker invariants */
    csz = end - start;
    assert(csz <= cap);

    /* calculate copy length from start to new_start */
    io_len = len < csz ? (pbs_uoffset)len : csz;
    start = start;
    new_start = start + io_len;

    if (io_len == 0) return 0;

    pb_debugf("start=%u io_len=%u new_start=%u",
        start, io_len, new_start);

    /* perform copy out, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((start & ~mask) == ((new_start - 1) & ~mask)) {
        memcpy(buf, pb->data + (start & mask), io_len);
    } else {
        pbs_uoffset o1 = (start & mask);
        pbs_uoffset l1 = (new_start & ~mask) - start;
        memcpy(buf, pb->data + o1, l1);
        memcpy(buf + l1, pb->data, io_len - l1);
    }

    /* store start <- new_start. */
    atomic_store_explicit(&pb->start, new_start, memory_order_release);

    return io_len;
}

static size_t pbs_buffer_write(pbs_buffer *pb, char *buf, size_t len)
{
    pbs_uoffset cap, mask, csz, io_len, start, end, new_end;

    /*                  start                   end                       *
     *                  |       start           |       end               *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end+cap         |       start             *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pbs_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

    /* fetch buffer markers */
    start = atomic_load_explicit(&pb->start, memory_order_relaxed);
    end = atomic_load_explicit(&pb->end, memory_order_acquire);

    /* ensure buffer marker invariants */
    csz = end - start;
    assert(csz <= cap);

    /* calculate copy length from end to new_end */
    io_len = len < cap - csz ? (pbs_uoffset)len : cap - csz;
    end = end;
    new_end = end + io_len;

    if (io_len == 0) return 0;

    pb_debugf("end=%u io_len=%u new_end=%u",
        end, io_len, new_end);

    /* perform copy in, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((end & ~mask) == ((new_end - 1) & ~mask)) {
        memcpy(pb->data + (end & mask), buf, io_len);
    } else {
        pbs_uoffset o1 = (end & mask);
        pbs_uoffset l1 = (new_end & ~mask) - end;
        memcpy(pb->data + o1, buf, l1);
        memcpy(pb->data, buf + l1, io_len - l1);
    }

    /* store end <- new_end. */
    atomic_store_explicit(&pb->end, new_end, memory_order_release);

    return io_len;
}

static io_span pbs_buffer_read_lock(pbs_buffer *pb, size_t len)
{
    pbs_uoffset cap, mask, csz, io_len, start, new_start, end;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start           |       end               *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end+cap         |       start             *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pbs_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

    /* fetch buffer markers */
    start = atomic_load_explicit(&pb->start, memory_order_acquire);
    end = atomic_load_explicit(&pb->end, memory_order_relaxed);

    /* ensure buffer marker invariants */
    csz = end - start;
    assert(csz <= cap);

    /* calculate copy length from start to new_start */
    io_len = len < csz ? (pbs_uoffset)len : csz;
    start = start;
    new_start = start + io_len;

    if ((start & ~mask) != ((new_start - 1) & ~mask)) {
        io_len = (new_start & ~mask) - start;
        new_start = start + io_len;
    }

    if (io_len == 0) return ticket;

    pb_debugf("start=%u io_len=%u new_start=%u",
        start, io_len, new_start);

    ticket.buf = pb->data + (start & mask);
    ticket.length = io_len;
    ticket.sequence = start;

    return ticket;
}

static io_span pbs_buffer_write_lock(pbs_buffer *pb, size_t len)
{
    pbs_uoffset cap, mask, csz, io_len, start, end, new_end;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start           |       end               *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end+cap         |       start             *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pbs_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

    /* fetch buffer markers */
    start = atomic_load_explicit(&pb->start, memory_order_relaxed);
    end = atomic_load_explicit(&pb->end, memory_order_acquire);

    /* ensure buffer marker invariants */
    csz = end - start;
    assert(csz <= cap);

    /* calculate copy length from end to new_end */
    io_len = len < cap - csz ? (pbs_uoffset)len : cap - csz;
    end = end;
    new_end = end + io_len;

    if ((end & ~mask) != ((new_end - 1) & ~mask)) {
        io_len = (new_end & ~mask) - end;
        new_end = end + io_len;
    }

    if (io_len == 0) return ticket;

    pb_debugf("end=%u io_len=%u new_end=%u",
        end, io_len, new_end);

    ticket.buf = pb->data + (end & mask);
    ticket.length = io_len;
    ticket.sequence = end;

    return ticket;
}

static int pbs_buffer_read_commit(pbs_buffer *pb, io_span ticket)
{
    pbs_uoffset start, new_start;

    if (ticket.length == 0) return 0;

    start = (pbs_uoffset)ticket.sequence;
    new_start = (pbs_uoffset)(ticket.sequence + ticket.length);

    /* store start <- new_start. */
    atomic_store_explicit(&pb->start, new_start, memory_order_release);

    return 0;
}

static int pbs_buffer_write_commit(pbs_buffer *pb, io_span ticket)
{
    pbs_uoffset end, new_end;

    if (ticket.length == 0) return 0;

    end = (pbs_uoffset)ticket.sequence;
    new_end = (pbs_uoffset)(ticket.sequence + ticket.length);

    /* store end <- new_end. */
    atomic_store_explicit(&pb->end, new_end, memory_order_release);

    return 0;
}

static io_buffer_ops pbs_ops =
{
    (io_read_fn *)pbs_buffer_read,
    (io_write_fn *)pbs_buffer_write,
    (io_read_lock_fn *)pbs_buffer_read_lock,
    (io_write_lock_fn *)pbs_buffer_write_lock,
    (io_read_commit_fn *)pbs_buffer_read_commit,
    (io_write_commit_fn *)pbs_buffer_write_commit
};

/*
 * multiple producer multiple consumer pipe buffer
 *
 * pipe buffer is a power of two sized circular buffer that provides
 * multi-threaded read/write using read ahead and write ahead pointers.
 */

typedef ushort pbm_uoffset;
typedef atomic_ushort atomic_pbm_uoffset;
typedef struct pbm_offsets pbm_offsets;
typedef struct pbm_buffer pbm_buffer;

struct pbm_offsets
{
    pbm_uoffset start;
    pbm_uoffset start_mark;
    pbm_uoffset end;
    pbm_uoffset end_mark;
};

struct pbm_buffer
{
    io_buffer io;
    atomic_size_t capacity;
    char *data;
    size_t _pad[5];
    atomic_ullong pof;
};

static io_buffer_ops pbm_ops;

static ullong pbm_pack_offsets(pbm_offsets pbo)
{
    ullong mask = ((1ull << 16) - 1);
    return ((pbo.start & mask) << 0) |
        ((pbo.start_mark & mask) << 16) |
        ((pbo.end & mask) << 32) |
        ((pbo.end_mark & mask) << 48);
}

static pbm_offsets pbm_unpack_offsets(ullong pof)
{
    ullong mask = ((1ull << 16) - 1);
    pbm_offsets pbo = {
        (pbm_uoffset)((pof >> 0) & mask),
        (pbm_uoffset)((pof >> 16) & mask),
        (pbm_uoffset)((pof >> 32) & mask),
        (pbm_uoffset)((pof >> 48) & mask)
    };
    return pbo;
}

static void pbm_buffer_init(pbm_buffer *pb, size_t capacity)
{
    assert(ispow2(capacity));
    assert(capacity < (1ull << (sizeof(pbm_uoffset) << 3)));
    pbm_offsets pbo = { 0 };
    pb->io.ops = &pbm_ops;
    pb->pof = pbm_pack_offsets(pbo);
    pb->capacity = capacity;
    pb->data = (char*)malloc(capacity);
    memset(pb->data, 0, capacity);
}

static void pbm_buffer_destroy(pbm_buffer *pb)
{
    free(pb->data);
    pb->data = NULL;
}

static size_t pbm_buffer_capacity(pbm_buffer *pb)
{
    return pb->capacity;
}

static size_t pbm_buffer_read(pbm_buffer *pb, char *buf, size_t len)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset cap, mask, csz, fsz, io_len, start_mark, new_start_mark;

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pbm_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pbm_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end - pof.start_mark;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from start_mark to new_start_mark */
    io_len = len < fsz ? (pbm_uoffset)len : fsz;
    start_mark = pof.start_mark;
    new_start_mark = pof.start_mark + io_len;

    if (io_len == 0) return 0;

    pb_debugf("start_mark=%u io_len=%u new_start_mark=%u",
        pof.start_mark, io_len, new_start_mark);

    /* compare swap start_mark <- new_start_mark. requires compare swap
     * due to buffer space invariant. uncontended if one reader/writer. */
    pof.start_mark = new_start_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pbm_pack_offsets(pof))) goto retry;

    /* perform copy out, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((start_mark & ~mask) == ((new_start_mark - 1) & ~mask)) {
        memcpy(buf, pb->data + (start_mark & mask), io_len);
    } else {
        pbm_uoffset o1 = (start_mark & mask);
        pbm_uoffset l1 = (new_start_mark & ~mask) - start_mark;
        memcpy(buf, pb->data + o1, l1);
        memcpy(buf + l1, pb->data, io_len - l1);
    }

    /* spin until start == start_mark for reads before us to complete
     * and store start <- new_start_mark. uncontended if one reader/writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pbm_unpack_offsets(pof_val);
        pof.start = start_mark;
        pof_val = pbm_pack_offsets(pof);
        pof.start = new_start_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pbm_pack_offsets(pof))) break;
    }

    return io_len;
}

static size_t pbm_buffer_write(pbm_buffer *pb, char *buf, size_t len)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset cap, mask, csz, fsz, io_len, end_mark, new_end_mark;

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pbm_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pbm_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end_mark - pof.start;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from end_mark to new_end_mark */
    io_len = len < cap - fsz ? (pbm_uoffset)len : cap - fsz;
    end_mark = pof.end_mark;
    new_end_mark = pof.end_mark + io_len;

    if (io_len == 0) return 0;

    pb_debugf("end_mark=%u io_len=%u new_end_mark=%u",
        pof.end_mark, io_len, new_end_mark);

    /* compare swap end_mark <- new_end_mark. requires compare swap
     * due to buffer space invariant. uncontended if one reader/writer. */
    pof.end_mark = new_end_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pbm_pack_offsets(pof))) goto retry;

    /* perform copy in, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((end_mark & ~mask) == ((new_end_mark - 1) & ~mask)) {
        memcpy(pb->data + (end_mark & mask), buf, io_len);
    } else {
        pbm_uoffset o1 = (end_mark & mask);
        pbm_uoffset l1 = (new_end_mark & ~mask) - end_mark;
        memcpy(pb->data + o1, buf, l1);
        memcpy(pb->data, buf + l1, io_len - l1);
    }

    /* spin until end == end_mark for writes before us to complete
     * and store end <- new_end_mark. uncontended if one reader/writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pbm_unpack_offsets(pof_val);
        pof.end = end_mark;
        pof_val = pbm_pack_offsets(pof);
        pof.end = new_end_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pbm_pack_offsets(pof))) break;
    }

    return io_len;
}

static io_span pbm_buffer_read_lock(pbm_buffer *pb, size_t len)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset cap, mask, csz, fsz, io_len, start_mark, new_start_mark;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pbm_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pbm_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end - pof.start_mark;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from start_mark to new_start_mark */
    io_len = len < fsz ? (pbm_uoffset)len : fsz;
    start_mark = pof.start_mark;
    new_start_mark = pof.start_mark + io_len;

    if ((start_mark & ~mask) != ((new_start_mark - 1) & ~mask)) {
        io_len = (new_start_mark & ~mask) - start_mark;
        new_start_mark = start_mark + io_len;
    }

    if (io_len == 0) return ticket;

    pb_debugf("start_mark=%u io_len=%u new_start_mark=%u",
        pof.start_mark, io_len, new_start_mark);

    /* compare swap start_mark <- new_start_mark. requires compare swap
     * due to buffer space invariant. uncontended if one reader/writer. */
    pof.start_mark = new_start_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pbm_pack_offsets(pof))) goto retry;

    ticket.buf = pb->data + (start_mark & mask);
    ticket.length = io_len;
    ticket.sequence = start_mark;

    return ticket;
}

static io_span pbm_buffer_write_lock(pbm_buffer *pb, size_t len)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset cap, mask, csz, fsz, io_len, end_mark, new_end_mark;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pbm_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pbm_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end_mark - pof.start;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from end_mark to new_end_mark */
    io_len = len < cap - fsz ? (pbm_uoffset)len : cap - fsz;
    end_mark = pof.end_mark;
    new_end_mark = pof.end_mark + io_len;

    if ((end_mark & ~mask) != ((new_end_mark - 1) & ~mask)) {
        io_len = (new_end_mark & ~mask) - end_mark;
        new_end_mark = end_mark + io_len;
    }

    if (io_len == 0) return ticket;

    pb_debugf("end_mark=%u io_len=%u new_end_mark=%u",
        pof.end_mark, io_len, new_end_mark);

    /* compare swap end_mark <- new_end_mark. requires compare swap
     * due to buffer space invariant. uncontended if one reader/writer. */
    pof.end_mark = new_end_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pbm_pack_offsets(pof))) goto retry;

    ticket.buf = pb->data + (end_mark & mask);
    ticket.length = io_len;
    ticket.sequence = end_mark;

    return ticket;
}

static int pbm_buffer_read_commit(pbm_buffer *pb, io_span ticket)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset start_mark, new_start_mark;

    if (ticket.length == 0) return 0;

    start_mark = (pbm_uoffset)ticket.sequence;
    new_start_mark = (pbm_uoffset)(ticket.sequence + ticket.length);

    /* spin until start == start_mark for reads before us to complete
     * and store start <- new_start_mark. uncontended if one reader/writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pbm_unpack_offsets(pof_val);
        pof.start = start_mark;
        pof_val = pbm_pack_offsets(pof);
        pof.start = new_start_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pbm_pack_offsets(pof))) break;
    }

    return 0;
}

static int pbm_buffer_write_commit(pbm_buffer *pb, io_span ticket)
{
    ullong pof_val;
    pbm_offsets pof;
    pbm_uoffset end_mark, new_end_mark;

    if (ticket.length == 0) return 0;

    end_mark = (pbm_uoffset)ticket.sequence;
    new_end_mark = (pbm_uoffset)(ticket.sequence + ticket.length);

    /* spin until end == end_mark for writes before us to complete
     * and store end <- new_end_mark. uncontended if one reader/writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pbm_unpack_offsets(pof_val);
        pof.end = end_mark;
        pof_val = pbm_pack_offsets(pof);
        pof.end = new_end_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pbm_pack_offsets(pof))) break;
    }

    return 0;
}

static io_buffer_ops pbm_ops =
{
    (io_read_fn *)pbm_buffer_read,
    (io_write_fn *)pbm_buffer_write,
    (io_read_lock_fn *)pbm_buffer_read_lock,
    (io_write_lock_fn *)pbm_buffer_write_lock,
    (io_read_commit_fn *)pbm_buffer_read_commit,
    (io_write_commit_fn *)pbm_buffer_write_commit
};
