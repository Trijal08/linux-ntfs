/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * NTFS compatibility helpers for older kernel APIs.
 */

#ifndef _LINUX_NTFS_COMPAT_H
#define _LINUX_NTFS_COMPAT_H

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/ratelimit.h>
#include <linux/highmem.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/uio.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/sched/signal.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
#define FSLABEL_MAX 256
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
#ifndef FS_IOC_GETFSLABEL
#define FS_IOC_GETFSLABEL	_IOR(0x94, 49, char[FSLABEL_MAX])
#endif
#ifndef FS_IOC_SETFSLABEL
#define FS_IOC_SETFSLABEL	_IOW(0x94, 50, char[FSLABEL_MAX])
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
#ifndef FMODE_NOWAIT
#define FMODE_NOWAIT 0
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 5, 0)
#ifndef FMODE_CAN_ODIRECT
#define FMODE_CAN_ODIRECT 0
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
#ifndef STATX_DIOALIGN
#define STATX_DIOALIGN 0
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static inline unsigned int bdev_max_discard_sectors(struct block_device *bdev)
{
	struct request_queue *q = bdev_get_queue(bdev);

	return q ? q->limits.max_discard_sectors : 0;
}

static inline unsigned int bdev_discard_granularity(struct block_device *bdev)
{
	struct request_queue *q = bdev_get_queue(bdev);

	return q ? q->limits.discard_granularity : 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
static inline int file_modified(struct file *file)
{
	return file_update_time(file);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0)
static inline void *kvrealloc(const void *p, size_t oldsize, size_t newsize,
		gfp_t flags)
{
	return krealloc(p, newsize, flags);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
#define folio page
#define __folio_index index

static inline struct page *page_folio(struct page *page)
{
	return page;
}

static inline struct page *folio_page(struct page *page, int n)
{
	return page + n;
}

static inline pgoff_t folio_pos(struct page *page)
{
	return page_offset(page);
}

static inline unsigned int folio_size(struct page *page)
{
	return PAGE_SIZE;
}

static inline unsigned int folio_shift(struct page *page)
{
	return PAGE_SHIFT;
}

static inline bool folio_test_uptodate(struct page *page)
{
	return PageUptodate(page);
}

static inline bool folio_test_locked(struct page *page)
{
	return PageLocked(page);
}

static inline void folio_mark_uptodate(struct page *page)
{
	SetPageUptodate(page);
}

static inline void folio_clear_uptodate(struct page *page)
{
	ClearPageUptodate(page);
}

static inline void folio_mark_dirty(struct page *page)
{
	set_page_dirty(page);
}

static inline void folio_clear_dirty(struct page *page)
{
	clear_page_dirty_for_io(page);
}

static inline void folio_lock(struct page *page)
{
	lock_page(page);
}

static inline void folio_unlock(struct page *page)
{
	unlock_page(page);
}

static inline void folio_get(struct page *page)
{
	get_page(page);
}

static inline void folio_put(struct page *page)
{
	put_page(page);
}

static inline void *kmap_local_folio(struct page *page, size_t offset)
{
	return kmap_atomic(page) + offset;
}

static inline void *kmap_local_page(struct page *page)
{
	return kmap_atomic(page);
}

static inline void kunmap_local(void *addr)
{
	kunmap_atomic(addr);
}

static inline struct page *read_mapping_folio(struct address_space *mapping,
		pgoff_t index, void *data)
{
	return read_mapping_page(mapping, index, data);
}

static inline struct page *filemap_lock_folio(struct address_space *mapping,
		pgoff_t index)
{
	return find_lock_page(mapping, index);
}

static inline void folio_zero_segment(struct page *page,
		unsigned int from, unsigned int to)
{
	zero_user_segment(page, from, to);
}

static inline void folio_zero_segments(struct page *page,
		unsigned int from1, unsigned int to1,
		unsigned int from2, unsigned int to2)
{
	zero_user_segments(page, from1, to1, from2, to2);
}

static inline void folio_fill_tail(struct page *page, unsigned int from,
		const void *src, size_t len)
{
	void *kaddr = kmap_atomic(page);

	memcpy(kaddr + from, src, len);
	kunmap_atomic(kaddr);
}

static inline size_t copy_folio_from_iter_atomic(struct page *page,
		size_t offset, size_t bytes, struct iov_iter *i)
{
	return iov_iter_copy_from_user_atomic(page, i, offset, bytes);
}

static inline size_t copy_page_from_iter_atomic(struct page *page,
		size_t offset, size_t bytes, struct iov_iter *i)
{
	return iov_iter_copy_from_user_atomic(page, i, offset, bytes);
}

static inline int fault_in_iov_iter_readable(const struct iov_iter *i,
		size_t bytes)
{
	return iov_iter_fault_in_readable((struct iov_iter *)i, bytes);
}

static inline void folio_start_writeback(struct page *page)
{
	set_page_writeback(page);
}

static inline void folio_end_writeback(struct page *page)
{
	end_page_writeback(page);
}

static inline bool bio_add_folio(struct bio *bio, struct page *page,
		unsigned int len, unsigned int offset)
{
	return bio_add_page(bio, page, len, offset) == len;
}

static inline void bio_add_folio_nofail(struct bio *bio, struct page *page,
		unsigned int len, unsigned int offset)
{
	BUG_ON(bio_add_page(bio, page, len, offset) != len);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 1, 0)
static inline void filemap_invalidate_lock(struct address_space *mapping)
{
}

static inline void filemap_invalidate_unlock(struct address_space *mapping)
{
}
#endif

#endif /* _LINUX_NTFS_COMPAT_H */
