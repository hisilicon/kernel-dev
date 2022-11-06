// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES. All rights reserved
 */

#include <linux/device.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vfio.h>
#include <linux/sched/mm.h>
#include <linux/anon_inodes.h>

#include "cmd.h"

/* Arbitrary to prevent userspace from consuming endless memory */
#define MAX_MIGRATION_SIZE (512*1024*1024)

static struct mlx5vf_pci_core_device *mlx5vf_drvdata(struct pci_dev *pdev)
{
	struct vfio_pci_core_device *core_device = dev_get_drvdata(&pdev->dev);

	return container_of(core_device, struct mlx5vf_pci_core_device,
			    core_device);
}

static struct page *
mlx5vf_get_migration_page(struct mlx5_vf_migration_file *migf,
			  unsigned long offset, struct sg_append_table *table)
{
	unsigned long cur_offset = 0;
	struct scatterlist *sg;
	unsigned int i;

	/* All accesses are sequential */
	if (offset < migf->last_offset || !migf->last_offset_sg) {
		migf->last_offset = 0;
		migf->last_offset_sg = table->sgt.sgl;
		migf->sg_last_entry = 0;
	}

	cur_offset = migf->last_offset;

	for_each_sg(migf->last_offset_sg, sg,
			table->sgt.orig_nents - migf->sg_last_entry, i) {
		if (offset < sg->length + cur_offset) {
			migf->last_offset_sg = sg;
			migf->sg_last_entry += i;
			migf->last_offset = cur_offset;
			return nth_page(sg_page(sg),
					(offset - cur_offset) / PAGE_SIZE);
		}
		cur_offset += sg->length;
	}
	return NULL;
}

static int mlx5vf_add_migration_pages(struct mlx5_vf_migration_file *migf,
				      unsigned int npages,
				      struct sg_append_table *table)
{
	unsigned int to_alloc = npages;
	struct page **page_list;
	unsigned long filled;
	unsigned int to_fill;
	int ret;

	to_fill = min_t(unsigned int, npages, PAGE_SIZE / sizeof(*page_list));
	page_list = kvzalloc(to_fill * sizeof(*page_list), GFP_KERNEL);
	if (!page_list)
		return -ENOMEM;

	do {
		filled = alloc_pages_bulk_array(GFP_KERNEL, to_fill, page_list);
		if (!filled) {
			ret = -ENOMEM;
			goto err;
		}
		to_alloc -= filled;
		ret = sg_alloc_append_table_from_pages(
			table, page_list, filled, 0,
			filled << PAGE_SHIFT, UINT_MAX, SG_MAX_SINGLE_ALLOC,
			GFP_KERNEL);

		if (ret)
			goto err;
		migf->allocated_length += filled * PAGE_SIZE;
		/* clean input for another bulk allocation */
		memset(page_list, 0, filled * sizeof(*page_list));
		to_fill = min_t(unsigned int, to_alloc,
				PAGE_SIZE / sizeof(*page_list));
	} while (to_alloc > 0);

	kvfree(page_list);
	return 0;

err:
	kvfree(page_list);
	return ret;
}

static void mlx5vf_prep_next_table(struct mlx5_vf_migration_file *migf)
{
	struct sg_page_iter sg_iter;

	lockdep_assert_held(&migf->lock);
	migf->table_start_pos += migf->image_length;
	/* clear sgtable, all data has been transferred */
	for_each_sgtable_page(&migf->table.sgt, &sg_iter, 0)
		__free_page(sg_page_iter_page(&sg_iter));
	sg_free_append_table(&migf->table);
	memset(&migf->table, 0, sizeof(migf->table));
	migf->image_length = 0;
	migf->allocated_length = 0;
	migf->last_offset_sg = NULL;
	migf->header_read = false;
}

static void mlx5vf_disable_fd(struct mlx5_vf_migration_file *migf)
{
	struct sg_page_iter sg_iter;

	mutex_lock(&migf->lock);
	/* Undo alloc_pages_bulk_array() */
	for_each_sgtable_page(&migf->table.sgt, &sg_iter, 0)
		__free_page(sg_page_iter_page(&sg_iter));
	sg_free_append_table(&migf->table);
	migf->disabled = true;
	migf->image_length = 0;
	migf->allocated_length = 0;
	migf->final_length = 0;
	migf->table_start_pos = 0;
	migf->filp->f_pos = 0;
	for_each_sgtable_page(&migf->final_table.sgt, &sg_iter, 0)
		__free_page(sg_page_iter_page(&sg_iter));
	sg_free_append_table(&migf->final_table);
	mutex_unlock(&migf->lock);
}

static int mlx5vf_release_file(struct inode *inode, struct file *filp)
{
	struct mlx5_vf_migration_file *migf = filp->private_data;

	mlx5vf_disable_fd(migf);
	mutex_destroy(&migf->lock);
	kfree(migf);
	return 0;
}

