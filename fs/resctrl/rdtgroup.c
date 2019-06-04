// SPDX-License-Identifier: GPL-2.0
/*
 * User interface for Resource Alloction in Resource Director Technology(RDT)
 *
 * Copyright (C) 2016 Intel Corporation
 *
 * Author: Fenghua Yu <fenghua.yu@intel.com>
 *
 * More information about RDT be found in the Intel (R) x86 Architecture
 * Software Developer Manual.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/cacheinfo.h>
#include <linux/cpu.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/fs_parser.h>
#include <linux/sysfs.h>
#include <linux/kernfs.h>
#include <linux/seq_buf.h>
#include <linux/seq_file.h>
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/slab.h>
#include <linux/task_work.h>
#include <linux/user_namespace.h>

#include <uapi/linux/magic.h>

#include "internal.h"

/* Mutex to protect rdtgroup access. */
DEFINE_MUTEX(rdtgroup_mutex);

static struct kernfs_root *rdt_root;
struct rdtgroup rdtgroup_default;
LIST_HEAD(rdt_all_groups);

static bool resctrl_cdp_enabled;

#define CDP_CLOSID_MAP_PAIR(closid, hw_c, hw_d)				\
do {									\
	if (resctrl_cdp_enabled) {					\
		hw_c = resctrl_closid_cdp_map(closid, CDP_CODE);	\
		hw_d = resctrl_closid_cdp_map(closid, CDP_DATA);	\
	} else {							\
		hw_c = resctrl_closid_cdp_map(closid, CDP_BOTH);	\
		hw_d = hw_c;						\
	}								\
} while (0)

/* list of entries for the schemata file */
LIST_HEAD(resctrl_all_schema);

/* the filesystem can only be mounted once */
bool resctrl_mounted;

/* Kernel fs node for "info" directory under root */
static struct kernfs_node *kn_info;

/* Kernel fs node for "mon_groups" directory under root */
static struct kernfs_node *kn_mongrp;

/* Kernel fs node for "mon_data" directory under root */
static struct kernfs_node *kn_mondata;

/*
 * Used to store the max resource data width
 * to display the schemata in a tabular format
 */
int max_data_width;

static struct seq_buf last_cmd_status;
static char last_cmd_status_buf[512];

struct dentry *debugfs_resctrl;

void rdt_last_cmd_clear(void)
{
	lockdep_assert_held(&rdtgroup_mutex);
	seq_buf_clear(&last_cmd_status);
}

void rdt_last_cmd_puts(const char *s)
{
	lockdep_assert_held(&rdtgroup_mutex);
	seq_buf_puts(&last_cmd_status, s);
}

void rdt_last_cmd_printf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lockdep_assert_held(&rdtgroup_mutex);
	seq_buf_vprintf(&last_cmd_status, fmt, ap);
	va_end(ap);
}

/*
 * Trivial allocator for CLOSIDs. Since h/w only supports a small number,
 * we can keep a bitmap of free CLOSIDs in a single integer.
 *
 * Using a global CLOSID across all resources has some advantages and
 * some drawbacks:
 * + We can simply set "current->closid" to assign a task to a resource
 *   group.
 * + Context switch code can avoid extra memory references deciding which
 *   CLOSID to load into the PQR_ASSOC MSR
 * - We give up some options in configuring resource groups across multi-socket
 *   systems.
 * - Our choices on how to configure each resource become progressively more
 *   limited as the number of resources grows.
 */
static u32 closid_free_map;
static u32 closid_free_map_len;

u32 closids_supported(void)
{
	return closid_free_map_len;
}

static void closid_init(void)
{
	closid_free_map_len = resctrl_arch_system_num_closid();

	/* while closid_free_map is a u32 */
	closid_free_map_len = min(closid_free_map_len, (u32)RESCTRL_MAX_CLOSID);

	if (resctrl_cdp_enabled)
		closid_free_map_len /= 2;

	closid_free_map = BIT_MASK(closid_free_map_len) - 1;

	/* CLOSID 0 is always reserved for the default group */
	closid_free_map &= ~1;
}

static int closid_alloc(void)
{
	u32 closid = ffs(closid_free_map);

	if (closid == 0)
		return -ENOSPC;
	closid--;
	closid_free_map &= ~(1 << closid);

	return closid;
}

void closid_free(int closid)
{
	closid_free_map |= 1 << closid;
}

/**
 * closid_allocated - test if provided closid is in use
 * @closid: closid to be tested
 *
 * Return: true if @closid is currently associated with a resource group,
 * false if @closid is free
 */
static bool closid_allocated(unsigned int closid)
{
	return (closid_free_map & (1 << closid)) == 0;
}

/**
 * rdtgroup_mode_by_closid - Return mode of resource group with closid
 * @closid: closid if the resource group
 *
 * Each resource group is associated with a @closid. Here the mode
 * of a resource group can be queried by searching for it using its closid.
 *
 * Return: mode as &enum rdtgrp_mode of resource group with closid @closid
 */
enum rdtgrp_mode rdtgroup_mode_by_closid(int closid)
{
	struct rdtgroup *rdtgrp;

	list_for_each_entry(rdtgrp, &rdt_all_groups, rdtgroup_list) {
		if (rdtgrp->closid == closid)
			return rdtgrp->mode;
	}

	return RDT_NUM_MODES;
}

static const char * const rdt_mode_str[] = {
	[RDT_MODE_SHAREABLE]		= "shareable",
	[RDT_MODE_EXCLUSIVE]		= "exclusive",
	[RDT_MODE_PSEUDO_LOCKSETUP]	= "pseudo-locksetup",
	[RDT_MODE_PSEUDO_LOCKED]	= "pseudo-locked",
};

/**
 * rdtgroup_mode_str - Return the string representation of mode
 * @mode: the resource group mode as &enum rdtgroup_mode
 *
 * Return: string representation of valid mode, "unknown" otherwise
 */
static const char *rdtgroup_mode_str(enum rdtgrp_mode mode)
{
	if (mode < RDT_MODE_SHAREABLE || mode >= RDT_NUM_MODES)
		return "unknown";

	return rdt_mode_str[mode];
}

/* set uid and gid of rdtgroup dirs and files to that of the creator */
static int rdtgroup_kn_set_ugid(struct kernfs_node *kn)
{
	struct iattr iattr = { .ia_valid = ATTR_UID | ATTR_GID,
				.ia_uid = current_fsuid(),
				.ia_gid = current_fsgid(), };

	if (uid_eq(iattr.ia_uid, GLOBAL_ROOT_UID) &&
	    gid_eq(iattr.ia_gid, GLOBAL_ROOT_GID))
		return 0;

	return kernfs_setattr(kn, &iattr);
}

static int rdtgroup_add_file(struct kernfs_node *parent_kn, struct rftype *rft)
{
	struct kernfs_node *kn;
	int ret;

	kn = __kernfs_create_file(parent_kn, rft->name, rft->mode,
				  GLOBAL_ROOT_UID, GLOBAL_ROOT_GID,
				  0, rft->kf_ops, rft, NULL, NULL);
	if (IS_ERR(kn))
		return PTR_ERR(kn);

	ret = rdtgroup_kn_set_ugid(kn);
	if (ret) {
		kernfs_remove(kn);
		return ret;
	}

	return 0;
}

static int rdtgroup_seqfile_show(struct seq_file *m, void *arg)
{
	struct kernfs_open_file *of = m->private;
	struct rftype *rft = of->kn->priv;

	if (rft->seq_show)
		return rft->seq_show(of, m, arg);
	return 0;
}

static ssize_t rdtgroup_file_write(struct kernfs_open_file *of, char *buf,
				   size_t nbytes, loff_t off)
{
	struct rftype *rft = of->kn->priv;

	if (rft->write)
		return rft->write(of, buf, nbytes, off);

	return -EINVAL;
}

static struct kernfs_ops rdtgroup_kf_single_ops = {
	.atomic_write_len	= PAGE_SIZE,
	.write			= rdtgroup_file_write,
	.seq_show		= rdtgroup_seqfile_show,
};

static struct kernfs_ops kf_mondata_ops = {
	.atomic_write_len	= PAGE_SIZE,
	.seq_show		= rdtgroup_mondata_show,
};

static bool is_cpu_list(struct kernfs_open_file *of)
{
	struct rftype *rft = of->kn->priv;

	return rft->flags & RFTYPE_FLAGS_CPUS_LIST;
}

static int rdtgroup_cpus_show(struct kernfs_open_file *of,
			      struct seq_file *s, void *v)
{
	struct rdtgroup *rdtgrp;
	struct cpumask *mask;
	int ret = 0;

	rdtgrp = rdtgroup_kn_lock_live(of->kn);

	if (rdtgrp) {
		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED) {
			if (!rdtgrp->plr->d) {
				rdt_last_cmd_clear();
				rdt_last_cmd_puts("Cache domain offline\n");
				ret = -ENODEV;
			} else {
				mask = &rdtgrp->plr->d->cpu_mask;
				seq_printf(s, is_cpu_list(of) ?
					   "%*pbl\n" : "%*pb\n",
					   cpumask_pr_args(mask));
			}
		} else {
			seq_printf(s, is_cpu_list(of) ? "%*pbl\n" : "%*pb\n",
				   cpumask_pr_args(&rdtgrp->cpu_mask));
		}
	} else {
		ret = -ENOENT;
	}
	rdtgroup_kn_unlock(of->kn);

	return ret;
}

/*
 * Update the PGR_ASSOC MSR on all cpus in @cpu_mask,
 *
 * Per task closids/rmids must have been set up before calling this function.
 * @r may be NULL.
 */
static void
update_closid_rmid(const struct cpumask *cpu_mask, struct rdtgroup *r)
{
	int cpu = get_cpu();
	struct resctrl_cpu_sync defaults;
	struct resctrl_cpu_sync *defaults_p = NULL;

	if (r) {
		CDP_CLOSID_MAP_PAIR(r->closid, defaults.closid_code,
				    defaults.closid_data);
		defaults.rmid = r->mon.rmid;
		defaults_p = &defaults;
	}

	if (cpumask_test_cpu(cpu, cpu_mask))
		resctrl_arch_update_cpu_defaults(defaults_p);
	smp_call_function_many(cpu_mask, resctrl_arch_update_cpu_defaults,
			       defaults_p, 1);
	put_cpu();
}

static void resctrl_set_cpu_default_closid(int cpu, u32 closid)
{
	hw_closid_t c, d;

	CDP_CLOSID_MAP_PAIR(closid, c, d);
	resctrl_arch_set_cpu_default_closid(cpu, c, d);
}

static int cpus_mon_write(struct rdtgroup *rdtgrp, cpumask_var_t newmask,
			  cpumask_var_t tmpmask)
{
	struct rdtgroup *prgrp = rdtgrp->mon.parent, *crgrp;
	struct list_head *head;

	/* Check whether cpus belong to parent ctrl group */
	cpumask_andnot(tmpmask, newmask, &prgrp->cpu_mask);
	if (cpumask_weight(tmpmask)) {
		rdt_last_cmd_puts("Can only add CPUs to mongroup that belong to parent\n");
		return -EINVAL;
	}

	/* Check whether cpus are dropped from this group */
	cpumask_andnot(tmpmask, &rdtgrp->cpu_mask, newmask);
	if (cpumask_weight(tmpmask)) {
		/* Give any dropped cpus to parent rdtgroup */
		cpumask_or(&prgrp->cpu_mask, &prgrp->cpu_mask, tmpmask);
		update_closid_rmid(tmpmask, prgrp);
	}

	/*
	 * If we added cpus, remove them from previous group that owned them
	 * and update per-cpu rmid
	 */
	cpumask_andnot(tmpmask, newmask, &rdtgrp->cpu_mask);
	if (cpumask_weight(tmpmask)) {
		head = &prgrp->mon.crdtgrp_list;
		list_for_each_entry(crgrp, head, mon.crdtgrp_list) {
			if (crgrp == rdtgrp)
				continue;
			cpumask_andnot(&crgrp->cpu_mask, &crgrp->cpu_mask,
				       tmpmask);
		}
		update_closid_rmid(tmpmask, rdtgrp);
	}

	/* Done pushing/pulling - update this group with new mask */
	cpumask_copy(&rdtgrp->cpu_mask, newmask);

	return 0;
}

static void cpumask_rdtgrp_clear(struct rdtgroup *r, struct cpumask *m)
{
	struct rdtgroup *crgrp;

	cpumask_andnot(&r->cpu_mask, &r->cpu_mask, m);
	/* update the child mon group masks as well*/
	list_for_each_entry(crgrp, &r->mon.crdtgrp_list, mon.crdtgrp_list)
		cpumask_and(&crgrp->cpu_mask, &r->cpu_mask, &crgrp->cpu_mask);
}

