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

typedef struct pipe_buffer pipe_buffer;

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
 * pipe buffer
 *
 * pipe buffer is a power of two sized circular buffer that provides
 * multithreaded read/write using read ahead and write ahead pointers.
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

typedef ushort pb_uoffset;
typedef atomic_ushort atomic_pb_uoffset;
typedef struct pb_offsets pb_offsets;

struct pb_offsets
{
    pb_uoffset start;
    pb_uoffset start_mark;
    pb_uoffset end;
    pb_uoffset end_mark;
};

struct pipe_buffer
{
    io_buffer io;
    atomic_size_t capacity;
    char *data;
    size_t _pad[4];
    atomic_ullong pof;
};

static io_buffer_ops pb_ops;

static ullong pb_pack_offsets(pb_offsets pbo)
{
    ullong mask = ((1ull << 16) - 1);
    return ((pbo.start & mask) << 0) |
        ((pbo.start_mark & mask) << 16) |
        ((pbo.end & mask) << 32) |
        ((pbo.end_mark & mask) << 48);
}

static pb_offsets pb_unpack_offsets(ullong pof)
{
    ullong mask = ((1ull << 16) - 1);
    pb_offsets pbo = {
        (pb_uoffset)((pof >> 0) & mask),
        (pb_uoffset)((pof >> 16) & mask),
        (pb_uoffset)((pof >> 32) & mask),
        (pb_uoffset)((pof >> 48) & mask)
    };
    return pbo;
}

static void pipe_buffer_init(pipe_buffer *pb, size_t capacity)
{
    assert(ispow2(capacity));
    assert(capacity < (1 << (sizeof(pb_uoffset) << 3)));
    pb_offsets pbo = { 0 };
    pb->io.ops = &pb_ops;
    pb->pof = pb_pack_offsets(pbo);
    pb->capacity = capacity;
    pb->data = (char*)malloc(capacity);
    memset(pb->data, 0, capacity);
}

static void pipe_buffer_destroy(pipe_buffer *pb)
{
    free(pb->data);
    pb->data = NULL;
}

static size_t pipe_buffer_capacity(pipe_buffer *pb)
{
    return pb->capacity;
}

static size_t pipe_buffer_read(pipe_buffer *pb, char *buf, size_t len)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset cap, mask, csz, fsz, io_len, start_mark, new_start_mark;

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pb_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pb_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end - pof.start_mark;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from start_mark to new_start_mark */
    io_len = len < fsz ? (pb_uoffset)len : fsz;
    start_mark = pof.start_mark;
    new_start_mark = pof.start_mark + io_len;

    if (io_len == 0) return 0;

    pb_debugf("start_mark=%u io_len=%u new_start_mark=%u",
        pof.start_mark, io_len, new_start_mark);

    /* compare swap start_mark <- new_start_mark. requires compare swap
     * due to buffer space invariant. uncontended if one reader. */
    pof.start_mark = new_start_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pb_pack_offsets(pof))) goto retry;

    /* perform copy out, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((start_mark & ~mask) == ((new_start_mark - 1) & ~mask)) {
        memcpy(buf, pb->data + (start_mark & mask), io_len);
    } else {
        pb_uoffset o1 = (start_mark & mask);
        pb_uoffset l1 = (new_start_mark & ~mask) - start_mark;
        memcpy(buf, pb->data + o1, l1);
        memcpy(buf + l1, pb->data, io_len - l1);
    }

    /* spin until start == start_mark for reads before us to complete
     * and store start <- new_start_mark. uncontended if one reader. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pb_unpack_offsets(pof_val);
        pof.start = start_mark;
        if (pof_val != pb_pack_offsets(pof)) continue;
        pof_val = pb_pack_offsets(pof);
        pof.start = new_start_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pb_pack_offsets(pof))) break;
    }

    return io_len;
}

static size_t pipe_buffer_write(pipe_buffer *pb, char *buf, size_t len)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset cap, mask, csz, fsz, io_len, end_mark, new_end_mark;

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return 0;

    cap = (pb_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pb_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end_mark - pof.start;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from end_mark to new_end_mark */
    io_len = len < cap - fsz ? (pb_uoffset)len : cap - fsz;
    end_mark = pof.end_mark;
    new_end_mark = pof.end_mark + io_len;

    if (io_len == 0) return 0;

    pb_debugf("end_mark=%u io_len=%u new_end_mark=%u",
        pof.end_mark, io_len, new_end_mark);

    /* compare swap end_mark <- new_end_mark. requires compare swap
     * due to buffer space invariant. uncontended if one writer. */
    pof.end_mark = new_end_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pb_pack_offsets(pof))) goto retry;

    /* perform copy in, and if we wrap split into two copies
     * while also applying the modulus to the buffer markers. */
    if ((end_mark & ~mask) == ((new_end_mark - 1) & ~mask)) {
        memcpy(pb->data + (end_mark & mask), buf, io_len);
    } else {
        pb_uoffset o1 = (end_mark & mask);
        pb_uoffset l1 = (new_end_mark & ~mask) - end_mark;
        memcpy(pb->data + o1, buf, l1);
        memcpy(pb->data, buf + l1, io_len - l1);
    }

    /* spin until end == end_mark for writes before us to complete
     * and store end <- new_end_mark. uncontended if one writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pb_unpack_offsets(pof_val);
        pof.end = end_mark;
        if (pof_val != pb_pack_offsets(pof)) continue;
        pof_val = pb_pack_offsets(pof);
        pof.end = new_end_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pb_pack_offsets(pof))) break;
    }

    return io_len;
}

static io_span pipe_buffer_read_lock(pipe_buffer *pb, size_t len)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset cap, mask, csz, fsz, io_len, start_mark, new_start_mark;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pb_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pb_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end - pof.start_mark;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from start_mark to new_start_mark */
    io_len = len < fsz ? (pb_uoffset)len : fsz;
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
     * due to buffer space invariant. uncontended if one reader. */
    pof.start_mark = new_start_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pb_pack_offsets(pof))) goto retry;

    ticket.buf = pb->data + (start_mark & mask);
    ticket.length = io_len;
    ticket.sequence = start_mark;

    return ticket;
}