#define MIGF_TOTAL_DATA(migf) \
	(migf->table_start_pos + migf->image_length + migf->final_length + \
	 migf->sw_headers_bytes_sent)

#define VFIO_MIG_STATE_PRE_COPY(mvdev) \
	(mvdev->mig_state == VFIO_DEVICE_STATE_PRE_COPY || \
	 mvdev->mig_state == VFIO_DEVICE_STATE_PRE_COPY_P2P)

#define VFIO_PRE_COPY_SUPP(mvdev) \
	(mvdev->core_device.vdev.migration_flags & VFIO_MIGRATION_PRE_COPY)

#define MIGF_HAS_DATA(migf) \
	(READ_ONCE(migf->image_length) || READ_ONCE(migf->final_length))

static size_t
mlx5vf_final_table_start_pos(struct mlx5_vf_migration_file *migf)
{
	return MIGF_TOTAL_DATA(migf) - migf->final_length;
}

static size_t mlx5vf_get_table_start_pos(struct mlx5_vf_migration_file *migf)
{
	return migf->table_start_pos + migf->sw_headers_bytes_sent;
}

static size_t mlx5vf_get_table_end_pos(struct mlx5_vf_migration_file *migf,
				       struct sg_append_table *table)
{
	if (table == &migf->final_table)
		return MIGF_TOTAL_DATA(migf);
	return migf->table_start_pos + migf->image_length +
		migf->sw_headers_bytes_sent;
}

static void mlx5vf_send_sw_header(struct mlx5_vf_migration_file *migf,
				  loff_t *pos, char __user **buf, size_t *len,
				  ssize_t *done)
{
	struct mlx5_vf_migration_header header = {};
	size_t header_size = sizeof(header);
	void *header_buf = &header;
	size_t size_to_transfer;

	if (*pos >= mlx5vf_final_table_start_pos(migf))
		header.image_size = migf->final_length;
	else
		header.image_size = migf->image_length;

	size_to_transfer = header_size -
			   (migf->sw_headers_bytes_sent % header_size);
	size_to_transfer = min_t(size_t, size_to_transfer, *len);
	header_buf += header_size - size_to_transfer;
	if (copy_to_user(*buf, header_buf, size_to_transfer)) {
		*done = -EFAULT;
		return;
	}

	migf->sw_headers_bytes_sent += size_to_transfer;
	migf->header_read = !(migf->sw_headers_bytes_sent % header_size);

	*pos += size_to_transfer;
	*len -= size_to_transfer;
	*done += size_to_transfer;
	*buf += size_to_transfer;
}

static struct sg_append_table *
mlx5vf_get_table(struct mlx5_vf_migration_file *migf, loff_t *pos)
{
	if (migf->final_length &&
	    *pos >= mlx5vf_final_table_start_pos(migf))
		return &migf->final_table;
	return &migf->table;
}

static ssize_t mlx5vf_save_read(struct file *filp, char __user *buf, size_t len,
			       loff_t *pos)
{
	struct mlx5_vf_migration_file *migf = filp->private_data;
	struct sg_append_table *table;
	ssize_t done = 0;

	if (pos)
		return -ESPIPE;
	pos = &filp->f_pos;

	if (!(filp->f_flags & O_NONBLOCK)) {
		if (wait_event_interruptible(migf->poll_wait,
			     (MIGF_HAS_DATA(migf) || migf->is_err ||
			      migf->precopy_err)))
			return -ERESTARTSYS;
	}

	mutex_lock(&migf->lock);
	if ((filp->f_flags & O_NONBLOCK) && !MIGF_HAS_DATA(migf)) {
		done = -EAGAIN;
		goto out_unlock;
	}
	if (*pos > MIGF_TOTAL_DATA(migf)) {
		done = -EINVAL;
		goto out_unlock;
	}
	if (migf->disabled || migf->is_err) {
		done = -ENODEV;
		goto out_unlock;
	}

	/* If we reach the end of the PRE_COPY size */
	if (MIGF_TOTAL_DATA(migf) == *pos &&
	    VFIO_MIG_STATE_PRE_COPY(migf->mvdev)) {
		done = -ENOMSG;
		goto out_unlock;
	}

	if (VFIO_PRE_COPY_SUPP(migf->mvdev) && !migf->header_read) {
		mlx5vf_send_sw_header(migf, pos, &buf, &len, &done);
		if (done < 0)
			goto out_unlock;
	}

	len = min_t(size_t, MIGF_TOTAL_DATA(migf) - *pos, len);
	table = mlx5vf_get_table(migf, pos);
	while (len) {
		struct sg_append_table *tmp = table;
		unsigned long offset;
		size_t page_offset;
		struct page *page;
		size_t page_len;
		u8 *from_buff;
		int ret;

		offset = *pos - mlx5vf_get_table_start_pos(migf);
		page_offset = offset % PAGE_SIZE;
		offset -= page_offset;
		page = mlx5vf_get_migration_page(migf, offset, table);
		if (!page) {
			if (done == 0)
				done = -EINVAL;
			goto out_unlock;
		}

		page_len = min_t(size_t, len, PAGE_SIZE - page_offset);
		/*
		 * In case an image is ended in the middle of the page, read
		 * until the end of the image and manage it.
		 */
		page_len = min_t(size_t, page_len,
				 mlx5vf_get_table_end_pos(migf, table) - *pos);
		from_buff = kmap_local_page(page);
		ret = copy_to_user(buf, from_buff + page_offset, page_len);
		kunmap_local(from_buff);
		if (ret) {
			done = -EFAULT;
			goto out_unlock;
		}
		*pos += page_len;
		len -= page_len;
		done += page_len;
		buf += page_len;
		/*
		 * In case we moved from PRE_COPY to STOP_COPY we need to prepare
		 * migf for final state when current state was fully transferred.
		 * Otherwise we might miss the final table and caller may get EOF
		 * by next read().
		 */
		if (migf->final_table.sgt.sgl &&
		    *pos == mlx5vf_final_table_start_pos(migf)) {
			mlx5vf_prep_next_table(migf);
			table = mlx5vf_get_table(migf, pos);
			/*
			 * Check whether the SAVE command has finished and we
			 * have some extra data.
			 */
			if (tmp == table)
				break;
			mlx5vf_send_sw_header(migf, pos, &buf, &len, &done);
			if (done < 0)
				goto out_unlock;
		}
	}

out_unlock:
	mutex_unlock(&migf->lock);
	return done;
}