static int cpus_ctrl_write(struct rdtgroup *rdtgrp, cpumask_var_t newmask,
			   cpumask_var_t tmpmask, cpumask_var_t tmpmask1)
{
	struct rdtgroup *r, *crgrp;
	struct list_head *head;

	/* Check whether cpus are dropped from this group */
	cpumask_andnot(tmpmask, &rdtgrp->cpu_mask, newmask);
	if (cpumask_weight(tmpmask)) {
		/* Can't drop from default group */
		if (rdtgrp == &rdtgroup_default) {
			rdt_last_cmd_puts("Can't drop CPUs from default group\n");
			return -EINVAL;
		}

		/* Give any dropped cpus to rdtgroup_default */
		cpumask_or(&rdtgroup_default.cpu_mask,
			   &rdtgroup_default.cpu_mask, tmpmask);
		update_closid_rmid(tmpmask, &rdtgroup_default);
	}

	/*
	 * If we added cpus, remove them from previous group and
	 * the prev group's child groups that owned them
	 * and update per-cpu closid/rmid.
	 */
	cpumask_andnot(tmpmask, newmask, &rdtgrp->cpu_mask);
	if (cpumask_weight(tmpmask)) {
		list_for_each_entry(r, &rdt_all_groups, rdtgroup_list) {
			if (r == rdtgrp)
				continue;
			cpumask_and(tmpmask1, &r->cpu_mask, tmpmask);
			if (cpumask_weight(tmpmask1))
				cpumask_rdtgrp_clear(r, tmpmask1);
		}
		update_closid_rmid(tmpmask, rdtgrp);
	}

	/* Done pushing/pulling - update this group with new mask */
	cpumask_copy(&rdtgrp->cpu_mask, newmask);

	/*
	 * Clear child mon group masks since there is a new parent mask
	 * now and update the rmid for the cpus the child lost.
	 */
	head = &rdtgrp->mon.crdtgrp_list;
	list_for_each_entry(crgrp, head, mon.crdtgrp_list) {
		cpumask_and(tmpmask, &rdtgrp->cpu_mask, &crgrp->cpu_mask);
		update_closid_rmid(tmpmask, rdtgrp);
		cpumask_clear(&crgrp->cpu_mask);
	}

	return 0;
}

static ssize_t rdtgroup_cpus_write(struct kernfs_open_file *of,
				   char *buf, size_t nbytes, loff_t off)
{
	cpumask_var_t tmpmask, newmask, tmpmask1;
	struct rdtgroup *rdtgrp;
	int ret;

	if (!buf)
		return -EINVAL;

	if (!zalloc_cpumask_var(&tmpmask, GFP_KERNEL))
		return -ENOMEM;
	if (!zalloc_cpumask_var(&newmask, GFP_KERNEL)) {
		free_cpumask_var(tmpmask);
		return -ENOMEM;
	}
	if (!zalloc_cpumask_var(&tmpmask1, GFP_KERNEL)) {
		free_cpumask_var(tmpmask);
		free_cpumask_var(newmask);
		return -ENOMEM;
	}

	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	rdt_last_cmd_clear();
	if (!rdtgrp) {
		ret = -ENOENT;
		rdt_last_cmd_puts("Directory was removed\n");
		goto unlock;
	}

	if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED ||
	    rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP) {
		ret = -EINVAL;
		rdt_last_cmd_puts("Pseudo-locking in progress\n");
		goto unlock;
	}

	if (is_cpu_list(of))
		ret = cpulist_parse(buf, newmask);
	else
		ret = cpumask_parse(buf, newmask);

	if (ret) {
		rdt_last_cmd_puts("Bad CPU list/mask\n");
		goto unlock;
	}

	/* check that user didn't specify any offline cpus */
	cpumask_andnot(tmpmask, newmask, cpu_online_mask);
	if (cpumask_weight(tmpmask)) {
		ret = -EINVAL;
		rdt_last_cmd_puts("Can only assign online CPUs\n");
		goto unlock;
	}

	if (rdtgrp->type == RDTCTRL_GROUP)
		ret = cpus_ctrl_write(rdtgrp, newmask, tmpmask, tmpmask1);
	else if (rdtgrp->type == RDTMON_GROUP)
		ret = cpus_mon_write(rdtgrp, newmask, tmpmask);
	else
		ret = -EINVAL;

unlock:
	rdtgroup_kn_unlock(of->kn);
	free_cpumask_var(tmpmask);
	free_cpumask_var(newmask);
	free_cpumask_var(tmpmask1);

	return ret ?: nbytes;
}

static void resctrl_set_closid(struct task_struct *tsk, u32 closid)
{
	hw_closid_t c, d;

	CDP_CLOSID_MAP_PAIR(closid, c, d);
	resctrl_arch_set_closid(tsk, c, d);
}

static bool resctrl_match_closid(struct task_struct *tsk, u32 closid)
{
	hw_closid_t c, d;

	CDP_CLOSID_MAP_PAIR(closid, c, d);
	return resctrl_arch_match_closid(tsk, c, d);
}

struct task_move_callback {
	struct callback_head	work;
	struct rdtgroup		*rdtgrp;
};

static void move_myself(struct callback_head *head)
{
	struct task_move_callback *callback;
	struct rdtgroup *rdtgrp;

	callback = container_of(head, struct task_move_callback, work);
	rdtgrp = callback->rdtgrp;

	/*
	 * If resource group was deleted before this task work callback
	 * was invoked, then assign the task to root group and free the
	 * resource group.
	 */
	if (atomic_dec_and_test(&rdtgrp->waitcount) &&
	    (rdtgrp->flags & RDT_DELETED)) {
		resctrl_set_closid(current, 0);
		resctrl_arch_set_rmid(current, 0);
		kfree(rdtgrp);
	}

	preempt_disable();
	/* update PQR_ASSOC MSR to make resource group go into effect */
	resctrl_sched_in();
	preempt_enable();

	kfree(callback);
}

static int __rdtgroup_move_task(struct task_struct *tsk,
				struct rdtgroup *rdtgrp)
{
	struct task_move_callback *callback;
	int ret;

	callback = kzalloc(sizeof(*callback), GFP_KERNEL);
	if (!callback)
		return -ENOMEM;
	callback->work.func = move_myself;
	callback->rdtgrp = rdtgrp;

	/*
	 * Take a refcount, so rdtgrp cannot be freed before the
	 * callback has been invoked.
	 */
	atomic_inc(&rdtgrp->waitcount);
	ret = task_work_add(tsk, &callback->work, true);
	if (ret) {
		/*
		 * Task is exiting. Drop the refcount and free the callback.
		 * No need to check the refcount as the group cannot be
		 * deleted before the write function unlocks rdtgroup_mutex.
		 */
		atomic_dec(&rdtgrp->waitcount);
		kfree(callback);
		rdt_last_cmd_puts("Task exited\n");
	} else {
		/*
		 * For ctrl_mon groups move both closid and rmid.
		 * For monitor groups, can move the tasks only from
		 * their parent CTRL group.
		 */
		if (rdtgrp->type == RDTCTRL_GROUP) {
			resctrl_set_closid(tsk, rdtgrp->closid);
			resctrl_arch_set_rmid(tsk, rdtgrp->mon.rmid);
		} else if (rdtgrp->type == RDTMON_GROUP) {
			u32 parent_closid = rdtgrp->mon.parent->closid;

			if (resctrl_match_closid(tsk, parent_closid)) {
				resctrl_arch_set_rmid(tsk, rdtgrp->mon.rmid);
			} else {
				rdt_last_cmd_puts("Can't move task to different control group\n");
				ret = -EINVAL;
			}
		}
	}
	return ret;
}

static bool is_closid_match(struct task_struct *t, struct rdtgroup *r)
{
	return (resctrl_arch_alloc_capable() && (r->type == RDTCTRL_GROUP) &&
		resctrl_match_closid(t, r->closid));
}

static bool is_rmid_match(struct task_struct *t, struct rdtgroup *r)
{
	return (resctrl_arch_mon_capable() && (r->type == RDTMON_GROUP) &&
		resctrl_arch_match_rmid(t, r->mon.rmid));
}

/**
 * rdtgroup_tasks_assigned - Test if tasks have been assigned to resource group
 * @r: Resource group
 *
 * Return: 1 if tasks have been assigned to @r, 0 otherwise
 */
int rdtgroup_tasks_assigned(struct rdtgroup *r)
{
	struct task_struct *p, *t;
	int ret = 0;

	lockdep_assert_held(&rdtgroup_mutex);

	rcu_read_lock();
	for_each_process_thread(p, t) {
		if (is_closid_match(t, r) || is_rmid_match(t, r)) {
			ret = 1;
			break;
		}
	}
	rcu_read_unlock();

	return ret;
}

static int rdtgroup_task_write_permission(struct task_struct *task,
					  struct kernfs_open_file *of)
{
	const struct cred *tcred = get_task_cred(task);
	const struct cred *cred = current_cred();
	int ret = 0;

	/*
	 * Even if we're attaching all tasks in the thread group, we only
	 * need to check permissions on one of them.
	 */
	if (!uid_eq(cred->euid, GLOBAL_ROOT_UID) &&
	    !uid_eq(cred->euid, tcred->uid) &&
	    !uid_eq(cred->euid, tcred->suid)) {
		rdt_last_cmd_printf("No permission to move task %d\n", task->pid);
		ret = -EPERM;
	}

	put_cred(tcred);
	return ret;
}

static int rdtgroup_move_task(pid_t pid, struct rdtgroup *rdtgrp,
			      struct kernfs_open_file *of)
{
	struct task_struct *tsk;
	int ret;

	rcu_read_lock();
	if (pid) {
		tsk = find_task_by_vpid(pid);
		if (!tsk) {
			rcu_read_unlock();
			rdt_last_cmd_printf("No task %d\n", pid);
			return -ESRCH;
		}
	} else {
		tsk = current;
	}

	get_task_struct(tsk);
	rcu_read_unlock();

	ret = rdtgroup_task_write_permission(tsk, of);
	if (!ret)
		ret = __rdtgroup_move_task(tsk, rdtgrp);

	put_task_struct(tsk);
	return ret;
}

static ssize_t rdtgroup_tasks_write(struct kernfs_open_file *of,
				    char *buf, size_t nbytes, loff_t off)
{
	struct rdtgroup *rdtgrp;
	int ret = 0;
	pid_t pid;

	if (kstrtoint(strstrip(buf), 0, &pid) || pid < 0)
		return -EINVAL;
	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	if (!rdtgrp) {
		rdtgroup_kn_unlock(of->kn);
		return -ENOENT;
	}
	rdt_last_cmd_clear();

	if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED ||
	    rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP) {
		ret = -EINVAL;
		rdt_last_cmd_puts("Pseudo-locking in progress\n");
		goto unlock;
	}

	ret = rdtgroup_move_task(pid, rdtgrp, of);

unlock:
	rdtgroup_kn_unlock(of->kn);

	return ret ?: nbytes;
}

static void show_rdt_tasks(struct rdtgroup *r, struct seq_file *s)
{
	struct task_struct *p, *t;

	rcu_read_lock();
	for_each_process_thread(p, t) {
		if (is_closid_match(t, r) || is_rmid_match(t, r))
			seq_printf(s, "%d\n", t->pid);
	}
	rcu_read_unlock();
}

static int rdtgroup_tasks_show(struct kernfs_open_file *of,
			       struct seq_file *s, void *v)
{
	struct rdtgroup *rdtgrp;
	int ret = 0;

	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	if (rdtgrp)
		show_rdt_tasks(rdtgrp, s);
	else
		ret = -ENOENT;
	rdtgroup_kn_unlock(of->kn);

	return ret;
}

static int rdt_last_cmd_status_show(struct kernfs_open_file *of,
				    struct seq_file *seq, void *v)
{
	int len;

	mutex_lock(&rdtgroup_mutex);
	len = seq_buf_used(&last_cmd_status);
	if (len)
		seq_printf(seq, "%.*s", len, last_cmd_status_buf);
	else
		seq_puts(seq, "ok\n");
	mutex_unlock(&rdtgroup_mutex);
	return 0;
}

static int rdt_num_closids_show(struct kernfs_open_file *of,
				struct seq_file *seq, void *v)
{
	/*
	 * Export closid_free_map_len not the arch's value as we may have
	 * reduced this for CDP.
	 */
	seq_printf(seq, "%d\n", closid_free_map_len);
	return 0;
}

static int rdt_default_ctrl_show(struct kernfs_open_file *of,
			     struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	struct rdt_resource *r = s->res;

	seq_printf(seq, "%x\n", r->default_ctrl);
	return 0;
}

static int rdt_min_cbm_bits_show(struct kernfs_open_file *of,
			     struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	struct rdt_resource *r = s->res;

	seq_printf(seq, "%u\n", r->cache.min_cbm_bits);
	return 0;
}

static int rdt_shareable_bits_show(struct kernfs_open_file *of,
				   struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	struct rdt_resource *r = s->res;

	seq_printf(seq, "%x\n", r->cache.shareable_bits);
	return 0;
}

static u32 resctrl_get_config(u32 closid, struct resctrl_schema *s, struct rdt_domain *d)
{
	enum resctrl_conf_type t = s->conf_type;
	struct rdt_resource *r = s->res;
	hw_closid_t hw_closid;
	u32 ret;

	hw_closid = resctrl_closid_cdp_map(closid, t);
	resctrl_arch_get_config(r, d, hw_closid, &ret);

	return ret;
}

