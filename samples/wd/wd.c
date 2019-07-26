/* SPDX-License-Identifier: Apache-2.0 */
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/poll.h>

#include "wd.h"
#include "wd_adapter.h"

#define SYS_CLASS_DIR	"/sys/class"
#define LINUX_DEV_DIR	"/dev"
#define UACCE_CLASS_DIR SYS_CLASS_DIR"/"UACCE_CLASS_NAME
#define _TRY_REQUEST_TIMES		64
#define PATH_ENDCHAR_SIZE		1
#define LINUX_CRTDIR_SIZE		1
#define LINUX_PRTDIR_SIZE		2

#ifdef WITH_LOG_FILE
FILE *flog_fd = NULL;
#endif

struct _dev_info {
	int node_id;
	int numa_dis;
	int iommu_type;
	int flags;
	int ref;
	int is_load;
	int available_instances;
	unsigned int weight;
	char alg_path[PATH_STR_SIZE];
	char dev_root[PATH_STR_SIZE];
	char name[WD_NAME_SIZE];
	char api[WD_NAME_SIZE];
	char algs[MAX_ATTR_STR_SIZE];
	unsigned long qfrs_offset[UACCE_QFRT_MAX];
};

static int _get_raw_attr(const char *dev_root, const char *attr, char *buf, size_t sz)
{
	char attr_file[PATH_STR_SIZE];
	int fd;
	int size;

	size = snprintf(attr_file, PATH_STR_SIZE, "%s/"UACCE_DEV_ATTRS"/%s",
			dev_root, attr);
	if (size <= 0) {
		WD_ERR("get %s/%s path fail!\n", dev_root, attr);
		return size;
	}

	/* The attr_file = "/sys/class/uacce/attrs/xxx",It's the Internal Definition File Node */
	fd = open(attr_file, O_RDONLY, 0);
	if (fd < 0) {
		WD_ERR("open %s fail!\n", attr_file);
		return fd;
	}
	size = read(fd, buf, sz);
	if (size <= 0) {
		WD_ERR("read nothing at %s!\n", attr_file);
		size = -ENODEV;
	}

	close(fd);
	return size;
}

static int _get_int_attr(struct _dev_info *dinfo, const char *attr)
{
	int size;
	char buf[MAX_ATTR_STR_SIZE];

	size = _get_raw_attr(dinfo->dev_root, attr, buf, MAX_ATTR_STR_SIZE);
	if (size <= 0 || size >= MAX_ATTR_STR_SIZE)
		return -EINVAL;
	else {
		/* Handing the read string's end tails '\n' to '\0' */
		if (buf[size] == '\n')
			buf[size] = '\0';
		return atoi((char *)&buf);
	}
}

/*
 * Get string from an attr of sysfs. '\n' is used as a token of substring.
 * So '\n' could be in the middle of the string or at the last of the string.
 * Now remove the token '\n' at the end of the string to avoid confusion.
 */
static int _get_str_attr(struct _dev_info *dinfo, const char *attr, char *buf,
			 size_t buf_sz)
{
	int size;

	size = _get_raw_attr(dinfo->dev_root, attr, buf, buf_sz);
	if (size <= 0) {
		buf[0] = '\0';
		return size;
	}

	if (size == buf_sz)
		size = size - 1;

	buf[size] = '\0';
	while ((size > 1) && (buf[size - 1] == '\n')) {
		buf[size - 1] = '\0';
		size = size - 1;
	}
	return size;
}

static int _get_ul_vec_attr(struct _dev_info *dinfo, const char *attr,
			       unsigned long *vec, int vec_sz)
{
	char buf[MAX_ATTR_STR_SIZE];
	int size, i, j;
	char *begin, *end;

	size = _get_raw_attr(dinfo->dev_root, attr, buf, MAX_ATTR_STR_SIZE);
	if (size <= 0) {
		for (i = 0; i < vec_sz; i++)
			vec[i] = 0;
		return size;
	}

	begin = buf;
	for (i = 0; i < vec_sz; i++) {
		vec[i] = strtoul(begin, &end, 0);
		if (!end)
			break;
		begin = end;
	}

	for (j = i; j < vec_sz; j++)
		vec[j] = 0;

	return 0;
}