static __poll_t mlx5vf_save_poll(struct file *filp,
				 struct poll_table_struct *wait)
{
	struct mlx5_vf_migration_file *migf = filp->private_data;
	__poll_t pollflags = 0;

	poll_wait(filp, &migf->poll_wait, wait);

	mutex_lock(&migf->lock);
	if (migf->disabled || migf->is_err)
		pollflags = EPOLLIN | EPOLLRDNORM | EPOLLRDHUP;
	else if (MIGF_HAS_DATA(migf) || migf->precopy_err)
		pollflags = EPOLLIN | EPOLLRDNORM;
	mutex_unlock(&migf->lock);

	return pollflags;
}

/*
 * FD is exposed and user can use it after receiving an error.
 * Mark migf in error, and wake the user.
 */
static void mlx5vf_mark_err(struct mlx5_vf_migration_file *migf)
{
	migf->is_err = true;
	wake_up_interruptible(&migf->poll_wait);
}

static ssize_t mlx5vf_precopy_ioctl(struct file *filp, unsigned int cmd,
				    unsigned long arg)
{
	struct mlx5_vf_migration_file *migf = filp->private_data;
	struct mlx5vf_pci_core_device *mvdev = migf->mvdev;
	bool first_state, state_finish_transfer;
	struct vfio_precopy_info info;
	loff_t *pos = &filp->f_pos;
	unsigned long minsz;
	size_t inc_length;
	int ret;

	if (cmd != VFIO_MIG_GET_PRECOPY_INFO)
		return -ENOTTY;

	minsz = offsetofend(struct vfio_precopy_info, dirty_bytes);

	if (copy_from_user(&info, (void __user *)arg, minsz))
		return -EFAULT;

	if (info.argsz < minsz)
		return -EINVAL;

	mutex_lock(&mvdev->state_mutex);
	if (!VFIO_MIG_STATE_PRE_COPY(migf->mvdev)) {
		ret = -EINVAL;
		goto err_state_unlock;
	}

	/*
	 * We can't issue a SAVE command when the device is suspended, so as
	 * part of VFIO_DEVICE_STATE_PRE_COPY_P2P no reason to query for extra
	 * bytes that can't be read.
	 */
	if (mvdev->mig_state != VFIO_DEVICE_STATE_PRE_COPY_P2P) {
		/*
		 * Once the query returns it's guaranteed that there is no
		 * active SAVE command.
		 * As so, the other code below is safe with the proper locks.
		 */
		ret = mlx5vf_cmd_query_vhca_migration_state(mvdev, &inc_length,
							    MLX5VF_QUERY_INC);
		if (ret)
			goto err_state_unlock;
	}

	mutex_lock(&migf->lock);
	if (*pos > MIGF_TOTAL_DATA(migf)) {
		ret = -EINVAL;
		goto err_migf_unlock;
	}

	if (migf->disabled || migf->is_err) {
		ret = -ENODEV;
		goto err_migf_unlock;
	}

	first_state = migf->table_start_pos == 0;
	if (first_state) {
		info.initial_bytes = MIGF_TOTAL_DATA(migf) - *pos;
		info.dirty_bytes = 0;
	} else {
		info.initial_bytes = 0;
		info.dirty_bytes = MIGF_TOTAL_DATA(migf) - *pos;
	}
	state_finish_transfer = *pos == MIGF_TOTAL_DATA(migf);
	if (!(state_finish_transfer && inc_length &&
	      mvdev->mig_state == VFIO_DEVICE_STATE_PRE_COPY)) {
		mutex_unlock(&migf->lock);
		goto done;
	}

	/*
	 * We finished transferring the current state and the device has a
	 * dirty state, save a new state to be ready for.
	 */
	mlx5vf_prep_next_table(migf);
	ret = mlx5vf_add_migration_pages(migf,
					 DIV_ROUND_UP_ULL(inc_length, PAGE_SIZE),
					 &migf->table);
	mutex_unlock(&migf->lock);
	if (ret) {
		mlx5vf_mark_err(migf);
		goto err_state_unlock;
	}

	ret = mlx5vf_cmd_save_vhca_state(mvdev, migf, true, true);
	if (ret) {
		mlx5vf_mark_err(migf);
		goto err_state_unlock;
	}

	info.dirty_bytes += inc_length;

done:
	mlx5vf_state_mutex_unlock(mvdev);
	return copy_to_user((void __user *)arg, &info, minsz);

err_migf_unlock:
	mutex_unlock(&migf->lock);
err_state_unlock:
	mlx5vf_state_mutex_unlock(mvdev);
	return ret;
}