/**
 * rdt_bit_usage_show - Display current usage of resources
 *
 * A domain is a shared resource that can now be allocated differently. Here
 * we display the current regions of the domain as an annotated bitmask.
 * For each domain of this resource its allocation bitmask
 * is annotated as below to indicate the current usage of the corresponding bit:
 *   0 - currently unused
 *   X - currently available for sharing and used by software and hardware
 *   H - currently used by hardware only but available for software use
 *   S - currently used and shareable by software only
 *   E - currently used exclusively by one resource group
 *   P - currently pseudo-locked by one resource group
 */
static int rdt_bit_usage_show(struct kernfs_open_file *of,
			      struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	u32 sw_shareable = 0, hw_shareable = 0;
	u32 exclusive = 0, pseudo_locked = 0;
	struct rdt_resource *r = s->res;
	struct rdt_domain *dom;
	int i, hwb, swb, excl, psl;
	enum rdtgrp_mode mode;
	bool sep = false;
	u32 ctrl_val;

	cpus_read_lock();
	mutex_lock(&rdtgroup_mutex);
	hw_shareable = r->cache.shareable_bits;
	list_for_each_entry(dom, &r->domains, list) {
		if (sep)
			seq_putc(seq, ';');
		sw_shareable = 0;
		exclusive = 0;
		seq_printf(seq, "%d=", dom->id);
		for (i = 0; i < closids_supported(); i++) {
			if (!closid_allocated(i))
				continue;
			ctrl_val = resctrl_get_config(i, s, dom);
			mode = rdtgroup_mode_by_closid(i);
			switch (mode) {
			case RDT_MODE_SHAREABLE:
				sw_shareable |= ctrl_val;
				break;
			case RDT_MODE_EXCLUSIVE:
				exclusive |= ctrl_val;
				break;
			case RDT_MODE_PSEUDO_LOCKSETUP:
			/*
			 * RDT_MODE_PSEUDO_LOCKSETUP is possible
			 * here but not included since the CBM
			 * associated with this CLOSID in this mode
			 * is not initialized and no task or cpu can be
			 * assigned this CLOSID.
			 */
				break;
			case RDT_MODE_PSEUDO_LOCKED:
			case RDT_NUM_MODES:
				WARN(1,
				     "invalid mode for closid %d\n", i);
				break;
			}
		}
		for (i = r->cache.cbm_len - 1; i >= 0; i--) {
			pseudo_locked = dom->plr ? dom->plr->cbm : 0;
			hwb = test_bit(i, (unsigned long *)&hw_shareable);
			swb = test_bit(i, (unsigned long *)&sw_shareable);
			excl = test_bit(i, (unsigned long *)&exclusive);
			psl = test_bit(i, (unsigned long *)&pseudo_locked);
			if (hwb && swb)
				seq_putc(seq, 'X');
			else if (hwb && !swb)
				seq_putc(seq, 'H');
			else if (!hwb && swb)
				seq_putc(seq, 'S');
			else if (excl)
				seq_putc(seq, 'E');
			else if (psl)
				seq_putc(seq, 'P');
			else /* Unused bits remain */
				seq_putc(seq, '0');
		}
		sep = true;
	}
	seq_putc(seq, '\n');
	mutex_unlock(&rdtgroup_mutex);
	cpus_read_unlock();

	return 0;
}

static int rdt_min_bw_show(struct kernfs_open_file *of,
			     struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	struct rdt_resource *r = s->res;

	seq_printf(seq, "%u\n", r->membw.min_bw);
	return 0;
}

static int rdt_num_rmids_show(struct kernfs_open_file *of,
			      struct seq_file *seq, void *v)
{
	seq_printf(seq, "%d\n", resctrl_arch_system_num_rmid());

	return 0;
}

static int rdt_mon_features_show(struct kernfs_open_file *of,
				 struct seq_file *seq, void *v)
{
	struct rdt_resource *r = of->kn->parent->priv;
	struct mon_evt *mevt;

	list_for_each_entry(mevt, &r->evt_list, list)
		seq_printf(seq, "%s\n", mevt->name);

	return 0;
}

static int rdt_bw_gran_show(struct kernfs_open_file *of,
			     struct seq_file *seq, void *v)
{
	struct rdt_resource *r = of->kn->parent->priv;

	seq_printf(seq, "%u\n", r->membw.bw_gran);
	return 0;
}

static int rdt_delay_linear_show(struct kernfs_open_file *of,
			     struct seq_file *seq, void *v)
{
	struct resctrl_schema *s = of->kn->parent->priv;
	struct rdt_resource *r = s->res;

	seq_printf(seq, "%u\n", r->membw.delay_linear);
	return 0;
}

static int max_threshold_occ_show(struct kernfs_open_file *of,
				  struct seq_file *seq, void *v)
{
	seq_printf(seq, "%u\n", resctrl_rmid_realloc_threshold);

	return 0;
}

static ssize_t max_threshold_occ_write(struct kernfs_open_file *of,
				       char *buf, size_t nbytes, loff_t off)
{
	unsigned int bytes;
	int ret;

	ret = kstrtouint(buf, 0, &bytes);
	if (ret)
		return ret;

	if (bytes > resctrl_arch_max_rmid_threshold())
		return -EINVAL;

	resctrl_rmid_realloc_threshold = bytes;

	return nbytes;
}

/*
 * rdtgroup_mode_show - Display mode of this resource group
 */
static int rdtgroup_mode_show(struct kernfs_open_file *of,
			      struct seq_file *s, void *v)
{
	struct rdtgroup *rdtgrp;

	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	if (!rdtgrp) {
		rdtgroup_kn_unlock(of->kn);
		return -ENOENT;
	}

	seq_printf(s, "%s\n", rdtgroup_mode_str(rdtgrp->mode));

	rdtgroup_kn_unlock(of->kn);
	return 0;
}

static u32 resctrl_get_config_cdp(u32 closid, struct resctrl_schema *s, struct rdt_domain *d)
{
	enum resctrl_conf_type t = s->conf_type;
	struct rdt_resource *r = s->res;
	hw_closid_t hw_closid;
	u32 ctrl, tmp;

	hw_closid = resctrl_closid_cdp_map(closid, t);
	resctrl_arch_get_config(r, d, hw_closid, &ctrl);

	if (s->res->cdp_enabled) {
		if (t == CDP_CODE)
			t = CDP_DATA;
		else
			t = CDP_CODE;
		hw_closid = resctrl_closid_cdp_map(closid, t);
		resctrl_arch_get_config(r, d, hw_closid, &tmp);
		ctrl |= tmp;
	}

	return ctrl;
}

/**
 * rdtgroup_cbm_overlaps - Does CBM for intended closid overlap with other
 *			   use of hardware
 * @s: Schema for the resource to which domain instance @d belongs.
 * @d: The domain instance for which @closid is being tested.
 * @cbm: Capacity bitmask being tested.
 * @closid: Intended closid for @cbm.
 * @exclusive: Only check if overlaps with exclusive resource groups
 *
 * Checks if provided @cbm intended to be used for @closid on domain
 * @d overlaps with any other closids or other hardware usage associated
 * with this domain. If @exclusive is true then only overlaps with
 * resource groups in exclusive mode will be considered. If @exclusive
 * is false then overlaps with any resource group or hardware entities
 * will be considered.
 *
 * Overlap test is not limited to the specific resource for
 * which the CBM is intended though - when dealing with CDP resources that
 * share the underlying hardware the overlap check should be performed on
 * the CDP resource sharing the hardware also.
 *
 * @cbm is unsigned long, even if only 32 bits are used, to make the
 * bitmap functions work correctly.
 *
 * Return: false if CBM does not overlap, true if it does.
 */
bool rdtgroup_cbm_overlaps(struct resctrl_schema *s, struct rdt_domain *d,
			   unsigned long cbm, int closid, bool exclusive)
{
	struct rdt_resource *r = s->res;
	enum rdtgrp_mode mode;
	unsigned long ctrl_b;
	int i;

	/* Check for any overlap with regions used by hardware directly */
	if (!exclusive) {
		ctrl_b = r->cache.shareable_bits;
		if (bitmap_intersects(&cbm, &ctrl_b, r->cache.cbm_len))
			return true;
	}

	/* Check for overlap with other resource groups */
	for (i = 0; i < closids_supported(); i++) {
		ctrl_b = resctrl_get_config_cdp(i, s, d);
		mode = rdtgroup_mode_by_closid(i);
		if (closid_allocated(i) && i != closid &&
		    mode != RDT_MODE_PSEUDO_LOCKSETUP) {
			if (bitmap_intersects(&cbm, &ctrl_b, r->cache.cbm_len)) {
				if (exclusive) {
					if (mode == RDT_MODE_EXCLUSIVE)
						return true;
					continue;
				}
				return true;
			}
		}
	}

	return false;
}

/**
 * rdtgroup_mode_test_exclusive - Test if this resource group can be exclusive
 *
 * An exclusive resource group implies that there should be no sharing of
 * its allocated resources. At the time this group is considered to be
 * exclusive this test can determine if its current schemata supports this
 * setting by testing for overlap with all other resource groups.
 *
 * Return: true if resource group can be exclusive, false if there is overlap
 * with allocations of other resource groups and thus this resource group
 * cannot be exclusive.
 */
static bool rdtgroup_mode_test_exclusive(struct rdtgroup *rdtgrp)
{
	int closid = rdtgrp->closid;
	struct resctrl_schema *s;
	struct rdt_resource *r;
	bool has_cache = false;
	hw_closid_t hw_closid;
	struct rdt_domain *d;
	u32 ctrl;

	lockdep_assert_cpus_held();

	list_for_each_entry(s, &resctrl_all_schema, list) {
		r = s->res;
		if (r->rid == RDT_RESOURCE_MBA)
			continue;
		has_cache = true;

		hw_closid = resctrl_closid_cdp_map(closid, s->conf_type);

		list_for_each_entry(d, &r->domains, list) {
			resctrl_arch_get_config(r, d, hw_closid, &ctrl);
			if (rdtgroup_cbm_overlaps(s, d, ctrl, closid, false)) {
				rdt_last_cmd_puts("Schemata overlaps\n");
				return false;
			}
		}
	}

	if (!has_cache) {
		rdt_last_cmd_puts("Cannot be exclusive without CAT/CDP\n");
		return false;
	}

	return true;
}

/**
 * rdtgroup_mode_write - Modify the resource group's mode
 *
 */
static ssize_t rdtgroup_mode_write(struct kernfs_open_file *of,
				   char *buf, size_t nbytes, loff_t off)
{
	struct rdtgroup *rdtgrp;
	enum rdtgrp_mode mode;
	int ret = 0;

	/* Valid input requires a trailing newline */
	if (nbytes == 0 || buf[nbytes - 1] != '\n')
		return -EINVAL;
	buf[nbytes - 1] = '\0';

	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	if (!rdtgrp) {
		rdtgroup_kn_unlock(of->kn);
		return -ENOENT;
	}

	rdt_last_cmd_clear();

	mode = rdtgrp->mode;

	if ((!strcmp(buf, "shareable") && mode == RDT_MODE_SHAREABLE) ||
	    (!strcmp(buf, "exclusive") && mode == RDT_MODE_EXCLUSIVE) ||
	    (!strcmp(buf, "pseudo-locksetup") &&
	     mode == RDT_MODE_PSEUDO_LOCKSETUP) ||
	    (!strcmp(buf, "pseudo-locked") && mode == RDT_MODE_PSEUDO_LOCKED))
		goto out;

	if (mode == RDT_MODE_PSEUDO_LOCKED) {
		rdt_last_cmd_puts("Cannot change pseudo-locked group\n");
		ret = -EINVAL;
		goto out;
	}

	if (!strcmp(buf, "shareable")) {
		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP) {
			ret = rdtgroup_locksetup_exit(rdtgrp);
			if (ret)
				goto out;
		}
		rdtgrp->mode = RDT_MODE_SHAREABLE;
	} else if (!strcmp(buf, "exclusive")) {
		if (!rdtgroup_mode_test_exclusive(rdtgrp)) {
			ret = -EINVAL;
			goto out;
		}
		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP) {
			ret = rdtgroup_locksetup_exit(rdtgrp);
			if (ret)
				goto out;
		}
		rdtgrp->mode = RDT_MODE_EXCLUSIVE;
	} else if (IS_ENABLED(RESCTRL_FS_PSEUDO_LOCK) &&
		   !strcmp(buf, "pseudo-locksetup")) {
		ret = rdtgroup_locksetup_enter(rdtgrp);
		if (ret)
			goto out;
		rdtgrp->mode = RDT_MODE_PSEUDO_LOCKSETUP;
	} else {
		rdt_last_cmd_puts("Unknown or unsupported mode\n");
		ret = -EINVAL;
	}

out:
	rdtgroup_kn_unlock(of->kn);
	return ret ?: nbytes;
}

/**
 * rdtgroup_cbm_to_size - Translate CBM to size in bytes
 * @r: RDT resource to which @d belongs.
 * @d: RDT domain instance.
 * @cbm: bitmask for which the size should be computed.
 *
 * The bitmask provided associated with the RDT domain instance @d will be
 * translated into how many bytes it represents. The size in bytes is
 * computed by first dividing the total cache size by the CBM length to
 * determine how many bytes each bit in the bitmask represents. The result
 * is multiplied with the number of bits set in the bitmask.
 *
 * @cbm is unsigned long, even if only 32 bits are used to make the
 * bitmap functions work correctly.
 */