static int _get_dev_info(struct _dev_info *dinfo, const char *alg)
{
	int ret;

	ret = _get_int_attr(dinfo, "isolate");
	if (ret < 0 || ret == 1)
		return -ENODEV;

	ret = _get_int_attr(dinfo, "dev_state");
	if (ret < 0)
		return ret;

	ret = _get_str_attr(dinfo, "algorithms",
			    dinfo->algs, MAX_ATTR_STR_SIZE);
	if (ret < 0)
		return ret;

	/* Add ALG check to cut later pointless logic */
	if (alg && !strstr(dinfo->algs, alg))
		return -ENODEV;
	ret = _get_int_attr(dinfo, "available_instances");
	if (ret <= 0)
		return -ENODEV;
	dinfo->available_instances = ret;

	ret = _get_int_attr(dinfo, "numa_distance");
	if (ret < 0)
		return ret;
	dinfo->numa_dis = ret;

	dinfo->node_id = _get_int_attr(dinfo, "node_id");

	ret = _get_int_attr(dinfo, "flags");
	if (ret < 0)
		return ret;
	dinfo->flags = ret;

	ret = _get_str_attr(dinfo, "api", dinfo->api, WD_NAME_SIZE);
	if (ret < 0)
		return ret;

	ret = _get_ul_vec_attr(dinfo, "qfrs_offset", dinfo->qfrs_offset,
			       UACCE_QFRT_MAX);
	if (ret < 0)
		return ret;

	/*
	 * Use available_instances as the base of weight.
	 * Remote NUMA node cuts the weight.
	 */
	if (dinfo->available_instances > 0)
		dinfo->weight = dinfo->available_instances;
	else
		dinfo->weight = 0;

	/* Check whether it's the remote distance. */
	if (dinfo->numa_dis)
		dinfo->weight = dinfo->weight >> 2;

	return 0;
}

static void _copy_if_better(struct _dev_info *old, struct _dev_info *new,
			    struct wd_capa *capa)
{
	dbg("try accelerator %s (inst_num=%d)...", new->name,
	    new->available_instances);

	/* Does the new dev match the need? */
	if (new->available_instances <= 0 || !strstr(new->algs, capa->alg))
		goto out;

	/* Is the new dev better? */
	if (!old->name[0] || (new->weight > old->weight)) {
		memcpy(old, new, sizeof(*old));
		dbg("adopted\n");
		return;
	}

out:
	dbg("ignored\n");
}

static void _pre_init_dev(struct _dev_info *dinfo, const char *name)
{
	int ret;

	strncpy(dinfo->name, name, WD_NAME_SIZE - PATH_ENDCHAR_SIZE);

	/*
	* The 16 chars UACCE_CLASS_DIR = "/sys/class/uacce"
	* The strncpy met the empty char  will auto stop and add '\0' on the tail.
	*/
	ret = snprintf(dinfo->dev_root, PATH_STR_SIZE, "%s/%s", UACCE_CLASS_DIR, name);
	if (ret < 0) {
		WD_ERR("get uacce file path fail!\n");
		return ;
	}
}

static int get_denoted_dev(struct wd_capa *capa, const char *dev,
				struct _dev_info *dinfop)
{
	_pre_init_dev(dinfop, dev);
	if (!_get_dev_info(dinfop, capa->alg))
		return 0;
	WD_ERR("fail to get dev %s!\n", dev);
	return -ENODEV;
}

static struct _dev_info *_find_available_res(struct wd_capa *capa, const char *dev, int *num)
{
	struct _dev_info dinfo, *dinfop = NULL;
	struct dirent *device;
	DIR *wd_class = NULL;
	char *name;
	int cnt = 0;

	dinfop = calloc(1, sizeof(*dinfop));
	if (!dinfop) {
		WD_ERR("calloc for dev info fail!\n");
		return NULL;
	}

	/* As user denotes a device */
	if (dev && dev[0] && dev[0] != '/') {
		if (!get_denoted_dev(capa, dev, dinfop))
			return dinfop;
	}

	wd_class = opendir(UACCE_CLASS_DIR);
	if (!wd_class) {
		WD_ERR("WD framework is not enabled on the system!\n");
		goto err_with_dinfop;
	}

	while (true) {
		device = readdir(wd_class);
		if (!device)
			break;
		name = device->d_name;
		if (!strncmp(name, ".", LINUX_CRTDIR_SIZE) ||
			!strncmp(name, "..", LINUX_PRTDIR_SIZE))
			continue;
		_pre_init_dev(&dinfo, name);
		if (!_get_dev_info(&dinfo, capa->alg)) {
			_copy_if_better(dinfop, &dinfo, capa);
			cnt++;
		}
	}