static const struct file_operations mlx5vf_save_fops = {
	.owner = THIS_MODULE,
	.read = mlx5vf_save_read,
	.poll = mlx5vf_save_poll,
	.unlocked_ioctl = mlx5vf_precopy_ioctl,
	.compat_ioctl = compat_ptr_ioctl,
	.release = mlx5vf_release_file,
	.llseek = no_llseek,
};

static int mlx5vf_pci_save_device_inc_data(struct mlx5vf_pci_core_device *mvdev)
{
	struct mlx5_vf_migration_file *migf = mvdev->saving_migf;
	size_t length;
	int ret;

	ret = mlx5vf_cmd_query_vhca_migration_state(mvdev, &length,
					MLX5VF_QUERY_INC | MLX5VF_QUERY_FINAL);
	if (ret)
		return ret;

	mutex_lock(&migf->lock);
	if (migf->is_err) {
		ret = -ENODEV;
		goto err;
	}
	/*
	 * We finished transferring the current state, prepare migf for final
	 * table. Otherwise we might miss the final table and caller may get
	 * EOF by next read().
	 */
	if (migf->filp->f_pos == MIGF_TOTAL_DATA(migf))
		mlx5vf_prep_next_table(migf);
	ret = mlx5vf_add_migration_pages(
		migf, DIV_ROUND_UP_ULL(length, PAGE_SIZE), &migf->final_table);
	if (ret) {
		mlx5vf_mark_err(migf);
		goto err;
	}

	mutex_unlock(&migf->lock);
	ret = mlx5vf_cmd_save_vhca_state(mvdev, migf, true, false);
	if (ret)
		mlx5vf_mark_err(migf);
	return ret;

err:
	mutex_unlock(&migf->lock);
	return ret;
}

static struct mlx5_vf_migration_file *
mlx5vf_pci_save_device_data(struct mlx5vf_pci_core_device *mvdev, bool track)
{
	struct mlx5_vf_migration_file *migf;
	size_t length;
	int ret;

	migf = kzalloc(sizeof(*migf), GFP_KERNEL);
	if (!migf)
		return ERR_PTR(-ENOMEM);

	migf->filp = anon_inode_getfile("mlx5vf_mig", &mlx5vf_save_fops, migf,
					O_RDONLY);
	if (IS_ERR(migf->filp)) {
		int err = PTR_ERR(migf->filp);

		kfree(migf);
		return ERR_PTR(err);
	}

	stream_open(migf->filp->f_inode, migf->filp);
	mutex_init(&migf->lock);
	init_waitqueue_head(&migf->poll_wait);
	init_waitqueue_head(&migf->save_wait);
	mlx5_cmd_init_async_ctx(mvdev->mdev, &migf->async_ctx);
	INIT_WORK(&migf->async_data.work, mlx5vf_mig_file_cleanup_cb);
	ret = mlx5vf_cmd_query_vhca_migration_state(mvdev, &length, 0);
	if (ret)
		goto out_free;

	ret = mlx5vf_add_migration_pages(
		migf, DIV_ROUND_UP_ULL(length, PAGE_SIZE), &migf->table);
	if (ret)
		goto out_free;

	migf->mvdev = mvdev;
	ret = mlx5vf_cmd_save_vhca_state(mvdev, migf, false, track);
	if (ret)
		goto out_free;
	return migf;
out_free:
	fput(migf->filp);
	return ERR_PTR(ret);
}