unsigned int rdtgroup_cbm_to_size(struct rdt_resource *r,
				  struct rdt_domain *d, unsigned long cbm)
{
	struct cpu_cacheinfo *ci;
	unsigned int size = 0;
	int num_b, i;

	num_b = bitmap_weight(&cbm, r->cache.cbm_len);
	ci = get_cpu_cacheinfo(cpumask_any(&d->cpu_mask));
	for (i = 0; i < ci->num_leaves; i++) {
		if (ci->info_list[i].level == r->cache_level) {
			size = ci->info_list[i].size / r->cache.cbm_len * num_b;
			break;
		}
	}

	return size;
}

/**
 * rdtgroup_size_show - Display size in bytes of allocated regions
 *
 * The "size" file mirrors the layout of the "schemata" file, printing the
 * size in bytes of each region instead of the capacity bitmask.
 *
 */
static int rdtgroup_size_show(struct kernfs_open_file *of,
			      struct seq_file *s, void *v)
{
	struct resctrl_schema *schema;
	struct rdtgroup *rdtgrp;
	struct rdt_resource *r;
	hw_closid_t hw_closid;
	struct rdt_domain *d;
	unsigned int size;
	int ret = 0;
	bool sep;
	u32 ctrl;

	rdtgrp = rdtgroup_kn_lock_live(of->kn);
	if (!rdtgrp) {
		rdtgroup_kn_unlock(of->kn);
		return -ENOENT;
	}

	if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED) {
		if (!rdtgrp->plr->d) {
			rdt_last_cmd_clear();
			rdt_last_cmd_puts("Cache domain offline\n");
			ret = -ENODEV;
		} else {
			seq_printf(s, "%*s:", RESCTRL_NAME_LEN,
				   rdtgrp->plr->s->name);
			size = rdtgroup_cbm_to_size(rdtgrp->plr->s->res,
						    rdtgrp->plr->d,
						    rdtgrp->plr->cbm);
			seq_printf(s, "%d=%u\n", rdtgrp->plr->d->id, size);
		}
		goto out;
	}

	list_for_each_entry(schema, &resctrl_all_schema, list) {
		r = schema->res;

		sep = false;
		hw_closid = resctrl_closid_cdp_map(rdtgrp->closid,
						   schema->conf_type);
		seq_printf(s, "%*s:", RESCTRL_NAME_LEN, schema->name);
		list_for_each_entry(d, &r->domains, list) {
			if (sep)
				seq_putc(s, ';');
			if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP) {
				size = 0;
			} else {
				resctrl_arch_get_config(r, d, hw_closid,
							&ctrl);
				if (r->rid == RDT_RESOURCE_MBA)
					size = ctrl;
				else
					size = rdtgroup_cbm_to_size(r, d, ctrl);
			}
			seq_printf(s, "%d=%u", d->id, size);
			sep = true;
		}
		seq_putc(s, '\n');
	}

out:
	rdtgroup_kn_unlock(of->kn);

	return ret;
}

/* rdtgroup information files for one cache resource. */
static struct rftype res_common_files[] = {
	{
		.name		= "last_cmd_status",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_last_cmd_status_show,
		.fflags		= RF_TOP_INFO,
	},
	{
		.name		= "num_closids",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_num_closids_show,
		.fflags		= RF_CTRL_INFO,
	},
	{
		.name		= "mon_features",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_mon_features_show,
		.fflags		= RF_MON_INFO,
	},
	{
		.name		= "num_rmids",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_num_rmids_show,
		.fflags		= RF_MON_INFO,
	},
	{
		.name		= "cbm_mask",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_default_ctrl_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_CACHE,
	},
	{
		.name		= "min_cbm_bits",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_min_cbm_bits_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_CACHE,
	},
	{
		.name		= "shareable_bits",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_shareable_bits_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_CACHE,
	},
	{
		.name		= "bit_usage",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_bit_usage_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_CACHE,
	},
	{
		.name		= "min_bandwidth",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_min_bw_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_MB,
	},
	{
		.name		= "bandwidth_gran",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_bw_gran_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_MB,
	},
	{
		.name		= "delay_linear",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdt_delay_linear_show,
		.fflags		= RF_CTRL_INFO | RFTYPE_RES_MB,
	},
	{
		.name		= "max_threshold_occupancy",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= max_threshold_occ_write,
		.seq_show	= max_threshold_occ_show,
		.fflags		= RF_MON_INFO | RFTYPE_RES_CACHE,
	},
	{
		.name		= "cpus",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= rdtgroup_cpus_write,
		.seq_show	= rdtgroup_cpus_show,
		.fflags		= RFTYPE_BASE,
	},
	{
		.name		= "cpus_list",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= rdtgroup_cpus_write,
		.seq_show	= rdtgroup_cpus_show,
		.flags		= RFTYPE_FLAGS_CPUS_LIST,
		.fflags		= RFTYPE_BASE,
	},
	{
		.name		= "tasks",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= rdtgroup_tasks_write,
		.seq_show	= rdtgroup_tasks_show,
		.fflags		= RFTYPE_BASE,
	},
	{
		.name		= "schemata",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= rdtgroup_schemata_write,
		.seq_show	= rdtgroup_schemata_show,
		.fflags		= RF_CTRL_BASE,
	},
	{
		.name		= "mode",
		.mode		= 0644,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.write		= rdtgroup_mode_write,
		.seq_show	= rdtgroup_mode_show,
		.fflags		= RF_CTRL_BASE,
	},
	{
		.name		= "size",
		.mode		= 0444,
		.kf_ops		= &rdtgroup_kf_single_ops,
		.seq_show	= rdtgroup_size_show,
		.fflags		= RF_CTRL_BASE,
	},

};

static int rdtgroup_add_files(struct kernfs_node *kn, unsigned long fflags)
{
	struct rftype *rfts, *rft;
	int ret, len;

	rfts = res_common_files;
	len = ARRAY_SIZE(res_common_files);

	lockdep_assert_held(&rdtgroup_mutex);

	for (rft = rfts; rft < rfts + len; rft++) {
		if ((fflags & rft->fflags) == rft->fflags) {
			ret = rdtgroup_add_file(kn, rft);
			if (ret)
				goto error;
		}
	}

	return 0;
error:
	pr_warn("Failed to add %s, err=%d\n", rft->name, ret);
	while (--rft >= rfts) {
		if ((fflags & rft->fflags) == rft->fflags)
			kernfs_remove_by_name(kn, rft->name);
	}
	return ret;
}

/**
 * rdtgroup_kn_mode_restrict - Restrict user access to named resctrl file
 * @r: The resource group with which the file is associated.
 * @name: Name of the file
 *
 * The permissions of named resctrl file, directory, or link are modified
 * to not allow read, write, or execute by any user.
 *
 * WARNING: This function is intended to communicate to the user that the
 * resctrl file has been locked down - that it is not relevant to the
 * particular state the system finds itself in. It should not be relied
 * on to protect from user access because after the file's permissions
 * are restricted the user can still change the permissions using chmod
 * from the command line.
 *
 * Return: 0 on success, <0 on failure.
 */
int rdtgroup_kn_mode_restrict(struct rdtgroup *r, const char *name)
{
	struct iattr iattr = {.ia_valid = ATTR_MODE,};
	struct kernfs_node *kn;
	int ret = 0;

	kn = kernfs_find_and_get_ns(r->kn, name, NULL);
	if (!kn)
		return -ENOENT;

	switch (kernfs_type(kn)) {
	case KERNFS_DIR:
		iattr.ia_mode = S_IFDIR;
		break;
	case KERNFS_FILE:
		iattr.ia_mode = S_IFREG;
		break;
	case KERNFS_LINK:
		iattr.ia_mode = S_IFLNK;
		break;
	}

	ret = kernfs_setattr(kn, &iattr);
	kernfs_put(kn);
	return ret;
}

/**
 * rdtgroup_kn_mode_restore - Restore user access to named resctrl file
 * @r: The resource group with which the file is associated.
 * @name: Name of the file
 * @mask: Mask of permissions that should be restored
 *
 * Restore the permissions of the named file. If @name is a directory the
 * permissions of its parent will be used.
 *
 * Return: 0 on success, <0 on failure.
 */
int rdtgroup_kn_mode_restore(struct rdtgroup *r, const char *name,
			     umode_t mask)
{
	struct iattr iattr = {.ia_valid = ATTR_MODE,};
	struct kernfs_node *kn, *parent;
	struct rftype *rfts, *rft;
	int ret, len;

	rfts = res_common_files;
	len = ARRAY_SIZE(res_common_files);

	for (rft = rfts; rft < rfts + len; rft++) {
		if (!strcmp(rft->name, name))
			iattr.ia_mode = rft->mode & mask;
	}

	kn = kernfs_find_and_get_ns(r->kn, name, NULL);
	if (!kn)
		return -ENOENT;

	switch (kernfs_type(kn)) {
	case KERNFS_DIR:
		parent = kernfs_get_parent(kn);
		if (parent) {
			iattr.ia_mode |= parent->mode;
			kernfs_put(parent);
		}
		iattr.ia_mode |= S_IFDIR;
		break;
	case KERNFS_FILE:
		iattr.ia_mode |= S_IFREG;
		break;
	case KERNFS_LINK:
		iattr.ia_mode |= S_IFLNK;
		break;
	}

	ret = kernfs_setattr(kn, &iattr);
	kernfs_put(kn);
	return ret;
}

static int rdtgroup_mkdir_info_resdir(void *priv, char *name,
				      unsigned long fflags)
{
	struct kernfs_node *kn_subdir;
	int ret;

	kn_subdir = kernfs_create_dir(kn_info, name,
				      kn_info->mode, priv);
	if (IS_ERR(kn_subdir))
		return PTR_ERR(kn_subdir);

	kernfs_get(kn_subdir);
	ret = rdtgroup_kn_set_ugid(kn_subdir);
	if (ret)
		return ret;

	ret = rdtgroup_add_files(kn_subdir, fflags);
	if (!ret)
		kernfs_activate(kn_subdir);

	return ret;
}

static int rdtgroup_create_info_dir(struct kernfs_node *parent_kn)
{
	enum resctrl_resource_level i;
	struct resctrl_schema *s;
	struct rdt_resource *r;
	unsigned long fflags;
	char name[32];
	int ret;

	/* create the directory */
	kn_info = kernfs_create_dir(parent_kn, "info", parent_kn->mode, NULL);
	if (IS_ERR(kn_info))
		return PTR_ERR(kn_info);
	kernfs_get(kn_info);

	ret = rdtgroup_add_files(kn_info, RF_TOP_INFO);
	if (ret)
		goto out_destroy;

	list_for_each_entry(s, &resctrl_all_schema, list) {
		r = s->res;
		fflags =  r->fflags | RF_CTRL_INFO;
		ret = rdtgroup_mkdir_info_resdir(s, s->name, fflags);
		if (ret)
			goto out_destroy;
	}

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		r = resctrl_arch_get_resource(i);
		if (!r->mon_capable)
			continue;

		fflags =  r->fflags | RF_MON_INFO;
		sprintf(name, "%s_MON", r->name);
		ret = rdtgroup_mkdir_info_resdir(r, name, fflags);
		if (ret)
			goto out_destroy;
	}

	/*
	 * This extra ref will be put in kernfs_remove() and guarantees
	 * that @rdtgrp->kn is always accessible.
	 */
	kernfs_get(kn_info);

	ret = rdtgroup_kn_set_ugid(kn_info);
	if (ret)
		goto out_destroy;

	kernfs_activate(kn_info);

	return 0;

out_destroy:
	kernfs_remove(kn_info);
	return ret;
}

static int
mongroup_create_dir(struct kernfs_node *parent_kn, struct rdtgroup *prgrp,
		    char *name, struct kernfs_node **dest_kn)
{
	struct kernfs_node *kn;
	int ret;

	/* create the directory */
	kn = kernfs_create_dir(parent_kn, name, parent_kn->mode, prgrp);
	if (IS_ERR(kn))
		return PTR_ERR(kn);

	if (dest_kn)
		*dest_kn = kn;

	/*
	 * This extra ref will be put in kernfs_remove() and guarantees
	 * that @rdtgrp->kn is always accessible.
	 */
	kernfs_get(kn);

	ret = rdtgroup_kn_set_ugid(kn);
	if (ret)
		goto out_destroy;

	kernfs_activate(kn);

	return 0;

out_destroy:
	kernfs_remove(kn);
	return ret;
}

static inline bool is_mba_linear(void)
{
	return resctrl_arch_get_resource(RDT_RESOURCE_MBA)->membw.delay_linear;
}

/*
 * MBA software controller is supported only if
 * MBM is supported and MBA is in linear scale.
 */
static bool supports_mba_mbps(void)
{
	struct rdt_resource *r = resctrl_arch_get_resource(RDT_RESOURCE_MBA);

	return (resctrl_is_mbm_enabled() &&
		r->alloc_capable && is_mba_linear());
}

