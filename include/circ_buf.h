/* SPDX-License-Identifier: GPL-2.0 */
/* Code adopted from the linux kernel, include/linux/circ_buf.h
 * Macros for operations on the power of two buffer.
 */

#ifndef _LINUX_CIRC_BUF_H
#define _LINUX_CIRC_BUF_H 1

struct circ_buf {
	char *buf;
	int head;
	int tail;
};

static inline void circ_advance_head(struct circ_buf *cb, int size)
{
	cb->head = (cb->head + 1) & (size - 1);
}

static inline void circ_advance_tail(struct circ_buf *cb, int size)
{
	cb->tail = (cb->tail + 1) & (size - 1);
}

/* Return the number of elements in the buffer. */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char as a
 * completely full buffer has head == tail, which is the same as empty.
 */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

/* Return the number of elements you can consecutively read before wrapping.
 * Another words, consecutive readable data. Carefully avoid accessing head
 * and tail more than once, so they can change underneath us without returning
 * inconsistent results.
 */
#define CIRC_CNT_TO_END(head,tail,size)        \
	({int end = (size) - (tail);           \
	  int n = ((head) + end) & ((size)-1); \
	  n < end ? n : end;})

/* Return the number of elements you can consecutively write before wrapping.
 * Another words, contiguous writable space.
 */
#define CIRC_SPACE_TO_END(head,tail,size)      \
	({int end = (size) - 1 - (head);       \
	  int n = (end + (tail)) & ((size)-1); \
	  n <= end ? n : end+1;})

#endif /* _LINUX_CIRC_BUF_H  */