static void mlx5vf_recv_sw_header(struct mlx5_vf_migration_file *migf,
				  loff_t *pos, const char __user **buf,
				  size_t *len, ssize_t *done)
{
	ssize_t header_size = sizeof(migf->header);
	void *header_buf = &migf->header;
	size_t size_to_recv;

	size_to_recv = header_size - (migf->sw_headers_bytes_sent % header_size);
	size_to_recv = min_t(size_t, size_to_recv, *len);
	header_buf += header_size - size_to_recv;
	if (copy_from_user(header_buf, *buf, size_to_recv)) {
		*done = -EFAULT;
		return;
	}

	*pos += size_to_recv;
	*len -= size_to_recv;
	*done += size_to_recv;
	*buf += size_to_recv;
	migf->sw_headers_bytes_sent += size_to_recv;
	migf->header_read = !(migf->sw_headers_bytes_sent % header_size);

	if (migf->sw_headers_bytes_sent % header_size)
		return;
	migf->expected_length = migf->header.image_size;
}

#define EXPECTED_TABLE_END_POSITION(migf) \
	(migf->table_start_pos + migf->expected_length + \
	 migf->sw_headers_bytes_sent)

static ssize_t mlx5vf_resume_write(struct file *filp, const char __user *buf,
				   size_t len, loff_t *pos)
{
	struct mlx5_vf_migration_file *migf = filp->private_data;
	loff_t requested_length;
	ssize_t done = 0;
	int ret = 0;

	if (pos)
		return -ESPIPE;
	pos = &filp->f_pos;

	if (*pos < 0 ||
	    check_add_overflow((loff_t)len, *pos, &requested_length))
		return -EINVAL;

	mutex_lock(&migf->mvdev->state_mutex);
	mutex_lock(&migf->lock);
	requested_length -= migf->table_start_pos;
	if (requested_length > MAX_MIGRATION_SIZE) {
		ret = -ENOMEM;
		goto out_unlock;
	}

	if (migf->disabled) {
		ret = -ENODEV;
		goto out_unlock;
	}

start_over:
	if (migf->allocated_length < requested_length) {
		ret = mlx5vf_add_migration_pages(
			migf,
			DIV_ROUND_UP(requested_length - migf->allocated_length,
				     PAGE_SIZE), &migf->table);
		if (ret)
			goto out_unlock;
	}

	if (VFIO_PRE_COPY_SUPP(migf->mvdev)) {
		if (!migf->header_read)
			mlx5vf_recv_sw_header(migf, pos, &buf, &len, &done);
		if (done < 0)
			goto out_unlock;
	}

	while (len) {
		unsigned long offset;
		size_t page_offset;
		struct page *page;
		size_t page_len;
		u8 *to_buff;

		offset = *pos - mlx5vf_get_table_start_pos(migf);
		page_offset = offset % PAGE_SIZE;
		offset -= page_offset;
		page = mlx5vf_get_migration_page(migf, offset,
						 &migf->table);
		if (!page) {
			if (done == 0)
				done = -EINVAL;
			goto out_unlock;
		}

		page_len = min_t(size_t, len, PAGE_SIZE - page_offset);
		if (VFIO_PRE_COPY_SUPP(migf->mvdev))
			page_len = min_t(size_t, page_len,
				 EXPECTED_TABLE_END_POSITION(migf) - *pos);

		to_buff = kmap_local_page(page);
		ret = copy_from_user(to_buff + page_offset, buf, page_len);
		kunmap_local(to_buff);
		if (ret) {
			ret = -EFAULT;
			goto out_unlock;
		}
		*pos += page_len;
		len -= page_len;
		done += page_len;
		buf += page_len;
		migf->image_length += page_len;

		if (*pos == EXPECTED_TABLE_END_POSITION(migf)) {
			ret = mlx5vf_cmd_load_vhca_state(migf->mvdev, migf);
			if (ret)
				goto out_unlock;
			mlx5vf_prep_next_table(migf);
			if (len) {
				requested_length -= migf->expected_length;
				goto start_over;
			}
		}
	}
out_unlock:
	mutex_unlock(&migf->lock);
	mlx5vf_state_mutex_unlock(migf->mvdev);
	return ret ? ret : done;
}

static const struct file_operations mlx5vf_resume_fops = {
	.owner = THIS_MODULE,
	.write = mlx5vf_resume_write,
	.release = mlx5vf_release_file,
	.llseek = no_llseek,
};

static struct mlx5_vf_migration_file *
mlx5vf_pci_resume_device_data(struct mlx5vf_pci_core_device *mvdev)
{
	struct mlx5_vf_migration_file *migf;

	migf = kzalloc(sizeof(*migf), GFP_KERNEL);
	if (!migf)
		return ERR_PTR(-ENOMEM);

	migf->filp = anon_inode_getfile("mlx5vf_mig", &mlx5vf_resume_fops, migf,
					O_WRONLY);
	if (IS_ERR(migf->filp)) {
		int err = PTR_ERR(migf->filp);

		kfree(migf);
		return ERR_PTR(err);
	}
	stream_open(migf->filp->f_inode, migf->filp);
	mutex_init(&migf->lock);
	migf->mvdev = mvdev;
	return migf;
}