/*
 * Enable or disable the MBA software controller
 * which helps user specify bandwidth in MBps.
 */
static int set_mba_sc(bool mba_sc)
{
	struct rdt_resource *r = resctrl_arch_get_resource(RDT_RESOURCE_MBA);

	if (!supports_mba_mbps() || mba_sc == is_mba_sc(r))
		return -EINVAL;

	r->membw.mba_sc = mba_sc;

	return 0;
}

/*
 *  If the architecture has nothing to do when toggling CDP, this is all
 *  resctrl expects to see.
 */
int __weak resctrl_arch_set_cdp_enabled(bool enabled)
{
	enum resctrl_resource_level i;
	struct rdt_resource *r;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		r = resctrl_arch_get_resource(i);
		if (r->cdp_capable)
			r->cdp_enabled = enabled;
	}

	return 0;
}

static int try_to_enable_cdp(int level)
{
	int err;
	struct rdt_resource *r = resctrl_arch_get_resource(level);

	if (!r->cdp_capable)
		return -EINVAL;

	err = resctrl_arch_set_cdp_enabled(true);
	if (!err)
		resctrl_cdp_enabled = true;

	return err;
}

/*
 * We don't allow rdtgroup directories to be created anywhere
 * except the root directory. Thus when looking for the rdtgroup
 * structure for a kernfs node we are either looking at a directory,
 * in which case the rdtgroup structure is pointed at by the "priv"
 * field, otherwise we have a file, and need only look to the parent
 * to find the rdtgroup.
 */
static struct rdtgroup *kernfs_to_rdtgroup(struct kernfs_node *kn)
{
	if (kernfs_type(kn) == KERNFS_DIR) {
		/*
		 * All the resource directories use "kn->priv"
		 * to point to the "struct rdtgroup" for the
		 * resource. "info" and its subdirectories don't
		 * have rdtgroup structures, so return NULL here.
		 */
		if (kn == kn_info || kn->parent == kn_info)
			return NULL;
		else
			return kn->priv;
	} else {
		return kn->parent->priv;
	}
}

struct rdtgroup *rdtgroup_kn_lock_live(struct kernfs_node *kn)
{
	struct rdtgroup *rdtgrp = kernfs_to_rdtgroup(kn);

	if (!rdtgrp)
		return NULL;

	atomic_inc(&rdtgrp->waitcount);
	kernfs_break_active_protection(kn);

	cpus_read_lock();
	mutex_lock(&rdtgroup_mutex);

	/* Was this group deleted while we waited? */
	if (rdtgrp->flags & RDT_DELETED)
		return NULL;

	return rdtgrp;
}

void rdtgroup_kn_unlock(struct kernfs_node *kn)
{
	struct rdtgroup *rdtgrp = kernfs_to_rdtgroup(kn);

	if (!rdtgrp)
		return;

	mutex_unlock(&rdtgroup_mutex);
	cpus_read_unlock();

	if (atomic_dec_and_test(&rdtgrp->waitcount) &&
	    (rdtgrp->flags & RDT_DELETED)) {
		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP ||
		    rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED)
			rdtgroup_pseudo_lock_remove(rdtgrp);
		kernfs_unbreak_active_protection(kn);
		kernfs_put(rdtgrp->kn);
		kfree(rdtgrp);
	} else {
		kernfs_unbreak_active_protection(kn);
	}
}

static int mkdir_mondata_all(struct kernfs_node *parent_kn,
			     struct rdtgroup *prgrp,
			     struct kernfs_node **mon_data_kn);

static int allocate_domain_mba_sc(int cpu, struct rdt_resource *res,
				  struct rdt_domain *d)
{
	u32 num_closid = closid_free_map_len;
	int i;

	d->mba_sc = kcalloc_node(num_closid, sizeof(*d->mba_sc),
				 GFP_KERNEL, cpu_to_node(cpu));
	if (!d->mba_sc)
		return -ENOMEM;

	for (i = 0; i < num_closid; i++)
		d->mba_sc[i].mbps_val = MBA_MAX_MBPS;

	return 0;
}

static int allocate_mba_sc(struct rdt_resource *r)
{
	int ret, cpu;
	struct rdt_domain *d;

	lockdep_assert_cpus_held();

	list_for_each_entry(d, &r->domains, list) {
		cpu = cpumask_any(&d->cpu_mask);
		ret = allocate_domain_mba_sc(cpu, r, d);
		if (ret)
			break;
	}

	return ret;
}

static void destroy_domain_mba_sc(struct rdt_resource *r,
				  struct rdt_domain *d)
{
	kfree(d->mba_sc);
	d->mba_sc = NULL;
}

static void destroy_mba_sc(struct rdt_resource *r)
{
	struct rdt_domain *d;

	lockdep_assert_cpus_held();

	list_for_each_entry(d, &r->domains, list)
		destroy_domain_mba_sc(r, d);
}

static int rdt_enable_ctx(struct rdt_fs_context *ctx)
{
	int ret = 0;

	if (ctx->enable_cdpl2)
		ret = try_to_enable_cdp(RDT_RESOURCE_L2);

	if (!ret && ctx->enable_cdpl3)
		ret = try_to_enable_cdp(RDT_RESOURCE_L3);

	if (!ret && ctx->enable_mba_mbps)
		ret = set_mba_sc(true);

	return ret;
}

static int add_schema(enum resctrl_conf_type t, struct rdt_resource *r)
{
	char *suffix = "";
	struct resctrl_schema *s;

	s = kzalloc(sizeof(*s), GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->res = r;
	s->conf_type = t;

	switch (t) {
	case CDP_CODE:
		suffix = "CODE";
		break;
	case CDP_DATA:
		suffix = "DATA";
		break;
	case CDP_BOTH:
		suffix = "";
		break;
        }

	WARN_ON_ONCE(strlen(r->name) + strlen(suffix) + 1 > RESCTRL_NAME_LEN);
	snprintf(s->name, sizeof(s->name), "%s%s", r->name, suffix);

	/*
	 * Choose a width for the resource data based on the resource that has
	 * widest cbm/data_width.
	 */
	max_data_width = max(max_data_width, r->data_width);

	INIT_LIST_HEAD(&s->list);
	list_add(&s->list, &resctrl_all_schema);

        return 0;
}

static int create_schemata_list(void)
{
	int ret;
	struct rdt_resource *r;
	enum resctrl_resource_level i;

	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		r = resctrl_arch_get_resource(i);
		if (!r->alloc_capable)
			continue;

		if (r->cdp_enabled) {
			ret = add_schema(CDP_CODE, r);
			ret |= add_schema(CDP_DATA, r);
		} else {
			ret = add_schema(CDP_BOTH, r);
		}
		if (ret)
			break;

		if (is_mba_sc(r)) {
			ret = allocate_mba_sc(r);
			if (ret)
				break;
		}
	}

	return ret;
}

/*
 * During rdt_kill_sb(), the mba_sc state is reset before
 * destroy_schemata_list() is called: unconditionally try to free the
 * array.
 */
static void destroy_schemata_list(void)
{
	struct resctrl_schema *s, *tmp;

	list_for_each_entry_safe(s, tmp, &resctrl_all_schema, list) {
		list_del(&s->list);
		destroy_mba_sc(s->res);
		kfree(s);
	}
}

static void cdp_disable_all(void)
{
	resctrl_arch_set_cdp_enabled(false);
	resctrl_cdp_enabled = false;
}

static int rdt_get_tree(struct fs_context *fc)
{
	struct rdt_resource *l3 = resctrl_arch_get_resource(RDT_RESOURCE_L3);
	struct rdt_fs_context *ctx = rdt_fc2context(fc);
	struct rdt_domain *dom;
	int ret;

	cpus_read_lock();
	mutex_lock(&rdtgroup_mutex);
	/*
	 * resctrl file system can only be mounted once.
	 */
	if (resctrl_mounted) {
		ret = -EBUSY;
		goto out;
	}

	ret = rdt_enable_ctx(ctx);
	if (ret < 0)
		goto out_cdp;

	ret = create_schemata_list();
	if (ret) {
		destroy_schemata_list();
		goto out_mba;
	}

	closid_init();

	ret = rdtgroup_create_info_dir(rdtgroup_default.kn);
	if (ret < 0)
		goto out_schemata_free;

	if (resctrl_arch_mon_capable()) {
		ret = mongroup_create_dir(rdtgroup_default.kn,
					  NULL, "mon_groups",
					  &kn_mongrp);
		if (ret < 0)
			goto out_info;
		kernfs_get(kn_mongrp);

		ret = mkdir_mondata_all(rdtgroup_default.kn,
					&rdtgroup_default, &kn_mondata);
		if (ret < 0)
			goto out_mongrp;
		kernfs_get(kn_mondata);
		rdtgroup_default.mon.mon_data_kn = kn_mondata;
	}

	if (IS_ENABLED(RESCTRL_FS_PSEUDO_LOCK)) {
		ret = rdt_pseudo_lock_init();
		if (ret)
			goto out_mondata;
	}

	ret = kernfs_get_tree(fc);
	if (ret < 0)
		goto out_psl;

	if (resctrl_arch_alloc_capable())
		resctrl_arch_enable_alloc();
	if (resctrl_arch_mon_capable())
		resctrl_arch_enable_mon();

	if (resctrl_arch_alloc_capable() || resctrl_arch_mon_capable())
		resctrl_mounted = true;

	if (resctrl_is_mbm_enabled()) {
		list_for_each_entry(dom, &l3->domains, list)
			mbm_setup_overflow_handler(dom, MBM_OVERFLOW_INTERVAL, -1);
	}

	goto out;

out_psl:
	if (IS_ENABLED(RESCTRL_FS_PSEUDO_LOCK))
		rdt_pseudo_lock_release();
out_mondata:
	if (resctrl_arch_mon_capable())
		kernfs_remove(kn_mondata);
out_mongrp:
	if (resctrl_arch_mon_capable())
		kernfs_remove(kn_mongrp);
out_info:
	kernfs_remove(kn_info);
out_schemata_free:
	destroy_schemata_list();
out_mba:
	if (ctx->enable_mba_mbps)
		set_mba_sc(false);
out_cdp:
	cdp_disable_all();
out:
	rdt_last_cmd_clear();
	mutex_unlock(&rdtgroup_mutex);
	cpus_read_unlock();
	return ret;
}

enum rdt_param {
	Opt_cdp,
	Opt_cdpl2,
	Opt_mba_mbps,
	nr__rdt_params
};

static const struct fs_parameter_spec rdt_param_specs[] = {
	fsparam_flag("cdp",		Opt_cdp),
	fsparam_flag("cdpl2",		Opt_cdpl2),
	fsparam_flag("mba_MBps",	Opt_mba_mbps),
	{}
};

static const struct fs_parameter_description rdt_fs_parameters = {
	.name		= "rdt",
	.specs		= rdt_param_specs,
};

static int rdt_parse_param(struct fs_context *fc, struct fs_parameter *param)
{
	struct rdt_fs_context *ctx = rdt_fc2context(fc);
	struct fs_parse_result result;
	int opt;

	opt = fs_parse(fc, &rdt_fs_parameters, param, &result);
	if (opt < 0)
		return opt;

	switch (opt) {
	case Opt_cdp:
		ctx->enable_cdpl3 = true;
		return 0;
	case Opt_cdpl2:
		ctx->enable_cdpl2 = true;
		return 0;
	case Opt_mba_mbps:
		if (supports_mba_mbps())
			return -EINVAL;
		ctx->enable_mba_mbps = true;
		return 0;
	}

	return -EINVAL;
}

static void rdt_fs_context_free(struct fs_context *fc)
{
	struct rdt_fs_context *ctx = rdt_fc2context(fc);

	kernfs_free_fs_context(fc);
	kfree(ctx);
}

static const struct fs_context_operations rdt_fs_context_ops = {
	.free		= rdt_fs_context_free,
	.parse_param	= rdt_parse_param,
	.get_tree	= rdt_get_tree,
};