	if (!dinfop->name[0]) {
		WD_ERR("Get no matching device!\n");
		goto err_with_dinfop;
	}

	closedir(wd_class);
	if (num)
		*num = cnt;
	return dinfop;

err_with_dinfop:
	free(dinfop);
	if (wd_class)
		closedir(wd_class);
	return NULL;
}

int wd_request_queue(struct wd_queue *q)
{
	int ret, try_cnt = 0;
	struct _dev_info *dev;

try_again:
	dev = _find_available_res(&q->capa, q->dev_path, NULL);
	if (dev == NULL) {
		WD_ERR("cannot find available dev\n");
		return -ENODEV;
	}

	ret = snprintf(q->dev_path, PATH_STR_SIZE, "%s/%s", LINUX_DEV_DIR, dev->name);
	if (ret < 0) {
		WD_ERR("get /%s path fail!\n", dev->name);
		goto err_with_dev;
	}

	q->fd = open(q->dev_path, O_RDWR | O_CLOEXEC);
	if (q->fd == -1) {
		try_cnt++;
		if (try_cnt < _TRY_REQUEST_TIMES) {
			free(dev);
			dev = NULL;
			goto try_again;
		}
		WD_ERR("fail to open %s\n", q->dev_path);
		ret = -ENODEV;
		goto err_with_dev;
	}

	q->hw_type = dev->api;
	q->dev_flags = dev->flags;
	q->dev_info = dev;
	memcpy(q->qfrs_offset, dev->qfrs_offset, sizeof(q->qfrs_offset));
	ret = drv_open(q);
	if (ret) {
		WD_ERR("fail to init the queue by driver!\n");
		goto err_with_fd;
	}

	ret = ioctl(q->fd, UACCE_CMD_START);
	if (ret) {
		WD_ERR("fail to start queue of %s\n", q->dev_path);
		goto err_with_drv_openned;
	}
	return 0;

err_with_drv_openned:
	drv_close(q);
err_with_fd:
	close(q->fd);
err_with_dev:
	free(dev);
	return ret;
}

void wd_release_queue(struct wd_queue *q)
{
	if (!q || !q->dev_info) {
		WD_ERR("release queue param error!\n");
		return;
	}
	if (q->ss_size)
		drv_unmap_reserve_mem(q, q->ss_va, q->ss_size);
	drv_close(q);
	if (ioctl(q->fd, UACCE_CMD_PUT_Q)) {
		WD_ERR("fail to put queue!\n");
		return;
	}
	close(q->fd);
	free(q->dev_info);
	q->dev_info = NULL;
}

int wd_send(struct wd_queue *q, void *req)
{
	return drv_send(q, req);
}

int wd_recv(struct wd_queue *q, void **resp)
{
	return drv_recv(q, resp);
}

static int wd_wait(struct wd_queue *q, __u16 ms)
{
	struct pollfd fds[1];
	int ret;

	fds[0].fd = q->fd;
	fds[0].events = POLLIN;
	ret = poll(fds, 1, ms);
	if (ret == -1)
		return -ENODEV;

	return 0;
}

int wd_recv_sync(struct wd_queue *q, void **resp, __u16 ms)
{
	int ret;

	while (1) {
		ret = wd_recv(q, resp);
		if (ret == -EBUSY) {
			ret = wd_wait(q, ms);
			if (ret)
				return ret;
		} else
			return ret;
	}
}

void wd_flush(struct wd_queue *q)
{
	drv_flush(q);
}

void *wd_reserve_memory(struct wd_queue *q, size_t size)
{
	return drv_reserve_mem(q, size);
}

int wd_share_reserved_memory(struct wd_queue *q, struct wd_queue *target_q)
{
	return ioctl(q->fd, UACCE_CMD_SHARE_SVAS, target_q->fd);
}

int wd_get_available_dev_num(const char *algorithm)
{
	struct _dev_info *dev;
	struct wd_capa capa;
	int num = 0;

	memset(&capa, 0, sizeof(capa));
	capa.alg = algorithm;
	dev = _find_available_res(&capa, NULL, &num);
	if (dev != NULL) {
		free(dev);
		dev = NULL;
	}
	return num;
}