void mlx5vf_disable_fds(struct mlx5vf_pci_core_device *mvdev)
{
	if (mvdev->resuming_migf) {
		mlx5vf_disable_fd(mvdev->resuming_migf);
		fput(mvdev->resuming_migf->filp);
		mvdev->resuming_migf = NULL;
	}
	if (mvdev->saving_migf) {
		mlx5_cmd_cleanup_async_ctx(&mvdev->saving_migf->async_ctx);
		cancel_work_sync(&mvdev->saving_migf->async_data.work);
		mlx5vf_disable_fd(mvdev->saving_migf);
		fput(mvdev->saving_migf->filp);
		mvdev->saving_migf = NULL;
	}
}

static struct file *
mlx5vf_pci_step_device_state_locked(struct mlx5vf_pci_core_device *mvdev,
				    u32 new)
{
	u32 cur = mvdev->mig_state;
	int ret;

	if (cur == VFIO_DEVICE_STATE_RUNNING_P2P && new == VFIO_DEVICE_STATE_STOP) {
		ret = mlx5vf_cmd_suspend_vhca(mvdev,
			MLX5_SUSPEND_VHCA_IN_OP_MOD_SUSPEND_RESPONDER);
		if (ret)
			return ERR_PTR(ret);
		return NULL;
	}

	if (cur == VFIO_DEVICE_STATE_STOP && new == VFIO_DEVICE_STATE_RUNNING_P2P) {
		ret = mlx5vf_cmd_resume_vhca(mvdev,
			MLX5_RESUME_VHCA_IN_OP_MOD_RESUME_RESPONDER);
		if (ret)
			return ERR_PTR(ret);
		return NULL;
	}

	if ((cur == VFIO_DEVICE_STATE_RUNNING && new == VFIO_DEVICE_STATE_RUNNING_P2P) ||
	    (cur == VFIO_DEVICE_STATE_PRE_COPY && new == VFIO_DEVICE_STATE_PRE_COPY_P2P)) {
		ret = mlx5vf_cmd_suspend_vhca(mvdev,
			MLX5_SUSPEND_VHCA_IN_OP_MOD_SUSPEND_INITIATOR);
		if (ret)
			return ERR_PTR(ret);
		return NULL;
	}

	if ((cur == VFIO_DEVICE_STATE_RUNNING_P2P && new == VFIO_DEVICE_STATE_RUNNING) ||
	    (cur == VFIO_DEVICE_STATE_PRE_COPY_P2P && new == VFIO_DEVICE_STATE_PRE_COPY)) {
		ret = mlx5vf_cmd_resume_vhca(mvdev,
			MLX5_RESUME_VHCA_IN_OP_MOD_RESUME_INITIATOR);
		if (ret)
			return ERR_PTR(ret);
		return NULL;
	}

	if (cur == VFIO_DEVICE_STATE_STOP && new == VFIO_DEVICE_STATE_STOP_COPY) {
		struct mlx5_vf_migration_file *migf;

		migf = mlx5vf_pci_save_device_data(mvdev, false);
		if (IS_ERR(migf))
			return ERR_CAST(migf);
		get_file(migf->filp);
		mvdev->saving_migf = migf;
		return migf->filp;
	}

	if ((cur == VFIO_DEVICE_STATE_STOP_COPY && new == VFIO_DEVICE_STATE_STOP) ||
	    (cur == VFIO_DEVICE_STATE_PRE_COPY && new == VFIO_DEVICE_STATE_RUNNING) ||
	    (cur == VFIO_DEVICE_STATE_PRE_COPY_P2P &&
	     new == VFIO_DEVICE_STATE_RUNNING_P2P)) {
		mlx5vf_disable_fds(mvdev);
		return NULL;
	}

	if (cur == VFIO_DEVICE_STATE_STOP && new == VFIO_DEVICE_STATE_RESUMING) {
		struct mlx5_vf_migration_file *migf;

		migf = mlx5vf_pci_resume_device_data(mvdev);
		if (IS_ERR(migf))
			return ERR_CAST(migf);
		get_file(migf->filp);
		mvdev->resuming_migf = migf;
		return migf->filp;
	}

	if (cur == VFIO_DEVICE_STATE_RESUMING && new == VFIO_DEVICE_STATE_STOP) {
		if (!VFIO_PRE_COPY_SUPP(mvdev)) {
			mutex_lock(&mvdev->resuming_migf->lock);
			ret = mlx5vf_cmd_load_vhca_state(mvdev,
							 mvdev->resuming_migf);
			mutex_unlock(&mvdev->resuming_migf->lock);
			if (ret)
				return ERR_PTR(ret);
		}
		mlx5vf_disable_fds(mvdev);
		return NULL;
	}

	if ((cur == VFIO_DEVICE_STATE_RUNNING && new == VFIO_DEVICE_STATE_PRE_COPY) ||
	    (cur == VFIO_DEVICE_STATE_RUNNING_P2P &&
	     new == VFIO_DEVICE_STATE_PRE_COPY_P2P)) {
		struct mlx5_vf_migration_file *migf;

		migf = mlx5vf_pci_save_device_data(mvdev, true);
		if (IS_ERR(migf))
			return ERR_CAST(migf);
		get_file(migf->filp);
		mvdev->saving_migf = migf;
		return migf->filp;
	}

	if (cur == VFIO_DEVICE_STATE_PRE_COPY_P2P && new == VFIO_DEVICE_STATE_STOP_COPY) {
		ret = mlx5vf_cmd_suspend_vhca(mvdev,
			MLX5_SUSPEND_VHCA_IN_OP_MOD_SUSPEND_RESPONDER);
		if (ret)
			return ERR_PTR(ret);
		ret = mlx5vf_pci_save_device_inc_data(mvdev);
		return ret ? ERR_PTR(ret) : NULL;
	}

	/*
	 * vfio_mig_get_next_state() does not use arcs other than the above
	 */
	WARN_ON(true);
	return ERR_PTR(-EINVAL);
}