static int rdt_init_fs_context(struct fs_context *fc)
{
	struct rdt_fs_context *ctx;

	ctx = kzalloc(sizeof(struct rdt_fs_context), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->kfc.root = rdt_root;
	ctx->kfc.magic = RDTGROUP_SUPER_MAGIC;
	fc->fs_private = &ctx->kfc;
	fc->ops = &rdt_fs_context_ops;
	if (fc->user_ns)
		put_user_ns(fc->user_ns);
	fc->user_ns = get_user_ns(&init_user_ns);
	fc->global = true;
	return 0;
}

/*
 * Move tasks from one to the other group. If @from is NULL, then all tasks
 * in the systems are moved unconditionally (used for teardown).
 *
 * If @mask is not NULL the cpus on which moved tasks are running are set
 * in that mask so the update smp function call is restricted to affected
 * cpus.
 */
static void rdt_move_group_tasks(struct rdtgroup *from, struct rdtgroup *to,
				 struct cpumask *mask)
{
	struct task_struct *p, *t;

	read_lock(&tasklist_lock);
	for_each_process_thread(p, t) {
		if (!from || is_closid_match(t, from) ||
		    is_rmid_match(t, from)) {
			resctrl_set_closid(t, to->closid);
			resctrl_arch_set_rmid(t, to->mon.rmid);

#ifdef CONFIG_SMP
			/*
			 * This is safe on x86 w/o barriers as the ordering
			 * of writing to task_cpu() and t->on_cpu is
			 * reverse to the reading here. The detection is
			 * inaccurate as tasks might move or schedule
			 * before the smp function call takes place. In
			 * such a case the function call is pointless, but
			 * there is no other side effect.
			 */
			if (mask && t->on_cpu)
				cpumask_set_cpu(task_cpu(t), mask);
#endif
		}
	}
	read_unlock(&tasklist_lock);
}

static void free_all_child_rdtgrp(struct rdtgroup *rdtgrp)
{
	struct rdtgroup *sentry, *stmp;
	struct list_head *head;
	hw_closid_t hw_closid;

	head = &rdtgrp->mon.crdtgrp_list;
	list_for_each_entry_safe(sentry, stmp, head, mon.crdtgrp_list) {
		hw_closid = resctrl_closid_cdp_map(sentry->closid, CDP_BOTH);
		free_rmid(hw_closid, sentry->mon.rmid);
		list_del(&sentry->mon.crdtgrp_list);
		kfree(sentry);
	}
}

/*
 * Forcibly remove all of subdirectories under root.
 */
static void rmdir_all_sub(void)
{
	struct rdtgroup *rdtgrp, *tmp;
	hw_closid_t hw_closid;

	/* Move all tasks to the default resource group */
	rdt_move_group_tasks(NULL, &rdtgroup_default, NULL);

	list_for_each_entry_safe(rdtgrp, tmp, &rdt_all_groups, rdtgroup_list) {
		/* Free any child rmids */
		free_all_child_rdtgrp(rdtgrp);

		/* Remove each rdtgroup other than root */
		if (rdtgrp == &rdtgroup_default)
			continue;

		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP ||
		    rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED)
			rdtgroup_pseudo_lock_remove(rdtgrp);

		/*
		 * Give any CPUs back to the default group. We cannot copy
		 * cpu_online_mask because a CPU might have executed the
		 * offline callback already, but is still marked online.
		 */
		cpumask_or(&rdtgroup_default.cpu_mask,
			   &rdtgroup_default.cpu_mask, &rdtgrp->cpu_mask);

		hw_closid = resctrl_closid_cdp_map(rdtgrp->closid, CDP_BOTH);
		free_rmid(hw_closid, rdtgrp->mon.rmid);

		kernfs_remove(rdtgrp->kn);
		list_del(&rdtgrp->rdtgroup_list);
		kfree(rdtgrp);
	}
	/* Notify online CPUs to update per cpu storage and PQR_ASSOC MSR */
	update_closid_rmid(cpu_online_mask, &rdtgroup_default);

	kernfs_remove(kn_info);
	kernfs_remove(kn_mongrp);
	kernfs_remove(kn_mondata);
}

static void rdt_kill_sb(struct super_block *sb)
{
	cpus_read_lock();
	mutex_lock(&rdtgroup_mutex);

	set_mba_sc(false);

	/* Put everything back to default values. */
	resctrl_arch_reset_resources();

	cdp_disable_all();
	rmdir_all_sub();
	if (IS_ENABLED(CONFIG_RESCTRL_FS_PSEUDO_LOCK))
		rdt_pseudo_lock_release();
	rdtgroup_default.mode = RDT_MODE_SHAREABLE;
	destroy_schemata_list();
	resctrl_arch_disable_alloc();
	resctrl_arch_disable_mon();
	resctrl_mounted = false;
	kernfs_kill_sb(sb);
	mutex_unlock(&rdtgroup_mutex);
	cpus_read_unlock();
}

static struct file_system_type rdt_fs_type = {
	.name			= "resctrl",
	.init_fs_context	= rdt_init_fs_context,
	.parameters		= &rdt_fs_parameters,
	.kill_sb		= rdt_kill_sb,
};

static int mon_addfile(struct kernfs_node *parent_kn, const char *name,
		       void *priv)
{
	struct kernfs_node *kn;
	int ret = 0;

	kn = __kernfs_create_file(parent_kn, name, 0444,
				  GLOBAL_ROOT_UID, GLOBAL_ROOT_GID, 0,
				  &kf_mondata_ops, priv, NULL, NULL);
	if (IS_ERR(kn))
		return PTR_ERR(kn);

	ret = rdtgroup_kn_set_ugid(kn);
	if (ret) {
		kernfs_remove(kn);
		return ret;
	}

	return ret;
}

/*
 * Remove all subdirectories of mon_data of ctrl_mon groups
 * and monitor groups with given domain id.
 */
static void rmdir_mondata_subdir_allrdtgrp(struct rdt_resource *r,
					   unsigned int dom_id)
{
	struct rdtgroup *prgrp, *crgrp;
	char name[32];

	list_for_each_entry(prgrp, &rdt_all_groups, rdtgroup_list) {
		sprintf(name, "mon_%s_%02d", r->name, dom_id);
		kernfs_remove_by_name(prgrp->mon.mon_data_kn, name);

		list_for_each_entry(crgrp, &prgrp->mon.crdtgrp_list, mon.crdtgrp_list)
			kernfs_remove_by_name(crgrp->mon.mon_data_kn, name);
	}
}

static int mkdir_mondata_subdir(struct kernfs_node *parent_kn,
				struct rdt_domain *d,
				struct rdt_resource *r, struct rdtgroup *prgrp)
{
	union mon_data_bits priv;
	struct kernfs_node *kn;
	struct mon_evt *mevt;
	struct rmid_read rr;
	char name[32];
	int ret;

	sprintf(name, "mon_%s_%02d", r->name, d->id);
	/* create the directory */
	kn = kernfs_create_dir(parent_kn, name, parent_kn->mode, prgrp);
	if (IS_ERR(kn))
		return PTR_ERR(kn);

	/*
	 * This extra ref will be put in kernfs_remove() and guarantees
	 * that kn is always accessible.
	 */
	kernfs_get(kn);
	ret = rdtgroup_kn_set_ugid(kn);
	if (ret)
		goto out_destroy;

	if (WARN_ON(list_empty(&r->evt_list))) {
		ret = -EPERM;
		goto out_destroy;
	}

	priv.u.rid = r->rid;
	priv.u.domid = d->id;
	list_for_each_entry(mevt, &r->evt_list, list) {
		priv.u.evtid = mevt->evtid;
		ret = mon_addfile(kn, mevt->name, priv.priv);
		if (ret)
			goto out_destroy;

		if (resctrl_is_mbm_event(mevt->evtid))
			mon_event_read(&rr, d, prgrp, mevt->evtid, true);
	}
	kernfs_activate(kn);
	return 0;

out_destroy:
	kernfs_remove(kn);
	return ret;
}

/*
 * Add all subdirectories of mon_data for "ctrl_mon" groups
 * and "monitor" groups with given domain id.
 */
static void mkdir_mondata_subdir_allrdtgrp(struct rdt_resource *r,
					   struct rdt_domain *d)
{
	struct kernfs_node *parent_kn;
	struct rdtgroup *prgrp, *crgrp;
	struct list_head *head;

	list_for_each_entry(prgrp, &rdt_all_groups, rdtgroup_list) {
		parent_kn = prgrp->mon.mon_data_kn;
		mkdir_mondata_subdir(parent_kn, d, r, prgrp);

		head = &prgrp->mon.crdtgrp_list;
		list_for_each_entry(crgrp, head, mon.crdtgrp_list) {
			parent_kn = crgrp->mon.mon_data_kn;
			mkdir_mondata_subdir(parent_kn, d, r, crgrp);
		}
	}
}

static int mkdir_mondata_subdir_alldom(struct kernfs_node *parent_kn,
				       struct rdt_resource *r,
				       struct rdtgroup *prgrp)
{
	struct rdt_domain *dom;
	int ret;

	lockdep_assert_cpus_held();