static io_span pipe_buffer_write_lock(pipe_buffer *pb, size_t len)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset cap, mask, csz, fsz, io_len, end_mark, new_end_mark;
    io_span ticket = { 0, 0, 0 };

    /*                  start                   end                       *
     *                  |       start_mark      |       end_mark          *
     *  ________________--------XXXXXXXXXXXXXXXX++++++++________________  *
     *                                                                    *
     *                  end+cap                 start                     *
     *                  |       end_mark+cap    |       start_mark        *
     *  XXXXXXXXXXXXXXXX++++++++________________--------XXXXXXXXXXXXXXXX  */

    if (len == 0) return ticket;

    cap = (pb_uoffset)atomic_load_explicit(&pb->capacity, memory_order_relaxed);
    mask = cap - 1;

retry:
    /* fetch buffer markers */
    pof_val = atomic_load_explicit(&pb->pof, memory_order_relaxed);
    pof = pb_unpack_offsets(pof_val);

    /* ensure buffer marker invariants */
    csz = pof.end - pof.start;
    fsz = pof.end_mark - pof.start;
    assert(csz <= cap);
    assert(fsz <= cap);

    /* calculate copy length from end_mark to new_end_mark */
    io_len = len < cap - fsz ? (pb_uoffset)len : cap - fsz;
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
     * due to buffer space invariant. uncontended if one writer. */
    pof.end_mark = new_end_mark;
    if (!atomic_compare_exchange_strong(&pb->pof, &pof_val,
        pb_pack_offsets(pof))) goto retry;

    ticket.buf = pb->data + (end_mark & mask);
    ticket.length = io_len;
    ticket.sequence = end_mark;

    return ticket;
}

static int pipe_buffer_read_commit(pipe_buffer *pb, io_span ticket)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset start_mark, new_start_mark;

    if (ticket.length == 0) return 0;

    start_mark = (pb_uoffset)ticket.sequence;
    new_start_mark = (pb_uoffset)(ticket.sequence + ticket.length);

    /* spin until start == start_mark for reads before us to complete
     * and store start <- new_start_mark. uncontended if one reader. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pb_unpack_offsets(pof_val);
        pof.start = start_mark;
        if (pof_val != pb_pack_offsets(pof)) continue;
        pof_val = pb_pack_offsets(pof);
        pof.start = new_start_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pb_pack_offsets(pof))) break;
    }

    return 0;
}

static int pipe_buffer_write_commit(pipe_buffer *pb, io_span ticket)
{
    ullong pof_val;
    pb_offsets pof;
    pb_uoffset end_mark, new_end_mark;

    if (ticket.length == 0) return 0;

    end_mark = (pb_uoffset)ticket.sequence;
    new_end_mark = (pb_uoffset)(ticket.sequence + ticket.length);

    /* spin until end == end_mark for writes before us to complete
     * and store end <- new_end_mark. uncontended if one writer. */
    for (;;) {
        pof_val = atomic_load_explicit(&pb->pof, memory_order_acquire);
        pof = pb_unpack_offsets(pof_val);
        pof.end = end_mark;
        if (pof_val != pb_pack_offsets(pof)) continue;
        pof_val = pb_pack_offsets(pof);
        pof.end = new_end_mark;
        if (atomic_compare_exchange_strong(&pb->pof, &pof_val,
            pb_pack_offsets(pof))) break;
    }

    return 0;
}

static io_buffer_ops pb_ops =
{
    (io_read_fn *)pipe_buffer_read,
    (io_write_fn *)pipe_buffer_write,
    (io_read_lock_fn *)pipe_buffer_read_lock,
    (io_write_lock_fn *)pipe_buffer_write_lock,
    (io_read_commit_fn *)pipe_buffer_read_commit,
    (io_write_commit_fn *)pipe_buffer_write_commit
};