/*
 * This function is called in all state_mutex unlock cases to
 * handle a 'deferred_reset' if exists.
 */
void mlx5vf_state_mutex_unlock(struct mlx5vf_pci_core_device *mvdev)
{
again:
	spin_lock(&mvdev->reset_lock);
	if (mvdev->deferred_reset) {
		mvdev->deferred_reset = false;
		spin_unlock(&mvdev->reset_lock);
		mvdev->mig_state = VFIO_DEVICE_STATE_RUNNING;
		mlx5vf_disable_fds(mvdev);
		goto again;
	}
	mutex_unlock(&mvdev->state_mutex);
	spin_unlock(&mvdev->reset_lock);
}

static struct file *
mlx5vf_pci_set_device_state(struct vfio_device *vdev,
			    enum vfio_device_mig_state new_state)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(
		vdev, struct mlx5vf_pci_core_device, core_device.vdev);
	enum vfio_device_mig_state next_state;
	struct file *res = NULL;
	int ret;

	mutex_lock(&mvdev->state_mutex);
	while (new_state != mvdev->mig_state) {
		ret = vfio_mig_get_next_state(vdev, mvdev->mig_state,
					      new_state, &next_state);
		if (ret) {
			res = ERR_PTR(ret);
			break;
		}
		res = mlx5vf_pci_step_device_state_locked(mvdev, next_state);
		if (IS_ERR(res))
			break;
		mvdev->mig_state = next_state;
		if (WARN_ON(res && new_state != mvdev->mig_state)) {
			fput(res);
			res = ERR_PTR(-EINVAL);
			break;
		}
	}
	mlx5vf_state_mutex_unlock(mvdev);
	return res;
}

static int mlx5vf_pci_get_data_size(struct vfio_device *vdev,
				    unsigned long *stop_copy_length)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(
		vdev, struct mlx5vf_pci_core_device, core_device.vdev);
	size_t state_size;
	int ret;

	mutex_lock(&mvdev->state_mutex);
	ret = mlx5vf_cmd_query_vhca_migration_state(mvdev,
						    &state_size, 0);
	if (!ret)
		*stop_copy_length = state_size;
	mlx5vf_state_mutex_unlock(mvdev);
	return ret;
}

static int mlx5vf_pci_get_device_state(struct vfio_device *vdev,
				       enum vfio_device_mig_state *curr_state)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(
		vdev, struct mlx5vf_pci_core_device, core_device.vdev);

	mutex_lock(&mvdev->state_mutex);
	*curr_state = mvdev->mig_state;
	mlx5vf_state_mutex_unlock(mvdev);
	return 0;
}

static void mlx5vf_pci_aer_reset_done(struct pci_dev *pdev)
{
	struct mlx5vf_pci_core_device *mvdev = mlx5vf_drvdata(pdev);

	if (!mvdev->migrate_cap)
		return;

	/*
	 * As the higher VFIO layers are holding locks across reset and using
	 * those same locks with the mm_lock we need to prevent ABBA deadlock
	 * with the state_mutex and mm_lock.
	 * In case the state_mutex was taken already we defer the cleanup work
	 * to the unlock flow of the other running context.
	 */
	spin_lock(&mvdev->reset_lock);
	mvdev->deferred_reset = true;
	if (!mutex_trylock(&mvdev->state_mutex)) {
		spin_unlock(&mvdev->reset_lock);
		return;
	}
	spin_unlock(&mvdev->reset_lock);
	mlx5vf_state_mutex_unlock(mvdev);
}