	list_for_each_entry(dom, &r->domains, list) {
		ret = mkdir_mondata_subdir(parent_kn, dom, r, prgrp);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * This creates a directory mon_data which contains the monitored data.
 *
 * mon_data has one directory for each domain whic are named
 * in the format mon_<domain_name>_<domain_id>. For ex: A mon_data
 * with L3 domain looks as below:
 * ./mon_data:
 * mon_L3_00
 * mon_L3_01
 * mon_L3_02
 * ...
 *
 * Each domain directory has one file per event:
 * ./mon_L3_00/:
 * llc_occupancy
 *
 */
static int mkdir_mondata_all(struct kernfs_node *parent_kn,
			     struct rdtgroup *prgrp,
			     struct kernfs_node **dest_kn)
{
	enum resctrl_resource_level i;
	struct rdt_resource *r;
	struct kernfs_node *kn;
	int ret;

	/*
	 * Create the mon_data directory first.
	 */
	ret = mongroup_create_dir(parent_kn, NULL, "mon_data", &kn);
	if (ret)
		return ret;

	if (dest_kn)
		*dest_kn = kn;

	/*
	 * Create the subdirectories for each domain. Note that all events
	 * in a domain like L3 are grouped into a resource whose domain is L3
	 */
	for (i = 0; i < RDT_NUM_RESOURCES; i++) {
		r = resctrl_arch_get_resource(i);
		if (!r->mon_capable)
			continue;

		ret = mkdir_mondata_subdir_alldom(kn, r, prgrp);
		if (ret)
			goto out_destroy;
	}

	return 0;

out_destroy:
	kernfs_remove(kn);
	return ret;
}

/**
 * cbm_ensure_valid - Enforce validity on provided CBM
 * @_val:	Candidate CBM
 * @r:		RDT resource to which the CBM belongs
 *
 * The provided CBM represents all cache portions available for use. This
 * may be represented by a bitmap that does not consist of contiguous ones
 * and thus be an invalid CBM.
 * Here the provided CBM is forced to be a valid CBM by only considering
 * the first set of contiguous bits as valid and clearing all bits.
 * The intention here is to provide a valid default CBM with which a new
 * resource group is initialized. The user can follow this with a
 * modification to the CBM if the default does not satisfy the
 * requirements.
 */
static void cbm_ensure_valid(u32 *_val, struct rdt_resource *r)
{
	/*
	 * Convert the u32 _val to an unsigned long required by all the bit
	 * operations within this function. No more than 32 bits of this
	 * converted value can be accessed because all bit operations are
	 * additionally provided with cbm_len that is initialized during
	 * hardware enumeration using five bits from the EAX register and
	 * thus never can exceed 32 bits.
	 */
	unsigned long *val = (unsigned long *)_val;
	unsigned int cbm_len = r->cache.cbm_len;
	unsigned long first_bit, zero_bit;

	if (*val == 0)
		return;

	first_bit = find_first_bit(val, cbm_len);
	zero_bit = find_next_zero_bit(val, cbm_len, first_bit);

	/* Clear any remaining bits to ensure contiguous region */
	bitmap_clear(val, zero_bit, cbm_len - zero_bit);
}

/*
 * Initialize cache resources per RDT domain
 *
 * Set the RDT domain up to start off with all usable allocations. That is,
 * all shareable and unused bits. All-zero CBM is invalid.
 */
static int __init_one_rdt_domain(struct rdt_domain *d, struct resctrl_schema *s,
				 u32 closid)
{
	enum resctrl_conf_type t = s->conf_type;
	struct resctrl_staged_config *cfg;
	struct rdt_resource *r = s->res;
	u32 used_b = 0, unused_b = 0;
	unsigned long tmp_cbm;
	enum rdtgrp_mode mode;
	u32 ctrl_val;
	int i;

	cfg = &d->staged_config[t];
	cfg->have_new_ctrl = false;
	cfg->new_ctrl = r->cache.shareable_bits;
	used_b = r->cache.shareable_bits;
	for (i = 0; i < closids_supported(); i++) {
		if (closid_allocated(i) && i != closid) {
			mode = rdtgroup_mode_by_closid(i);
			if (mode == RDT_MODE_PSEUDO_LOCKSETUP) {
				/*
				 * ctrl values for locksetup aren't relevant
				 * until the schemata is written, and the mode
				 * becomes RDT_MODE_PSEUDO_LOCKED.
				 */
				continue;
			}

			/*
			 * If CDP is active include peer domain's
			 * usage to ensure there is no overlap
			 * with an exclusive group.
			 */
			ctrl_val = resctrl_get_config_cdp(i, s, d);
			used_b |= ctrl_val;
			if (mode == RDT_MODE_SHAREABLE)
				cfg->new_ctrl |= ctrl_val;
		}
	}
	if (d->plr && d->plr->cbm > 0)
		used_b |= d->plr->cbm;
	unused_b = used_b ^ (BIT_MASK(r->cache.cbm_len) - 1);
	unused_b &= BIT_MASK(r->cache.cbm_len) - 1;
	cfg->new_ctrl |= unused_b;
	/*
	 * Force the initial CBM to be valid, user can
	 * modify the CBM based on system availability.
	 */
	cbm_ensure_valid(&cfg->new_ctrl, r);
	/*
	 * Assign the u32 CBM to an unsigned long to ensure that
	 * bitmap_weight() does not access out-of-bound memory.
	 */
	tmp_cbm = cfg->new_ctrl;
	if (bitmap_weight(&tmp_cbm, r->cache.cbm_len) < r->cache.min_cbm_bits) {
		rdt_last_cmd_printf("No space on %s:%d\n", r->name, d->id);
		return -ENOSPC;
	}
	cfg->hw_closid = resctrl_closid_cdp_map(closid, t);
	cfg->have_new_ctrl = true;

	return 0;
}

/*
 * Initialize cache resources with default values.
 *
 * A new RDT group is being created on an allocation capable (CAT)
 * supporting system. Set this group up to start off with all usable
 * allocations.
 *
 * If there are no more shareable bits available on any domain then
 * the entire allocation will fail.
 */
static int rdtgroup_init_cat(struct resctrl_schema *s, u32 closid)
{
	struct rdt_domain *d;
	int ret;

	list_for_each_entry(d, &s->res->domains, list) {
		ret = __init_one_rdt_domain(d, s, closid);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/* Initialize MBA resource with default values. */
static void rdtgroup_init_mba(struct rdt_resource *r, u32 closid)
{
	struct resctrl_staged_config *cfg;
	struct rdt_domain *d;

	list_for_each_entry(d, &r->domains, list) {
		cfg = &d->staged_config[CDP_BOTH];
		cfg->new_ctrl = is_mba_sc(r) ? MBA_MAX_MBPS : r->default_ctrl;
		cfg->hw_closid = resctrl_closid_cdp_map(closid, CDP_BOTH);
		cfg->have_new_ctrl = true;
	}
}

/* Initialize the RDT group's allocations. */
static int rdtgroup_init_alloc(struct rdtgroup *rdtgrp)
{
	struct resctrl_schema *s;
	struct rdt_resource *r;
	int ret;

	lockdep_assert_cpus_held();
	lockdep_assert_held(&rdtgroup_mutex);

	list_for_each_entry(s, &resctrl_all_schema, list) {
		r = s->res;
		if (r->rid == RDT_RESOURCE_MBA) {
			rdtgroup_init_mba(r, rdtgrp->closid);
		} else {
			ret = rdtgroup_init_cat(s, rdtgrp->closid);
			if (ret < 0)
				return ret;
		}

		ret = resctrl_arch_update_domains(r);
		if (ret < 0) {
			rdt_last_cmd_puts("Failed to initialize allocations\n");
			return ret;
		}

	}

	rdtgrp->mode = RDT_MODE_SHAREABLE;

	return 0;
}

static int mkdir_rdt_prepare(struct kernfs_node *parent_kn,
			     struct kernfs_node *prgrp_kn,
			     const char *name, umode_t mode,
			     enum rdt_group_type rtype, struct rdtgroup **r)
{
	struct rdtgroup *prdtgrp, *rdtgrp;
	struct kernfs_node *kn;
	hw_closid_t hw_closid;
	uint files = 0;
	int ret;

	prdtgrp = rdtgroup_kn_lock_live(prgrp_kn);
	rdt_last_cmd_clear();
	if (!prdtgrp) {
		ret = -ENODEV;
		rdt_last_cmd_puts("Directory was removed\n");
		goto out_unlock;
	}

	if (rtype == RDTMON_GROUP &&
	    (prdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP ||
	     prdtgrp->mode == RDT_MODE_PSEUDO_LOCKED)) {
		ret = -EINVAL;
		rdt_last_cmd_puts("Pseudo-locking in progress\n");
		goto out_unlock;
	}

	/* allocate the rdtgroup. */
	rdtgrp = kzalloc(sizeof(*rdtgrp), GFP_KERNEL);
	if (!rdtgrp) {
		ret = -ENOSPC;
		rdt_last_cmd_puts("Kernel out of memory\n");
		goto out_unlock;
	}
	*r = rdtgrp;
	rdtgrp->mon.parent = prdtgrp;
	rdtgrp->type = rtype;

	/* allocate the closid. */
	if (rtype == RDTCTRL_GROUP) {
		ret = closid_alloc();
		if (ret < 0) {
			rdt_last_cmd_puts("Out of CLOSIDs\n");
			goto out_free_rgrp;
		}
		rdtgrp->closid = ret;
	} else {
		rdtgrp->closid = prdtgrp->closid;
	}
	INIT_LIST_HEAD(&rdtgrp->mon.crdtgrp_list);

	/* kernfs creates the directory for rdtgrp */
	kn = kernfs_create_dir(parent_kn, name, mode, rdtgrp);
	if (IS_ERR(kn)) {
		ret = PTR_ERR(kn);
		rdt_last_cmd_puts("kernfs create error\n");
		goto out_closid_free;
	}
	rdtgrp->kn = kn;

	/*
	 * kernfs_remove() will drop the reference count on "kn" which
	 * will free it. But we still need it to stick around for the
	 * rdtgroup_kn_unlock(kn} call below. Take one extra reference
	 * here, which will be dropped inside rdtgroup_kn_unlock().
	 */
	kernfs_get(kn);

	ret = rdtgroup_kn_set_ugid(kn);
	if (ret) {
		rdt_last_cmd_puts("kernfs perm error\n");
		goto out_destroy;
	}

	files = RFTYPE_BASE | BIT(RF_CTRLSHIFT + rtype);
	ret = rdtgroup_add_files(kn, files);
	if (ret) {
		rdt_last_cmd_puts("kernfs fill error\n");
		goto out_destroy;
	}

	if (resctrl_arch_mon_capable()) {
		hw_closid = resctrl_closid_cdp_map(rdtgrp->closid, CDP_BOTH);
		ret = alloc_rmid(hw_closid);
		if (ret < 0) {
			rdt_last_cmd_puts("Out of RMIDs\n");
			goto out_destroy;
		}
		rdtgrp->mon.rmid = ret;

		ret = mkdir_mondata_all(kn, rdtgrp, &rdtgrp->mon.mon_data_kn);
		if (ret) {
			rdt_last_cmd_puts("kernfs subdir error\n");
			goto out_idfree;
		}
	}
	kernfs_activate(kn);

	/*
	 * The caller unlocks the prgrp_kn upon success.
	 */
	return 0;

out_idfree:
	free_rmid(hw_closid, rdtgrp->mon.rmid);
out_destroy:
	kernfs_remove(rdtgrp->kn);
out_closid_free:
	if (rtype == RDTCTRL_GROUP)
		closid_free(rdtgrp->closid);
out_free_rgrp:
	kfree(rdtgrp);
out_unlock:
	rdtgroup_kn_unlock(prgrp_kn);
	return ret;
}

static void mkdir_rdt_prepare_clean(struct rdtgroup *rgrp)
{
	hw_closid_t hw_closid = resctrl_closid_cdp_map(rgrp->closid, CDP_BOTH);

	kernfs_remove(rgrp->kn);
	free_rmid(hw_closid, rgrp->mon.rmid);
	kfree(rgrp);
}

/*
 * Create a monitor group under "mon_groups" directory of a control
 * and monitor group(ctrl_mon). This is a resource group
 * to monitor a subset of tasks and cpus in its parent ctrl_mon group.
 */
static int rdtgroup_mkdir_mon(struct kernfs_node *parent_kn,
			      struct kernfs_node *prgrp_kn,
			      const char *name,
			      umode_t mode)
{
	struct rdtgroup *rdtgrp, *prgrp;
	int ret;

	ret = mkdir_rdt_prepare(parent_kn, prgrp_kn, name, mode, RDTMON_GROUP,
				&rdtgrp);
	if (ret)
		return ret;

	prgrp = rdtgrp->mon.parent;

	/*
	 * Add the rdtgrp to the list of rdtgrps the parent
	 * ctrl_mon group has to track.
	 */
	list_add_tail(&rdtgrp->mon.crdtgrp_list, &prgrp->mon.crdtgrp_list);

	rdtgroup_kn_unlock(prgrp_kn);
	return ret;
}

/*
 * These are rdtgroups created under the root directory. Can be used
 * to allocate and monitor resources.
 */
static int rdtgroup_mkdir_ctrl_mon(struct kernfs_node *parent_kn,
				   struct kernfs_node *prgrp_kn,
				   const char *name, umode_t mode)
{
	struct rdtgroup *rdtgrp;
	struct kernfs_node *kn;
	int ret;

	ret = mkdir_rdt_prepare(parent_kn, prgrp_kn, name, mode, RDTCTRL_GROUP,
				&rdtgrp);
	if (ret)
		return ret;

	kn = rdtgrp->kn;
	ret = rdtgroup_init_alloc(rdtgrp);
	if (ret < 0)
		goto out_common_fail;

	list_add(&rdtgrp->rdtgroup_list, &rdt_all_groups);

	if (resctrl_arch_mon_capable()) {
		/*
		 * Create an empty mon_groups directory to hold the subset
		 * of tasks and cpus to monitor.
		 */
		ret = mongroup_create_dir(kn, NULL, "mon_groups", NULL);
		if (ret) {
			rdt_last_cmd_puts("kernfs subdir error\n");
			goto out_del_list;
		}
	}

	goto out_unlock;

out_del_list:
	list_del(&rdtgrp->rdtgroup_list);
out_common_fail:
	mkdir_rdt_prepare_clean(rdtgrp);
out_unlock:
	rdtgroup_kn_unlock(prgrp_kn);
	return ret;
}

/*
 * We allow creating mon groups only with in a directory called "mon_groups"
 * which is present in every ctrl_mon group. Check if this is a valid
 * "mon_groups" directory.
 *
 * 1. The directory should be named "mon_groups".
 * 2. The mon group itself should "not" be named "mon_groups".
 *   This makes sure "mon_groups" directory always has a ctrl_mon group
 *   as parent.
 */
static bool is_mon_groups(struct kernfs_node *kn, const char *name)
{
	return (!strcmp(kn->name, "mon_groups") &&
		strcmp(name, "mon_groups"));
}

static int rdtgroup_mkdir(struct kernfs_node *parent_kn, const char *name,
			  umode_t mode)
{
	/* Do not accept '\n' to avoid unparsable situation. */
	if (strchr(name, '\n'))
		return -EINVAL;

	/*
	 * If the parent directory is the root directory and RDT
	 * allocation is supported, add a control and monitoring
	 * subdirectory
	 */
	if (resctrl_arch_alloc_capable() && parent_kn == rdtgroup_default.kn)
		return rdtgroup_mkdir_ctrl_mon(parent_kn, parent_kn, name, mode);

	/*
	 * If RDT monitoring is supported and the parent directory is a valid
	 * "mon_groups" directory, add a monitoring subdirectory.
	 */
	if (resctrl_arch_mon_capable() && is_mon_groups(parent_kn, name))
		return rdtgroup_mkdir_mon(parent_kn, parent_kn->parent, name, mode);

	return -EPERM;
}

static int rdtgroup_rmdir_mon(struct kernfs_node *kn, struct rdtgroup *rdtgrp,
			      cpumask_var_t tmpmask)
{
	struct rdtgroup *prdtgrp = rdtgrp->mon.parent;
	hw_closid_t hw_closid;
	int cpu;

	/* Give any tasks back to the parent group */
	rdt_move_group_tasks(rdtgrp, prdtgrp, tmpmask);

	/* Update per cpu rmid of the moved CPUs first */
	for_each_cpu(cpu, &rdtgrp->cpu_mask)
		resctrl_arch_set_cpu_default_rmid(cpu, prdtgrp->mon.rmid);

	/*
	 * Update the MSR on moved CPUs and CPUs which have moved
	 * task running on them.
	 */
	cpumask_or(tmpmask, tmpmask, &rdtgrp->cpu_mask);
	update_closid_rmid(tmpmask, NULL);

	rdtgrp->flags = RDT_DELETED;
	hw_closid = resctrl_closid_cdp_map(rdtgrp->closid, CDP_BOTH);
	free_rmid(hw_closid, rdtgrp->mon.rmid);

	/*
	 * Remove the rdtgrp from the parent ctrl_mon group's list
	 */
	WARN_ON(list_empty(&prdtgrp->mon.crdtgrp_list));
	list_del(&rdtgrp->mon.crdtgrp_list);

	/*
	 * one extra hold on this, will drop when we kfree(rdtgrp)
	 * in rdtgroup_kn_unlock()
	 */
	kernfs_get(kn);
	kernfs_remove(rdtgrp->kn);

	return 0;
}

static int rdtgroup_ctrl_remove(struct kernfs_node *kn,
				struct rdtgroup *rdtgrp)
{
	rdtgrp->flags = RDT_DELETED;
	list_del(&rdtgrp->rdtgroup_list);

	/*
	 * one extra hold on this, will drop when we kfree(rdtgrp)
	 * in rdtgroup_kn_unlock()
	 */
	kernfs_get(kn);
	kernfs_remove(rdtgrp->kn);
	return 0;
}

static int rdtgroup_rmdir_ctrl(struct kernfs_node *kn, struct rdtgroup *rdtgrp,
			       cpumask_var_t tmpmask)
{
	int cpu;
	hw_closid_t hw_closid;

	/* Give any tasks back to the default group */
	rdt_move_group_tasks(rdtgrp, &rdtgroup_default, tmpmask);

	/* Give any CPUs back to the default group */
	cpumask_or(&rdtgroup_default.cpu_mask,
		   &rdtgroup_default.cpu_mask, &rdtgrp->cpu_mask);

	/* Update per cpu closid and rmid of the moved CPUs first */
	for_each_cpu(cpu, &rdtgrp->cpu_mask) {
		resctrl_set_cpu_default_closid(cpu, rdtgroup_default.closid);
		resctrl_arch_set_cpu_default_rmid(cpu, rdtgroup_default.mon.rmid);
	}

	/*
	 * Update the MSR on moved CPUs and CPUs which have moved
	 * task running on them.
	 */
	cpumask_or(tmpmask, tmpmask, &rdtgrp->cpu_mask);
	update_closid_rmid(tmpmask, NULL);

	hw_closid = resctrl_closid_cdp_map(rdtgrp->closid, CDP_BOTH);
	free_rmid(hw_closid, rdtgrp->mon.rmid);
	closid_free(rdtgrp->closid);

	/*
	 * Free all the child monitor group rmids.
	 */
	free_all_child_rdtgrp(rdtgrp);

	rdtgroup_ctrl_remove(kn, rdtgrp);

	return 0;
}

static int rdtgroup_rmdir(struct kernfs_node *kn)
{
	struct kernfs_node *parent_kn = kn->parent;
	struct rdtgroup *rdtgrp;
	cpumask_var_t tmpmask;
	int ret = 0;

	if (!zalloc_cpumask_var(&tmpmask, GFP_KERNEL))
		return -ENOMEM;

	rdtgrp = rdtgroup_kn_lock_live(kn);
	if (!rdtgrp) {
		ret = -EPERM;
		goto out;
	}

	/*
	 * If the rdtgroup is a ctrl_mon group and parent directory
	 * is the root directory, remove the ctrl_mon group.
	 *
	 * If the rdtgroup is a mon group and parent directory
	 * is a valid "mon_groups" directory, remove the mon group.
	 */
	if (rdtgrp->type == RDTCTRL_GROUP && parent_kn == rdtgroup_default.kn) {
		if (rdtgrp->mode == RDT_MODE_PSEUDO_LOCKSETUP ||
		    rdtgrp->mode == RDT_MODE_PSEUDO_LOCKED) {
			ret = rdtgroup_ctrl_remove(kn, rdtgrp);
		} else {
			ret = rdtgroup_rmdir_ctrl(kn, rdtgrp, tmpmask);
		}
	} else if (rdtgrp->type == RDTMON_GROUP &&
		 is_mon_groups(parent_kn, kn->name)) {
		ret = rdtgroup_rmdir_mon(kn, rdtgrp, tmpmask);
	} else {
		ret = -EPERM;
	}

out:
	rdtgroup_kn_unlock(kn);
	free_cpumask_var(tmpmask);
	return ret;
}

static int rdtgroup_show_options(struct seq_file *seq, struct kernfs_root *kf)
{
	struct rdt_resource *l2 = resctrl_arch_get_resource(RDT_RESOURCE_L2);
	struct rdt_resource *l3 = resctrl_arch_get_resource(RDT_RESOURCE_L3);

	if (l3->cdp_enabled)
		seq_puts(seq, ",cdp");

	if (l2->cdp_enabled)
		seq_puts(seq, ",cdpl2");

	if (is_mba_sc(NULL))
		seq_puts(seq, ",mba_MBps");

	return 0;
}

static struct kernfs_syscall_ops rdtgroup_kf_syscall_ops = {
	.mkdir		= rdtgroup_mkdir,
	.rmdir		= rdtgroup_rmdir,
	.show_options	= rdtgroup_show_options,
};

static int rdtgroup_setup_root(void)
{
	int ret;

	rdt_root = kernfs_create_root(&rdtgroup_kf_syscall_ops,
				      KERNFS_ROOT_CREATE_DEACTIVATED |
				      KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK,
				      &rdtgroup_default);
	if (IS_ERR(rdt_root))
		return PTR_ERR(rdt_root);

	mutex_lock(&rdtgroup_mutex);

	rdtgroup_default.closid = 0;
	rdtgroup_default.mon.rmid = 0;
	rdtgroup_default.type = RDTCTRL_GROUP;
	INIT_LIST_HEAD(&rdtgroup_default.mon.crdtgrp_list);

	list_add(&rdtgroup_default.rdtgroup_list, &rdt_all_groups);

	ret = rdtgroup_add_files(rdt_root->kn, RF_CTRL_BASE);
	if (ret) {
		kernfs_destroy_root(rdt_root);
		goto out;
	}

	rdtgroup_default.kn = rdt_root->kn;
	kernfs_activate(rdtgroup_default.kn);

out:
	mutex_unlock(&rdtgroup_mutex);

	return ret;
}


/* call with cpus_read_lock() held */
struct rdt_domain * __weak resctrl_arch_find_domain(struct rdt_resource *r, int id)
{
	struct rdt_domain *d;

	if (!r)
		return NULL;

	list_for_each_entry(d, &r->domains, list) {
		if (d->id == id)
			return d;
	}

	return NULL;
}

/*
 * Allocate any resource specific monitor arrays.
 * Called from cpuhp callbacks before resctrl_online_domain().
 */
static int resctrl_domain_setup_mon_state(struct rdt_resource *r,
					  struct rdt_domain *d)
{
	size_t tsize;
	u32 rmid_limit = resctrl_arch_num_rmid_idx();

	if (resctrl_arch_is_llc_occupancy_enabled()) {
		d->rmid_busy_llc = bitmap_zalloc(rmid_limit, GFP_KERNEL);
		if (!d->rmid_busy_llc)
			return -ENOMEM;
	}
	if (resctrl_arch_is_mbm_total_enabled()) {
		tsize = sizeof(*d->mbm_total);
		d->mbm_total = kcalloc(rmid_limit, tsize, GFP_KERNEL);
		if (!d->mbm_total) {
			bitmap_free(d->rmid_busy_llc);
			return -ENOMEM;
		}
	}
	if (resctrl_arch_is_mbm_local_enabled()) {
		tsize = sizeof(*d->mbm_local);
		d->mbm_local = kcalloc(rmid_limit, tsize, GFP_KERNEL);
		if (!d->mbm_local) {
			bitmap_free(d->rmid_busy_llc);
			kfree(d->mbm_total);
			return -ENOMEM;
		}
	}

	return 0;
}

static void resctrl_domain_teardown_mon_state(struct rdt_domain *d)
{
	kfree(d->rmid_busy_llc);
	kfree(d->mbm_total);
	kfree(d->mbm_local);
}

void resctrl_offline_domain(struct rdt_resource *r, struct rdt_domain *d)
{
	if (!r->mon_capable)
		return;

	mutex_lock(&rdtgroup_mutex);
	/*
	 * If resctrl is mounted, remove all the
	 * per domain monitor data directories.
	 */
	if (resctrl_mounted)
		rmdir_mondata_subdir_allrdtgrp(r, d->id);

	if (resctrl_is_mbm_enabled())
		cancel_delayed_work(&d->mbm_over);
	if (resctrl_arch_is_llc_occupancy_enabled() &&  has_busy_rmid(r, d)) {
		/*
		 * When a package is going down, forcefully
		 * decrement rmid->ebusy. There is no way to know
		 * that the L3 was flushed and hence may lead to
		 * incorrect counts in rare scenarios, but leaving
		 * the RMID as busy creates RMID leaks if the
		 * package never comes back.
		 */
		__check_limbo(d, true);
		cancel_delayed_work(&d->cqm_limbo);
	}

	if (resctrl_mounted && is_mba_sc(r))
		destroy_domain_mba_sc(r, d);

	resctrl_domain_teardown_mon_state(d);

	mutex_unlock(&rdtgroup_mutex);
}

int resctrl_online_domain(struct rdt_resource *r, struct rdt_domain *d)
{
	int err;

	if (!r->mon_capable)
		return 0;

	mutex_lock(&rdtgroup_mutex);

	err = resctrl_domain_setup_mon_state(r, d);
	if (err) {
		resctrl_domain_teardown_mon_state(d);
		return err;
	}

	if (resctrl_is_mbm_enabled()) {
		INIT_DELAYED_WORK(&d->mbm_over, mbm_handle_overflow);
		mbm_setup_overflow_handler(d, MBM_OVERFLOW_INTERVAL, -1);
	}

	if (resctrl_arch_is_llc_occupancy_enabled())
		INIT_DELAYED_WORK(&d->cqm_limbo, cqm_handle_limbo);

	/* If resctrl is mounted, add per domain monitor data directories. */
	if (resctrl_mounted)
		mkdir_mondata_subdir_allrdtgrp(r, d);

	if (resctrl_mounted && is_mba_sc(r))
		allocate_domain_mba_sc(smp_processor_id(), r, d);

	mutex_unlock(&rdtgroup_mutex);

	return 0;
}

int resctrl_online_cpu(unsigned int cpu)
{
	mutex_lock(&rdtgroup_mutex);
	/* The cpu is set in default rdtgroup after online. */
	cpumask_set_cpu(cpu, &rdtgroup_default.cpu_mask);
	mutex_unlock(&rdtgroup_mutex);

	return 0;
}

static void clear_childcpus(struct rdtgroup *r, unsigned int cpu)
{
	struct rdtgroup *cr;

	list_for_each_entry(cr, &r->mon.crdtgrp_list, mon.crdtgrp_list) {
		if (cpumask_test_and_clear_cpu(cpu, &cr->cpu_mask)) {
			break;
		}
	}
}

void resctrl_offline_cpu(unsigned int cpu)
{
	struct rdt_domain *d;
	struct rdtgroup *rdtgrp;
	struct rdt_resource *l3 = resctrl_arch_get_resource(RDT_RESOURCE_L3);

	mutex_lock(&rdtgroup_mutex);
	list_for_each_entry(rdtgrp, &rdt_all_groups, rdtgroup_list) {
		if (cpumask_test_and_clear_cpu(cpu, &rdtgrp->cpu_mask)) {
			clear_childcpus(rdtgrp, cpu);
			break;
		}
	}

	d = resctrl_get_domain_from_cpu(cpu, l3);
	if (d) {
		if (resctrl_is_mbm_enabled() && cpu == d->mbm_work_cpu) {
			cancel_delayed_work(&d->mbm_over);
			mbm_setup_overflow_handler(d, 0, cpu);
		}
		if (resctrl_arch_is_llc_occupancy_enabled() &&
		    cpu == d->cqm_work_cpu && has_busy_rmid(l3, d)) {
			cancel_delayed_work(&d->cqm_limbo);
			cqm_setup_limbo_handler(d, 0, cpu);
		}
	}
	mutex_unlock(&rdtgroup_mutex);
}

/*
 * resctrl_init - resctrl filesystem initialization
 *
 * Setup resctrl file system including set up root, create mount point,
 * register resctrl filesystem, and initialize files under root directory.
 *
 * Return: 0 on success or -errno
 */
int resctrl_init(void)
{
	int ret = 0;

	seq_buf_init(&last_cmd_status, last_cmd_status_buf,
		     sizeof(last_cmd_status_buf));

	ret = resctrl_mon_resource_init();
	if (ret)
		return ret;

	ret = rdtgroup_setup_root();
	if (ret)
		return ret;

	ret = sysfs_create_mount_point(fs_kobj, "resctrl");
	if (ret)
		goto cleanup_root;

	ret = register_filesystem(&rdt_fs_type);
	if (ret)
		goto cleanup_mountpoint;

	/*
	 * Adding the resctrl debugfs directory here may not be ideal since
	 * it would let the resctrl debugfs directory appear on the debugfs
	 * filesystem before the resctrl filesystem is mounted.
	 * It may also be ok since that would enable debugging of RDT before
	 * resctrl is mounted.
	 * The reason why the debugfs directory is created here and not in
	 * rdt_get_tree() is because rdt_get_tree() takes rdtgroup_mutex and
	 * during the debugfs directory creation also &sb->s_type->i_mutex_key
	 * (the lockdep class of inode->i_rwsem). Other filesystem
	 * interactions (eg. SyS_getdents) have the lock ordering:
	 * &sb->s_type->i_mutex_key --> &mm->mmap_sem
	 * During mmap(), called with &mm->mmap_sem, the rdtgroup_mutex
	 * is taken, thus creating dependency:
	 * &mm->mmap_sem --> rdtgroup_mutex for the latter that can cause
	 * issues considering the other two lock dependencies.
	 * By creating the debugfs directory here we avoid a dependency
	 * that may cause deadlock (even though file operations cannot
	 * occur until the filesystem is mounted, but I do not know how to
	 * tell lockdep that).
	 */
	debugfs_resctrl = debugfs_create_dir("resctrl", NULL);

	return 0;

cleanup_mountpoint:
	sysfs_remove_mount_point(fs_kobj, "resctrl");
cleanup_root:
	kernfs_destroy_root(rdt_root);

	return ret;
}

void resctrl_exit(void)
{
	debugfs_remove_recursive(debugfs_resctrl);
	unregister_filesystem(&rdt_fs_type);
	sysfs_remove_mount_point(fs_kobj, "resctrl");
	kernfs_destroy_root(rdt_root);
}
