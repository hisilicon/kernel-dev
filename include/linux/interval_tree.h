/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_INTERVAL_TREE_H
#define _LINUX_INTERVAL_TREE_H

#include <linux/rbtree.h>

struct interval_tree_node {
	struct rb_node rb;
	unsigned long start;	/* Start of interval */
	unsigned long last;	/* Last location _in_ interval */
	unsigned long __subtree_last;
};

extern void
interval_tree_insert(struct interval_tree_node *node,
		     struct rb_root_cached *root);

extern void
interval_tree_remove(struct interval_tree_node *node,
		     struct rb_root_cached *root);

extern struct interval_tree_node *
interval_tree_iter_first(struct rb_root_cached *root,
			 unsigned long start, unsigned long last);

extern struct interval_tree_node *
interval_tree_iter_next(struct interval_tree_node *node,
			unsigned long start, unsigned long last);

/*
 * This iterator travels over spans in an interval tree. It does not return
 * nodes but classifies each span as either a hole, where no nodes intersect, or
 * a used, which is fully covered by nodes. Each iteration step toggles between
 * hole and used until the entire range is covered. The returned spans always
 * fully cover the requested range.
 *
 * The iterator is greedy, it always returns the largest hole or used possible,
 * consolidating all consecutive nodes.
 *
 * Only is_hole, start_hole/used and last_hole/used are part of the external
 * interface.
 */
struct interval_tree_span_iter {
	struct interval_tree_node *nodes[2];
	unsigned long first_index;
	unsigned long last_index;
	union {
		unsigned long start_hole;
		unsigned long start_used;
	};
	union {
		unsigned long last_hole;
		unsigned long last_used;
	};
	/* 0 == used, 1 == is_hole, -1 == done iteration */
	int is_hole;
};

void interval_tree_span_iter_first(struct interval_tree_span_iter *state,
				   struct rb_root_cached *itree,
				   unsigned long first_index,
				   unsigned long last_index);
void interval_tree_span_iter_next(struct interval_tree_span_iter *state);

static inline bool
interval_tree_span_iter_done(struct interval_tree_span_iter *state)
{
	return state->is_hole == -1;
}

#endif	/* _LINUX_INTERVAL_TREE_H */
