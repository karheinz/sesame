#ifndef JVIRT_ARRAYS_H
#define JVIRT_ARRAYS_H

/*
 * The control blocks for virtual arrays.
 * Note that these blocks are allocated in the "small" pool area.
 * System-dependent info for the associated backing store (if any) is hidden
 * inside the backing_store_info struct.
 */
GLOBAL(JBLOCKARRAY)
access_virt_barray (j_common_ptr cinfo, jvirt_barray_ptr ptr,
                    JDIMENSION start_row, JDIMENSION num_rows,
                    boolean writable);

struct jvirt_sarray_control {
  JSAMPARRAY mem_buffer;        /* => the in-memory buffer */
  JDIMENSION rows_in_array;     /* total virtual array height */
  JDIMENSION samplesperrow;     /* width of array (and of memory buffer) */
  JDIMENSION maxaccess;         /* max rows accessed by access_virt_sarray */
  JDIMENSION rows_in_mem;       /* height of memory buffer */
  JDIMENSION rowsperchunk;      /* allocation chunk size in mem_buffer */
  JDIMENSION cur_start_row;     /* first logical row # in the buffer */
  JDIMENSION first_undef_row;   /* row # of first uninitialized row */
  boolean pre_zero;             /* pre-zero mode requested? */
  boolean dirty;                /* do current buffer contents need written? */
  boolean b_s_open;             /* is backing-store data valid? */
  jvirt_sarray_ptr next;        /* link to next virtual sarray control block */
  backing_store_info b_s_info;  /* System-dependent control info */
};

struct jvirt_barray_control {
  JBLOCKARRAY mem_buffer;       /* => the in-memory buffer */
  JDIMENSION rows_in_array;     /* total virtual array height */
  JDIMENSION blocksperrow;      /* width of array (and of memory buffer) */
  JDIMENSION maxaccess;         /* max rows accessed by access_virt_barray */
  JDIMENSION rows_in_mem;       /* height of memory buffer */
  JDIMENSION rowsperchunk;      /* allocation chunk size in mem_buffer */
  JDIMENSION cur_start_row;     /* first logical row # in the buffer */
  JDIMENSION first_undef_row;   /* row # of first uninitialized row */
  boolean pre_zero;             /* pre-zero mode requested? */
  boolean dirty;                /* do current buffer contents need written? */
  boolean b_s_open;             /* is backing-store data valid? */
  jvirt_barray_ptr next;        /* link to next virtual barray control block */
  backing_store_info b_s_info;  /* System-dependent control info */
};

#endif