static int mlx5vf_pci_open_device(struct vfio_device *core_vdev)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(
		core_vdev, struct mlx5vf_pci_core_device, core_device.vdev);
	struct vfio_pci_core_device *vdev = &mvdev->core_device;
	int ret;

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	if (mvdev->migrate_cap)
		mvdev->mig_state = VFIO_DEVICE_STATE_RUNNING;
	vfio_pci_core_finish_enable(vdev);
	return 0;
}

static void mlx5vf_pci_close_device(struct vfio_device *core_vdev)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(
		core_vdev, struct mlx5vf_pci_core_device, core_device.vdev);

	mlx5vf_cmd_close_migratable(mvdev);
	vfio_pci_core_close_device(core_vdev);
}

static const struct vfio_migration_ops mlx5vf_pci_mig_ops = {
	.migration_set_state = mlx5vf_pci_set_device_state,
	.migration_get_state = mlx5vf_pci_get_device_state,
	.migration_get_data_size = mlx5vf_pci_get_data_size,
};

static const struct vfio_log_ops mlx5vf_pci_log_ops = {
	.log_start = mlx5vf_start_page_tracker,
	.log_stop = mlx5vf_stop_page_tracker,
	.log_read_and_clear = mlx5vf_tracker_read_and_clear,
};

static int mlx5vf_pci_init_dev(struct vfio_device *core_vdev)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(core_vdev,
			struct mlx5vf_pci_core_device, core_device.vdev);
	int ret;

	ret = vfio_pci_core_init_dev(core_vdev);
	if (ret)
		return ret;

	mlx5vf_cmd_set_migratable(mvdev, &mlx5vf_pci_mig_ops,
				  &mlx5vf_pci_log_ops);

	return 0;
}

static void mlx5vf_pci_release_dev(struct vfio_device *core_vdev)
{
	struct mlx5vf_pci_core_device *mvdev = container_of(core_vdev,
			struct mlx5vf_pci_core_device, core_device.vdev);

	mlx5vf_cmd_remove_migratable(mvdev);
	vfio_pci_core_release_dev(core_vdev);
}

static const struct vfio_device_ops mlx5vf_pci_ops = {
	.name = "mlx5-vfio-pci",
	.init = mlx5vf_pci_init_dev,
	.release = mlx5vf_pci_release_dev,
	.open_device = mlx5vf_pci_open_device,
	.close_device = mlx5vf_pci_close_device,
	.ioctl = vfio_pci_core_ioctl,
	.device_feature = vfio_pci_core_ioctl_feature,
	.read = vfio_pci_core_read,
	.write = vfio_pci_core_write,
	.mmap = vfio_pci_core_mmap,
	.request = vfio_pci_core_request,
	.match = vfio_pci_core_match,
};

static int mlx5vf_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct mlx5vf_pci_core_device *mvdev;
	int ret;

	mvdev = vfio_alloc_device(mlx5vf_pci_core_device, core_device.vdev,
				  &pdev->dev, &mlx5vf_pci_ops);
	if (IS_ERR(mvdev))
		return PTR_ERR(mvdev);

	dev_set_drvdata(&pdev->dev, &mvdev->core_device);
	ret = vfio_pci_core_register_device(&mvdev->core_device);
	if (ret)
		goto out_put_vdev;
	return 0;

out_put_vdev:
	vfio_put_device(&mvdev->core_device.vdev);
	return ret;
}

static void mlx5vf_pci_remove(struct pci_dev *pdev)
{
	struct mlx5vf_pci_core_device *mvdev = mlx5vf_drvdata(pdev);

	vfio_pci_core_unregister_device(&mvdev->core_device);
	vfio_put_device(&mvdev->core_device.vdev);
}

static const struct pci_device_id mlx5vf_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101e) }, /* ConnectX Family mlx5Gen Virtual Function */
	{}
};

MODULE_DEVICE_TABLE(pci, mlx5vf_pci_table);

static const struct pci_error_handlers mlx5vf_err_handlers = {
	.reset_done = mlx5vf_pci_aer_reset_done,
	.error_detected = vfio_pci_core_aer_err_detected,
};

static struct pci_driver mlx5vf_pci_driver = {
	.name = KBUILD_MODNAME,
	.id_table = mlx5vf_pci_table,
	.probe = mlx5vf_pci_probe,
	.remove = mlx5vf_pci_remove,
	.err_handler = &mlx5vf_err_handlers,
	.driver_managed_dma = true,
};

static void __exit mlx5vf_pci_cleanup(void)
{
	pci_unregister_driver(&mlx5vf_pci_driver);
}

static int __init mlx5vf_pci_init(void)
{
	return pci_register_driver(&mlx5vf_pci_driver);
}

module_init(mlx5vf_pci_init);
module_exit(mlx5vf_pci_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Max Gurtovoy <mgurtovoy@nvidia.com>");
MODULE_AUTHOR("Yishai Hadas <yishaih@nvidia.com>");
MODULE_DESCRIPTION(
	"MLX5 VFIO PCI - User Level meta-driver for MLX5 device family");
