// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2016 Facebook
 * Copyright (C) 2013-2014 Jens Axboe
 */

#include <linux/sched.h>
#include <linux/random.h>
#include <linux/sbitmap.h>
#include <linux/seq_file.h>
#include <linux/mm.h>

static int init_alloc_hint(struct sbitmap *sb, gfp_t flags)
{
	unsigned depth = sb->depth;

	sb->alloc_hint = alloc_percpu_gfp(unsigned int, flags);
	if (!sb->alloc_hint)
		return -ENOMEM;

	if (sb->numa_aware) {
		unsigned int depth_per_node = sb->depth_per_node;
		if (depth && !sb->round_robin) {
			int i;
			
			for_each_possible_cpu(i) {
				int nid = cpu_to_node(i);
				unsigned int base = nid * depth_per_node;
				unsigned int hint = base + (prandom_u32() % depth_per_node);

				if (((i % 5) == 0) || (i < 4))
					pr_err("%s numa_aware cpu%d hint=%d depth_per_node=%d\n", __func__, i, hint, depth_per_node);

				*per_cpu_ptr(sb->alloc_hint, i) = hint;

				sbitmap_check_hint(sb, i, hint);
			}
		}
	} else {
		if (depth && !sb->round_robin) {
			int i;

			for_each_possible_cpu(i)
				*per_cpu_ptr(sb->alloc_hint, i) = prandom_u32() % depth;
		}
	}
	return 0;
}

void sbitmap_deferred_clear_bit(struct sbitmap *sb, const unsigned int bitnr)
{
	unsigned long *addr;
	
	if (sb->numa_aware) {
		struct sbitmap_word *map;

		// debug part
		if (0) {
			unsigned int shift = sb->shift;
			unsigned int depth_per_node_shift = sb->depth_per_node_shift;
			int normal_shift = ilog2(BITS_PER_LONG);
			unsigned int nid_debug = bitnr >> depth_per_node_shift;
			unsigned int __bitnr_debug = bitnr;
			unsigned int depth_per_node_mask = (1 << sb->depth_per_node_shift) - 1;
			unsigned int bindex;
			unsigned int bindex_debug;
			unsigned int bindex_debug2;
			unsigned int bindex_debug_mask = (1 << (sb->depth_per_node_shift - sb->shift)) - 1;
			unsigned int __bitnr = bitnr;


			unsigned int depth_per_node = sb->depth_per_node;
			unsigned int nid = bitnr / depth_per_node;
			unsigned int index;
			//bool test;
			
			__bitnr -= (nid * depth_per_node);
			index = SB_NR_TO_INDEX(sb, __bitnr);
			map = sb->numa_map[nid];
			map += index;

			__bitnr_debug &= depth_per_node_mask;

			if (nid != nid_debug)
				pr_err_once("%s error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask);

			if (__bitnr_debug != __bitnr)
				pr_err_once("%s2 error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug);

			bindex = SB_NR_TO_INDEX(sb, __bitnr);
			bindex_debug = (bitnr & depth_per_node_mask) >> shift;
			bindex_debug2 = __bitnr_debug >> shift;

			if (bindex != bindex_debug)
				pr_err_once("%s3 error bindex=%d bindex_debug=%d nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex_debug_mask=0x%x index=%d shift=%d\n",
					__func__, bindex, bindex_debug, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex_debug_mask, index, shift);

			if (bindex != bindex_debug2)
				pr_err_once("%s4 error bindex=%d bindex_debug=%d bindex_debug2=%d nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex_debug_mask=0x%x index=%d shift=%d\n",
					__func__, bindex, bindex_debug, bindex_debug2, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex_debug_mask, index, shift);


			if (__bitnr_debug != __bitnr)
				pr_err_once("%s5 error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex=%d bindex_debug=%d\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex, bindex_debug);

		
			addr = &map->cleared;
	//		test = test_bit(SB_NR_TO_BIT(sb, __bitnr), addr);
	//		WARN_ONCE(test, "%s bitnr=%d index=%d nid=%d __bitnr=%d depth_per_node=%d\n", __func__, bitnr, index, nid, __bitnr, depth_per_node);

			set_bit(SB_NR_TO_BIT(sb, __bitnr), addr);				
		}else {
			const unsigned int depth_per_node_shift = sb->depth_per_node_shift;
			const unsigned int depth_per_node_mask = (1 << sb->depth_per_node_shift) - 1;
			unsigned int __bitnr = bitnr & depth_per_node_mask;
			const unsigned int nid = bitnr >> depth_per_node_shift;
			const unsigned int shift = sb->shift;
			unsigned int index = __bitnr >> shift;
		//	bool test;
		//	unsigned int __bitnr_test = __bitnr & (depth_per_node_mask >> (depth_per_node_shift - shift));

			map = sb->numa_map[nid];
			map += index;
			
		
			addr = &map->cleared;
		//	test = test_bit(SB_NR_TO_BIT(sb, __bitnr), addr);
			//WARN_ONCE(test, "%s bitnr=%d index=%d nid=%d __bitnr=%d depth_per_node=%d\n", __func__, bitnr, index, nid, __bitnr, sb->depth_per_node_shift);
		//	WARN_ONCE(__bitnr_test != SB_NR_TO_BIT(sb, __bitnr), "%s bitnr=%d __bitnr=%d __bitnr_test=%d SB_NR_TO_BIT=%d\n", __func__, bitnr, __bitnr, __bitnr_test, SB_NR_TO_BIT(sb, __bitnr));
			set_bit(SB_NR_TO_BIT(sb, __bitnr), addr);
		}

	} else {
		addr = &sb->map[SB_NR_TO_INDEX(sb, bitnr)].cleared;
		set_bit(SB_NR_TO_BIT(sb, bitnr), addr);
	}
}


static void sbitmap_deferred_clear_bit_debug(struct sbitmap *sb, const unsigned int bitnr)
{
//	unsigned long *addr;
	
	if (sb->numa_aware) {
		struct sbitmap_word *map;
		unsigned int depth_per_node = sb->depth_per_node;
		unsigned int nid = bitnr / depth_per_node;
		unsigned int index;
		unsigned int __bitnr = bitnr;
//		bool test;

		__bitnr -= (nid * depth_per_node);
		index = SB_NR_TO_INDEX(sb, __bitnr);
		map = sb->numa_map[nid];
		map += index;

		// debug part
		{
			unsigned int shift = sb->shift;
			unsigned int depth_per_node_shift = sb->depth_per_node_shift;
			int normal_shift = ilog2(BITS_PER_LONG);
			unsigned int nid_debug = bitnr >> depth_per_node_shift;
			unsigned int __bitnr_debug = bitnr;
			unsigned int depth_per_node_mask = (1 << sb->depth_per_node_shift) - 1;
			unsigned int bindex;
			unsigned int bindex_debug;
			unsigned int bindex_debug2;
			unsigned int bindex_debug_mask = (1 << (sb->depth_per_node_shift - sb->shift)) - 1;

			__bitnr_debug &= depth_per_node_mask;

			if (nid != nid_debug)
				pr_err_once("%s error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask);

			if (__bitnr_debug != __bitnr)
				pr_err_once("%s2 error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug);

			bindex = SB_NR_TO_INDEX(sb, __bitnr);
			bindex_debug = (bitnr & depth_per_node_mask) >> shift;
			bindex_debug2 = __bitnr_debug >> shift;

			if (bindex != bindex_debug)
				pr_err_once("%s3 error bindex=%d bindex_debug=%d nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex_debug_mask=0x%x index=%d shift=%d\n",
					__func__, bindex, bindex_debug, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex_debug_mask, index, shift);

			if (bindex != bindex_debug2)
				pr_err_once("%s4 error bindex=%d bindex_debug=%d bindex_debug2=%d nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex_debug_mask=0x%x index=%d shift=%d\n",
					__func__, bindex, bindex_debug, bindex_debug2, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex_debug_mask, index, shift);


			if (__bitnr_debug != __bitnr)
				pr_err_once("%s5 error nid=%d nid_debug=%d bitnr=%d depth_per_node_shift=%d normal_shift=%d depth_per_node_mask=0x%x __bitnr=%d __bitnr_debug=%d bindex=%d bindex_debug=%d\n",
					__func__, nid, nid_debug, bitnr, depth_per_node_shift, normal_shift, depth_per_node_mask, __bitnr, __bitnr_debug, bindex, bindex_debug);
		}

//		addr = &map->cleared;
//		test = test_bit(SB_NR_TO_BIT(sb, __bitnr), addr);
//		WARN_ONCE(test, "%s bitnr=%d index=%d nid=%d __bitnr=%d depth_per_node=%d\n", __func__, bitnr, index, nid, __bitnr, depth_per_node);

//		set_bit(SB_NR_TO_BIT(sb, __bitnr), addr);
	} else {
//		addr = &sb->map[SB_NR_TO_INDEX(sb, bitnr)].cleared;
//		set_bit(SB_NR_TO_BIT(sb, bitnr), addr);
	}
}


static inline unsigned update_alloc_hint_before_get(struct sbitmap *sb,
						    unsigned int depth, int *nid, int *cpu_ptr)
{
	unsigned hint;
	
	unsigned int *hint_ptr;
	int cpu = raw_smp_processor_id();
	*nid = cpu_to_node(cpu);
	*cpu_ptr = cpu;
	hint_ptr = per_cpu_ptr(sb->alloc_hint, cpu);

	hint = *hint_ptr;
#if 0
	if (sb->map_nr_numa) {
		unsigned int depth_per_node = sb->depth_per_node;
		int nid = cpu_to_node(cpu);
		unsigned int base = depth_per_node * nid;
		unsigned int limit = base + depth_per_node;

		if (hint < base || hint >= limit)
			pr_err_ratelimited("%s cpu%d hint=%d base=%d limit=%d\n", __func__, cpu, hint, base, limit);
	}
#endif
	if (unlikely(hint >= depth)) {
		WARN_ONCE(1, "%s sb=%pS hint=%d depth=%d map_nr=%d\n", __func__, sb, hint, depth, sb->map_nr);
		hint = depth ? prandom_u32() % depth : 0;
		this_cpu_write(*sb->alloc_hint, hint);
		BUG_ON(sb->map_nr_numa);
	}

	return hint;
}

static inline void update_alloc_hint_after_get(struct sbitmap *sb,
					       unsigned int depth,
					       unsigned int hint,
					       unsigned int nr,
					       int nid,
					       int cpu)
{
	if (nr == -1) {
		/* If the map is full, a hint won't do us much good. */
		this_cpu_write(*sb->alloc_hint, 0);
	} else if (nr == hint || unlikely(sb->round_robin)) {
		/* Only update the hint if we used it. */
		if (sb->map_nr_numa) {
//			int cpu = raw_smp_processor_id();
	//		int cpu_nid = cpu_to_node(cpu);
		//	unsigned int depth_per_node = sb->depth_per_node;
			unsigned int *hint_ptr = per_cpu_ptr(sb->alloc_hint, cpu);
			unsigned int depth_per_node = sb->depth_per_node;

			//if (cpu_nid != nid) {
			if (0) {
			//	unsigned int base = depth_per_node * cpu_nid;
			//	hint = base + (prandom_u32() % depth_per_node);
			} else {
				unsigned int base = depth_per_node * nid;
				unsigned int limit = base + depth_per_node;

				hint = nr + 1;
				if (hint == limit)
					hint = base;
			}

			//pr_err_ratelimited("%s nr=%d depth_per_node=%d base=%d limit=%d\n", __func__, nr, depth_per_node, base, limit);
		
			*hint_ptr = hint;
			sbitmap_check_hint(sb, cpu, hint);
		} else {
			hint = nr + 1;
			if (hint >= depth - 1)
				hint = 0;
			this_cpu_write(*sb->alloc_hint, hint);
		}
	}
}

/*
 * See if we have deferred clears that we can batch move
 */
static inline bool sbitmap_deferred_clear(struct sbitmap_word *map)
{
	unsigned long mask;

	if (!READ_ONCE(map->cleared))
		return false;

	/*
	 * First get a stable cleared mask, setting the old mask to 0.
	 */
	mask = xchg(&map->cleared, 0);

	/*
	 * Now clear the masked bits in our free word
	 */
	atomic_long_andnot(mask, (atomic_long_t *)&map->word);
	BUILD_BUG_ON(sizeof(atomic_long_t) != sizeof(map->word));
	return true;
}

int sbitmap_init_node(struct sbitmap *sb, unsigned int depth, int shift,
		      gfp_t flags, int node, bool round_robin,
		      bool alloc_hint)
{
	unsigned int bits_per_word;
	unsigned int i;
	int bit;
//	pr_err("%s numa_nodes_parsed=%d MAX_NUMNODES=%d\n", __func__, num_online_nodes(), MAX_NUMNODES);
	if (shift < 0)
		shift = sbitmap_calculate_shift(depth);

	bits_per_word = 1U << shift;
	if (bits_per_word > BITS_PER_LONG)
		return -EINVAL;

	sb->shift = shift;
	sb->node = node;
	sb->depth = depth;
	sb->round_robin = round_robin;

//	if ((depth % num_online_nodes() == 0) && (depth > 4000) && (depth % bits_per_word == 0)) //1024 is max sdev queue depth and then MAX_SCHED_RQ is 2000
	if (depth > 4000 && is_power_of_2(depth) && is_power_of_2(num_online_nodes()))
		sb->numa_aware = true;
	else
		sb->numa_aware = false;

	//pr_err("%s2 numa_aware=%d depth=%d bits_per_word=%d sb=%pS\n", __func__, sb->numa_aware, depth, bits_per_word, sb);

	if (depth == 0) {
		sb->map = NULL;
		return 0;
	}

	if (alloc_hint) {
		sb->depth_per_node = sb->depth / num_online_nodes();
		if (init_alloc_hint(sb, flags))
			return -ENOMEM;
	} else {
		sb->alloc_hint = NULL;
	}

	if (sb->numa_aware) {
		int nid;
		unsigned int depth_per_node;
		unsigned int depth_per_node_shift;

		depth_per_node = sb->depth_per_node = sb->depth / num_online_nodes();
		depth_per_node_shift = sb->depth_per_node_shift = ilog2(depth_per_node);


		sb->map_nr_numa = DIV_ROUND_UP(sb->depth_per_node, bits_per_word);

		pr_err("%s3 numa_aware=%d depth=%d map_nr_numa=%d bits_per_word=%d depth_per_node_shift=%d shift=%d\n",
			__func__, sb->numa_aware, depth, sb->map_nr_numa, bits_per_word, depth_per_node_shift, sb->shift);

		for (nid = 0; nid < num_online_nodes(); nid++) {
			struct page *page;
			sb->numa_map[nid] = kcalloc_node(sb->map_nr_numa, sizeof(*sb->map), flags, nid);
			if (!sb->numa_map[nid]) {
				free_percpu(sb->alloc_hint);
				return -ENOMEM;
			}
			page = virt_to_page(sb->numa_map[nid]);
			pr_err("%s3 sb->numa_map[%d]=%pS page=%pS virt_to_node=%d sz=%ld\n",
				__func__, nid, sb->numa_map[nid], page, page_to_nid(page), sb->map_nr_numa * sizeof(*sb->map));

			for (i = 0; i < sb->map_nr_numa; i++) {
				struct sbitmap_word *numa_map = sb->numa_map[nid];
	
				numa_map[i].depth = min(depth, bits_per_word);
		//		pr_err("%s3.1 nid=%d numa_map[%d].depth=%lu\n", __func__, nid, i, numa_map[i].depth);
				depth -= numa_map[i].depth;
			}
		}

	} else {
		sb->map_nr = DIV_ROUND_UP(sb->depth, bits_per_word);

		//pr_err("%s4 numa_aware=%d depth=%d map_nr=%d\n", __func__, sb->numa_aware, depth, sb->map_nr);

		sb->map = kcalloc_node(sb->map_nr, sizeof(*sb->map), flags, node);
		if (!sb->map) {
			free_percpu(sb->alloc_hint);
			return -ENOMEM;
		}

		for (i = 0; i < sb->map_nr; i++) {
			sb->map[i].depth = min(depth, bits_per_word);
			depth -= sb->map[i].depth;
		}
	}

	for (bit = 0; bit < sb->depth; bit++)
		sbitmap_deferred_clear_bit_debug(sb, bit);
	
	return 0;
}
EXPORT_SYMBOL_GPL(sbitmap_init_node);

void sbitmap_resize(struct sbitmap *sb, unsigned int depth)
{
	unsigned int bits_per_word = 1U << sb->shift;
	unsigned int i;

	if (WARN_ONCE(sb->numa_aware, "%s depth=%d sb->depth=%d\n", __func__, depth, sb->depth))
		return;

	for (i = 0; i < sb->map_nr; i++)
		sbitmap_deferred_clear(&sb->map[i]);

	sb->depth = depth;
	sb->map_nr = DIV_ROUND_UP(sb->depth, bits_per_word);

	for (i = 0; i < sb->map_nr; i++) {
		sb->map[i].depth = min(depth, bits_per_word);
		depth -= sb->map[i].depth;
	}
}
EXPORT_SYMBOL_GPL(sbitmap_resize);

static int __sbitmap_get_word(unsigned long *word, unsigned long depth,
			      unsigned int hint, bool wrap)
{
	int nr;

	/* don't wrap if starting from 0 */
	wrap = wrap && hint;

	while (1) {
		nr = find_next_zero_bit(word, depth, hint);
		if (unlikely(nr >= depth)) {
			/*
			 * We started with an offset, and we didn't reset the
			 * offset to 0 in a failure case, so start from 0 to
			 * exhaust the map.
			 */
			if (hint && wrap) {
				hint = 0;
				continue;
			}
			return -1;
		}

		if (!test_and_set_bit_lock(nr, word))
			break;

		hint = nr + 1;
		if (hint >= depth - 1)
			hint = 0;
	}

	return nr;
}

static int sbitmap_find_bit_in_index(struct sbitmap_word *map,
				     unsigned int alloc_hint, int round_robin)
{
	int nr;


	do {
		nr = __sbitmap_get_word(&map->word, map->depth, alloc_hint,
					!round_robin);
		if (nr != -1)
			break;
		if (!sbitmap_deferred_clear(map))
			break;
	} while (1);

	return nr;
}


static void __sbitmap_get_debug(struct sbitmap *sb, const unsigned int alloc_hint,  const int nid,  const unsigned int index, const unsigned int base, const unsigned int __alloc_hint)
{
	const unsigned int depth_per_node_shift = sb->depth_per_node_shift;
	const unsigned int depth_per_node_mask = (1 << sb->depth_per_node_shift) - 1;
	unsigned int b__alloc_hint = alloc_hint & depth_per_node_mask;
	const unsigned int nid_debug = alloc_hint >> depth_per_node_shift;
	const unsigned int shift = sb->shift;
	unsigned int bindex = b__alloc_hint >> shift;
	//unsigned int b__alloc_hint2 = SB_NR_TO_BIT(sb, b__alloc_hint);
	unsigned int debug_base;
	
	struct sbitmap_word *map;
	
	if (index > 50000)
		pr_err_once("%s x3 index=%d\n", __func__, index);

	map = sb->numa_map[nid];
	if (bindex != index)
		panic("%s error index=%d nid=%d nid_debug=%d alloc_hint=%d depth_per_node_shift=%d depth_per_node_mask=0x%x bindex=%d index=%d\n",
			__func__, index, nid, nid_debug, alloc_hint, depth_per_node_shift, depth_per_node_mask, bindex, index);

		//	if (__alloc_hint_temp != b__alloc_hint2)
		//		pr_err_once("%s2 error SB_NR_TO_BIT(sb, __alloc_hint)=%d b__alloc_hint=%d b__alloc_hint2=%d alloc_hint=%d __alloc_hint_temp=%d\n",
		//			__func__, SB_NR_TO_BIT(sb, __alloc_hint), b__alloc_hint, b__alloc_hint2, alloc_hint, __alloc_hint_temp);

	debug_base = alloc_hint & ~depth_per_node_mask;
	if (base != debug_base)
		panic("%s3 error alloc_hint=%d base=%d debug_base=%d\n",
					__func__, alloc_hint, base, debug_base);


	if (sb->round_robin)
		b__alloc_hint = SB_NR_TO_BIT(sb, b__alloc_hint);
	else
		b__alloc_hint = 0;

	if (b__alloc_hint != __alloc_hint)
		panic("%s4 error alloc_hint=%d base=%d debug_base=%d b__alloc_hint=%d __alloc_hint=%d\n",
					__func__, alloc_hint, base, debug_base, b__alloc_hint, __alloc_hint);
}

static int __sbitmap_get(struct sbitmap *sb, const unsigned int alloc_hint, const unsigned int nid, const unsigned int cpu)
{
	unsigned int i, index;
	int nr = -1;

	unsigned int __alloc_hint = alloc_hint;
//	static unsigned long long count__sbitmap_get;
//	static unsigned long long count__sbitmap_get_no_bit;

	/*
	 * Unless we're doing round robin tag allocation, just use the
	 * alloc_hint to find the right word index. No point in looping
	 * twice in find_next_zero_bit() for that case.
	 */
	//count__sbitmap_get++;

	if (sb->numa_aware) {
		struct sbitmap_word *map;

		unsigned int depth_per_node = sb->depth_per_node;

		unsigned int depth_per_node_shift = sb->depth_per_node_shift;
		unsigned int base;


		if (1) {
			//unsigned int nid = __alloc_hint / depth_per_node;
			unsigned int base2 = nid << depth_per_node_shift;
			unsigned int __alloc_hint_temp;
		//	unsigned int index2;
		//	index2 = SB_NR_TO_INDEX(sb, __alloc_hint);
			base = nid * depth_per_node;
			__alloc_hint -= base;
			index = SB_NR_TO_INDEX(sb, __alloc_hint);
			if (base != base2)
				pr_err_once("%s error base=%d base2=%d depth_per_node=%d depth_per_node_shift=%d\n",
					__func__, base, base2, depth_per_node, depth_per_node_shift);
		//	if (index != index2)
		//		pr_err_once("%s index=%d index2=%d alloc_hint=%d depth_per_node=%d nid=%d sb->map_nr_numa=%d round_robin=%d\n",
		//		__func__, index, index2, alloc_hint, depth_per_node, nid, sb->map_nr_numa, sb->round_robin);
			map = sb->numa_map[nid];

			__alloc_hint_temp = SB_NR_TO_BIT(sb, __alloc_hint);


			//debug
			if (index > 50000)
				pr_err_once("%s x1 index=%d alloc_hint=%d nid=%d base2=%d __alloc_hint=%d cpu=%d\n", __func__, index, alloc_hint, nid, base2, __alloc_hint, cpu);

			if (sb->round_robin)
				__alloc_hint = SB_NR_TO_BIT(sb, __alloc_hint);
			else
				__alloc_hint = 0;
		}else {
			const unsigned int depth_per_node_shift = sb->depth_per_node_shift;
			const unsigned int depth_per_node_mask = (1 << sb->depth_per_node_shift) - 1;
			unsigned int b__alloc_hint = alloc_hint & depth_per_node_mask;
			const unsigned int nid_debug = alloc_hint >> depth_per_node_shift;
			const unsigned int shift = sb->shift;
			unsigned int bindex = b__alloc_hint >> shift;
			//unsigned int b__alloc_hint2 = SB_NR_TO_BIT(sb, b__alloc_hint);
			unsigned int debug_base;

			map = sb->numa_map[nid];
			index = bindex;
			if (bindex != index)
				panic("%s error nid=%d nid_debug=%d alloc_hint=%d depth_per_node_shift=%d depth_per_node_mask=0x%x bindex=%d index=%d\n",
					__func__, nid, nid_debug, alloc_hint, depth_per_node_shift, depth_per_node_mask, bindex, index);

		//	if (__alloc_hint_temp != b__alloc_hint2)
		//		pr_err_once("%s2 error SB_NR_TO_BIT(sb, __alloc_hint)=%d b__alloc_hint=%d b__alloc_hint2=%d alloc_hint=%d __alloc_hint_temp=%d\n",
		//			__func__, SB_NR_TO_BIT(sb, __alloc_hint), b__alloc_hint, b__alloc_hint2, alloc_hint, __alloc_hint_temp);

			debug_base = alloc_hint & ~depth_per_node_mask;
			base = debug_base;
			if (base != debug_base)
				panic("%s3 error alloc_hint=%d base=%d debug_base=%d\n",
					__func__, alloc_hint, base, debug_base);


			if (sb->round_robin)
				__alloc_hint = SB_NR_TO_BIT(sb, b__alloc_hint);
			else
				__alloc_hint = 0;
		}
		
		if (index > 50000)
			pr_err_once("%s x2 index=%d\n", __func__, index);

		__sbitmap_get_debug(sb, alloc_hint, nid, index, base, __alloc_hint);

		for (i = 0; i < sb->map_nr_numa; i++) {
			struct sbitmap_word *map2 = &map[index];
	//		struct page *page = virt_to_page(map2);
//			int nid_page = page_to_nid(page);
		//	static unsigned long long count__sbitmap_get;
		//	static unsigned long long count__sbitmap_get_wrong_nid;
		//	count__sbitmap_get++;
		//	if (nid != nid_page)
		//		count__sbitmap_get_wrong_nid++;

		//	if ((count__sbitmap_get % 5000000) == 0)
		//		pr_err("%s count__sbitmap_get=%lld wrong=%lld\n", __func__, count__sbitmap_get, count__sbitmap_get_wrong_nid);

			nr = sbitmap_find_bit_in_index(map2, __alloc_hint, sb->round_robin);
			if (nr != -1) {
				nr += index << sb->shift;
				nr += base;
				//nr += index * depth_per_node;

				break;
			}
		
			/* Jump to next index. */
			__alloc_hint = 0;
			if (++index >= sb->map_nr_numa)
				index = 0;
		}
	} else {
		index = SB_NR_TO_INDEX(sb, __alloc_hint);

		/*
		 * Unless we're doing round robin tag allocation, just use the
		 * alloc_hint to find the right word index. No point in looping
		 * twice in find_next_zero_bit() for that case.
		 */
		if (sb->round_robin)
			__alloc_hint = SB_NR_TO_BIT(sb, __alloc_hint);
		else
			__alloc_hint = 0;

		for (i = 0; i < sb->map_nr; i++) {
			struct sbitmap_word *map = &sb->map[index];

			nr = sbitmap_find_bit_in_index(map, __alloc_hint, sb->round_robin);
			if (nr != -1) {
				nr += index << sb->shift;
				break;
			}

			/* Jump to next index. */
			__alloc_hint = 0;
			if (++index >= sb->map_nr)
				index = 0;
		}

	}

//	if (nr == -1)
//		count__sbitmap_get_no_bit++;
	
//	if ((count__sbitmap_get % 5000000) == 0)
//		pr_err("%s count__sbitmap_get=%lld none=%lld\n", __func__, count__sbitmap_get, count__sbitmap_get_no_bit);

	return nr;
}

int sbitmap_get(struct sbitmap *sb)
{
	int nr;
	unsigned int hint, depth;
	int nid, cpu;

	if (WARN_ON_ONCE(unlikely(!sb->alloc_hint)))
		return -1;

	depth = READ_ONCE(sb->depth);
	hint = update_alloc_hint_before_get(sb, depth, &nid, &cpu);
	nr = __sbitmap_get(sb, hint, nid, cpu);
	update_alloc_hint_after_get(sb, depth, hint, nr, nid, cpu);

	return nr;
}
EXPORT_SYMBOL_GPL(sbitmap_get);

static int __sbitmap_get_shallow(struct sbitmap *sb,
				 unsigned int alloc_hint,
				 unsigned long shallow_depth)
{
	unsigned int i, index;
	int nr = -1;

	index = SB_NR_TO_INDEX(sb, alloc_hint);

	if (WARN_ON_ONCE(sb->numa_aware))
		return -1;

	for (i = 0; i < sb->map_nr; i++) {
again:
		nr = __sbitmap_get_word(&sb->map[index].word,
					min(sb->map[index].depth, shallow_depth),
					SB_NR_TO_BIT(sb, alloc_hint), true);
		if (nr != -1) {
			nr += index << sb->shift;
			break;
		}

		if (sbitmap_deferred_clear(&sb->map[index]))
			goto again;

		/* Jump to next index. */
		index++;
		alloc_hint = index << sb->shift;

		if (index >= sb->map_nr) {
			index = 0;
			alloc_hint = 0;
		}
	}

	return nr;
}

int sbitmap_get_shallow(struct sbitmap *sb, unsigned long shallow_depth)
{
	int nr;
	unsigned int hint, depth;
	int nid;
	int cpu;

	if (WARN_ON_ONCE(unlikely(!sb->alloc_hint)))
		return -1;

	depth = READ_ONCE(sb->depth);
	hint = update_alloc_hint_before_get(sb, depth, &nid, &cpu);
	nr = __sbitmap_get_shallow(sb, hint, shallow_depth);
	update_alloc_hint_after_get(sb, depth, hint, nr, nid, cpu);

	return nr;
}
EXPORT_SYMBOL_GPL(sbitmap_get_shallow);

bool sbitmap_any_bit_set(const struct sbitmap *sb)
{
	unsigned int i;

	bool numa_aware = sb->numa_aware;

	if (numa_aware) {
		int nid;

		for (nid = 0; nid < num_online_nodes(); nid++) {
			struct sbitmap_word *map = sb->numa_map[nid];
			for (i = 0; i < sb->map_nr_numa; i++) {
				struct sbitmap_word *map2 = &map[i];
				if (map2[i].word & ~map2[i].cleared)
					return true;
			}
		}
	} else {
		for (i = 0; i < sb->map_nr; i++) {
			if (sb->map[i].word & ~sb->map[i].cleared)
				return true;
		}
	}

	return false;
}
EXPORT_SYMBOL_GPL(sbitmap_any_bit_set);

static unsigned int __sbitmap_weight(const struct sbitmap *sb, bool set)
{
	unsigned int i, weight = 0;
	bool numa_aware = sb->numa_aware;

	if (numa_aware) {
		int nid;

		for (nid = 0; nid < num_online_nodes(); nid++) {
			struct sbitmap_word *map = sb->numa_map[nid];
			for (i = 0; i < sb->map_nr_numa; i++) {
				const struct sbitmap_word *word = &map[i];
		
				if (set)
					weight += bitmap_weight(&word->word, word->depth);
				else
					weight += bitmap_weight(&word->cleared, word->depth);
			}
		}
	} else {
		for (i = 0; i < sb->map_nr; i++) {
			const struct sbitmap_word *word = &sb->map[i];

			if (set)
				weight += bitmap_weight(&word->word, word->depth);
			else
				weight += bitmap_weight(&word->cleared, word->depth);
		}
	}
	return weight;
}

static unsigned int sbitmap_cleared(const struct sbitmap *sb)
{
	return __sbitmap_weight(sb, false);
}

unsigned int sbitmap_weight(const struct sbitmap *sb)
{
	return __sbitmap_weight(sb, true) - sbitmap_cleared(sb);
}
EXPORT_SYMBOL_GPL(sbitmap_weight);

void sbitmap_show(struct sbitmap *sb, struct seq_file *m)
{
	#ifdef experiment
	seq_printf(m, "depth=%u\n", sb->depth);
	seq_printf(m, "busy=%u\n", sbitmap_weight(sb));
	seq_printf(m, "cleared=%u\n", sbitmap_cleared(sb));
	seq_printf(m, "bits_per_word=%u\n", 1U << sb->shift);
	seq_printf(m, "map_nr=%u\n", sb->map_nr);
	#endif
}
EXPORT_SYMBOL_GPL(sbitmap_show);

static inline void emit_byte(struct seq_file *m, unsigned int offset, u8 byte)
{
	if ((offset & 0xf) == 0) {
		if (offset != 0)
			seq_putc(m, '\n');
		seq_printf(m, "%08x:", offset);
	}
	if ((offset & 0x1) == 0)
		seq_putc(m, ' ');
	seq_printf(m, "%02x", byte);
}

void sbitmap_bitmap_show(struct sbitmap *sb, struct seq_file *m)
{
	#ifdef experiment
	u8 byte = 0;
	unsigned int byte_bits = 0;
	unsigned int offset = 0;
	int i;

	for (i = 0; i < sb->map_nr; i++) {
		unsigned long word = READ_ONCE(sb->map[i].word);
		unsigned long cleared = READ_ONCE(sb->map[i].cleared);
		unsigned int word_bits = READ_ONCE(sb->map[i].depth);

		word &= ~cleared;

		while (word_bits > 0) {
			unsigned int bits = min(8 - byte_bits, word_bits);

			byte |= (word & (BIT(bits) - 1)) << byte_bits;
			byte_bits += bits;
			if (byte_bits == 8) {
				emit_byte(m, offset, byte);
				byte = 0;
				byte_bits = 0;
				offset++;
			}
			word >>= bits;
			word_bits -= bits;
		}
	}
	if (byte_bits) {
		emit_byte(m, offset, byte);
		offset++;
	}
	if (offset)
		seq_putc(m, '\n');
	#endif
}
EXPORT_SYMBOL_GPL(sbitmap_bitmap_show);

static unsigned int sbq_calc_wake_batch(struct sbitmap_queue *sbq,
					unsigned int depth)
{
	unsigned int wake_batch;
	unsigned int shallow_depth;

	/*
	 * For each batch, we wake up one queue. We need to make sure that our
	 * batch size is small enough that the full depth of the bitmap,
	 * potentially limited by a shallow depth, is enough to wake up all of
	 * the queues.
	 *
	 * Each full word of the bitmap has bits_per_word bits, and there might
	 * be a partial word. There are depth / bits_per_word full words and
	 * depth % bits_per_word bits left over. In bitwise arithmetic:
	 *
	 * bits_per_word = 1 << shift
	 * depth / bits_per_word = depth >> shift
	 * depth % bits_per_word = depth & ((1 << shift) - 1)
	 *
	 * Each word can be limited to sbq->min_shallow_depth bits.
	 */
	shallow_depth = min(1U << sbq->sb.shift, sbq->min_shallow_depth);
	depth = ((depth >> sbq->sb.shift) * shallow_depth +
		 min(depth & ((1U << sbq->sb.shift) - 1), shallow_depth));
	wake_batch = clamp_t(unsigned int, depth / SBQ_WAIT_QUEUES, 1,
			     SBQ_WAKE_BATCH);

	return wake_batch;
}

int sbitmap_queue_init_node(struct sbitmap_queue *sbq, unsigned int depth,
			    int shift, bool round_robin, gfp_t flags, int node)
{
	int ret;
	int i;

	ret = sbitmap_init_node(&sbq->sb, depth, shift, flags, node,
				round_robin, true);
	if (ret)
		return ret;

	sbq->min_shallow_depth = UINT_MAX;
	sbq->wake_batch = sbq_calc_wake_batch(sbq, depth);
	atomic_set(&sbq->wake_index, 0);
	atomic_set(&sbq->ws_active, 0);

	sbq->ws = kzalloc_node(SBQ_WAIT_QUEUES * sizeof(*sbq->ws), flags, node);
	if (!sbq->ws) {
		sbitmap_free(&sbq->sb);
		return -ENOMEM;
	}

	for (i = 0; i < SBQ_WAIT_QUEUES; i++) {
		init_waitqueue_head(&sbq->ws[i].wait);
		atomic_set(&sbq->ws[i].wait_cnt, sbq->wake_batch);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(sbitmap_queue_init_node);

static inline void __sbitmap_queue_update_wake_batch(struct sbitmap_queue *sbq,
					    unsigned int wake_batch)
{
	int i;

	if (sbq->wake_batch != wake_batch) {
		WRITE_ONCE(sbq->wake_batch, wake_batch);
		/*
		 * Pairs with the memory barrier in sbitmap_queue_wake_up()
		 * to ensure that the batch size is updated before the wait
		 * counts.
		 */
		smp_mb();
		for (i = 0; i < SBQ_WAIT_QUEUES; i++)
			atomic_set(&sbq->ws[i].wait_cnt, 1);
	}
}

static void sbitmap_queue_update_wake_batch(struct sbitmap_queue *sbq,
					    unsigned int depth)
{
	unsigned int wake_batch;

	wake_batch = sbq_calc_wake_batch(sbq, depth);
	__sbitmap_queue_update_wake_batch(sbq, wake_batch);
}

void sbitmap_queue_recalculate_wake_batch(struct sbitmap_queue *sbq,
					    unsigned int users)
{
	unsigned int wake_batch;
	unsigned int min_batch;
	unsigned int depth = (sbq->sb.depth + users - 1) / users;

	min_batch = sbq->sb.depth >= (4 * SBQ_WAIT_QUEUES) ? 4 : 1;

	wake_batch = clamp_val(depth / SBQ_WAIT_QUEUES,
			min_batch, SBQ_WAKE_BATCH);
	__sbitmap_queue_update_wake_batch(sbq, wake_batch);
}
EXPORT_SYMBOL_GPL(sbitmap_queue_recalculate_wake_batch);

void sbitmap_queue_resize(struct sbitmap_queue *sbq, unsigned int depth)
{
	sbitmap_queue_update_wake_batch(sbq, depth);
	sbitmap_resize(&sbq->sb, depth);
}
EXPORT_SYMBOL_GPL(sbitmap_queue_resize);

int __sbitmap_queue_get(struct sbitmap_queue *sbq)
{
	return sbitmap_get(&sbq->sb);
}
EXPORT_SYMBOL_GPL(__sbitmap_queue_get);

unsigned long __sbitmap_queue_get_batch(struct sbitmap_queue *sbq, int nr_tags,
					unsigned int *offset)
{
	#ifdef experiment
	struct sbitmap *sb = &sbq->sb;
	unsigned int hint, depth;
	unsigned long index, nr;
	int i;

	if (unlikely(sb->round_robin))
		return 0;

	depth = READ_ONCE(sb->depth);
	hint = update_alloc_hint_before_get(sb, depth);

	index = SB_NR_TO_INDEX(sb, hint);

	for (i = 0; i < sb->map_nr; i++) {
		struct sbitmap_word *map = &sb->map[index];
		unsigned long get_mask;

		sbitmap_deferred_clear(map);
		if (map->word == (1UL << (map->depth - 1)) - 1)
			continue;

		nr = find_first_zero_bit(&map->word, map->depth);
		if (nr + nr_tags <= map->depth) {
			atomic_long_t *ptr = (atomic_long_t *) &map->word;
			int map_tags = min_t(int, nr_tags, map->depth);
			unsigned long val, ret;

			get_mask = ((1UL << map_tags) - 1) << nr;
			do {
				val = READ_ONCE(map->word);
				ret = atomic_long_cmpxchg(ptr, val, get_mask | val);
			} while (ret != val);
			get_mask = (get_mask & ~ret) >> nr;
			if (get_mask) {
				*offset = nr + (index << sb->shift);
				update_alloc_hint_after_get(sb, depth, hint,
							*offset + map_tags - 1);
				return get_mask;
			}
		}
		/* Jump to next index. */
		if (++index >= sb->map_nr)
			index = 0;
	}
	#endif

	return 0;
}

int __sbitmap_queue_get_shallow(struct sbitmap_queue *sbq,
				unsigned int shallow_depth)
{
	WARN_ON_ONCE(shallow_depth < sbq->min_shallow_depth);

	return sbitmap_get_shallow(&sbq->sb, shallow_depth);
}
EXPORT_SYMBOL_GPL(__sbitmap_queue_get_shallow);

void sbitmap_queue_min_shallow_depth(struct sbitmap_queue *sbq,
				     unsigned int min_shallow_depth)
{
	sbq->min_shallow_depth = min_shallow_depth;
	sbitmap_queue_update_wake_batch(sbq, sbq->sb.depth);
}
EXPORT_SYMBOL_GPL(sbitmap_queue_min_shallow_depth);

static struct sbq_wait_state *sbq_wake_ptr(struct sbitmap_queue *sbq)
{
	int i, wake_index;

	if (!atomic_read(&sbq->ws_active))
		return NULL;

	wake_index = atomic_read(&sbq->wake_index);
	for (i = 0; i < SBQ_WAIT_QUEUES; i++) {
		struct sbq_wait_state *ws = &sbq->ws[wake_index];

		if (waitqueue_active(&ws->wait)) {
			if (wake_index != atomic_read(&sbq->wake_index))
				atomic_set(&sbq->wake_index, wake_index);
			return ws;
		}

		wake_index = sbq_index_inc(wake_index);
	}

	return NULL;
}

static bool __sbq_wake_up(struct sbitmap_queue *sbq)
{
	struct sbq_wait_state *ws;
	unsigned int wake_batch;
	int wait_cnt;

	ws = sbq_wake_ptr(sbq);
	if (!ws)
		return false;

	wait_cnt = atomic_dec_return(&ws->wait_cnt);
	if (wait_cnt <= 0) {
		int ret;

		wake_batch = READ_ONCE(sbq->wake_batch);

		/*
		 * Pairs with the memory barrier in sbitmap_queue_resize() to
		 * ensure that we see the batch size update before the wait
		 * count is reset.
		 */
		smp_mb__before_atomic();

		/*
		 * For concurrent callers of this, the one that failed the
		 * atomic_cmpxhcg() race should call this function again
		 * to wakeup a new batch on a different 'ws'.
		 */
		ret = atomic_cmpxchg(&ws->wait_cnt, wait_cnt, wake_batch);
		if (ret == wait_cnt) {
			sbq_index_atomic_inc(&sbq->wake_index);
			wake_up_nr(&ws->wait, wake_batch);
			return false;
		}

		return true;
	}

	return false;
}

void sbitmap_queue_wake_up(struct sbitmap_queue *sbq)
{
	while (__sbq_wake_up(sbq))
		;
}
EXPORT_SYMBOL_GPL(sbitmap_queue_wake_up);

static inline void sbitmap_update_cpu_hint(struct sbitmap *sb, int cpu, int tag)
{
	if (likely(!sb->round_robin && tag < sb->depth)) {
		if (sb->map_nr_numa) {
			unsigned int depth_per_node = sb->depth_per_node;
			int nid = cpu_to_node(cpu);
			int tag_nid = tag / depth_per_node;
			
			if (nid != tag_nid) {
				unsigned int base = nid * depth_per_node;

				tag = base + (prandom_u32() % depth_per_node);
			}
			sbitmap_check_hint(sb, cpu, tag);
			data_race(*per_cpu_ptr(sb->alloc_hint, cpu) = tag);
		} else {
			data_race(*per_cpu_ptr(sb->alloc_hint, cpu) = tag);
		}
	}
}

void sbitmap_queue_clear_batch(struct sbitmap_queue *sbq, int offset,
				int *tags, int nr_tags)
{
	struct sbitmap *sb = &sbq->sb;

	#ifdef experiment
	unsigned long *addr = NULL;
	unsigned long mask = 0;
	int i;

	smp_mb__before_atomic();
	for (i = 0; i < nr_tags; i++) {
		const int tag = tags[i] - offset;
		unsigned long *this_addr;

		/* since we're clearing a batch, skip the deferred map */
		this_addr = &sb->map[SB_NR_TO_INDEX(sb, tag)].word;
		if (!addr) {
			addr = this_addr;
		} else if (addr != this_addr) {
			atomic_long_andnot(mask, (atomic_long_t *) addr);
			mask = 0;
			addr = this_addr;
		}
		mask |= (1UL << SB_NR_TO_BIT(sb, tag));
	}

	if (mask)
		atomic_long_andnot(mask, (atomic_long_t *) addr);

	smp_mb__after_atomic();
	sbitmap_queue_wake_up(sbq);
	sbitmap_update_cpu_hint(&sbq->sb, raw_smp_processor_id(),
					tags[nr_tags - 1] - offset);
	#else
	BUG_ON(sb->map_nr_numa);
	#endif
}

void sbitmap_queue_clear(struct sbitmap_queue *sbq, unsigned int nr,
			 unsigned int cpu)
{
	/*
	 * Once the clear bit is set, the bit may be allocated out.
	 *
	 * Orders READ/WRITE on the associated instance(such as request
	 * of blk_mq) by this bit for avoiding race with re-allocation,
	 * and its pair is the memory barrier implied in __sbitmap_get_word.
	 *
	 * One invariant is that the clear bit has to be zero when the bit
	 * is in use.
	 */
	smp_mb__before_atomic();
	sbitmap_deferred_clear_bit(&sbq->sb, nr);

	/*
	 * Pairs with the memory barrier in set_current_state() to ensure the
	 * proper ordering of clear_bit_unlock()/waitqueue_active() in the waker
	 * and test_and_set_bit_lock()/prepare_to_wait()/finish_wait() in the
	 * waiter. See the comment on waitqueue_active().
	 */
	smp_mb__after_atomic();
	sbitmap_queue_wake_up(sbq);
	sbitmap_update_cpu_hint(&sbq->sb, cpu, nr);
}
EXPORT_SYMBOL_GPL(sbitmap_queue_clear);

void sbitmap_queue_wake_all(struct sbitmap_queue *sbq)
{
	int i, wake_index;

	/*
	 * Pairs with the memory barrier in set_current_state() like in
	 * sbitmap_queue_wake_up().
	 */
	smp_mb();
	wake_index = atomic_read(&sbq->wake_index);
	for (i = 0; i < SBQ_WAIT_QUEUES; i++) {
		struct sbq_wait_state *ws = &sbq->ws[wake_index];

		if (waitqueue_active(&ws->wait))
			wake_up(&ws->wait);

		wake_index = sbq_index_inc(wake_index);
	}
}
EXPORT_SYMBOL_GPL(sbitmap_queue_wake_all);

void sbitmap_queue_show(struct sbitmap_queue *sbq, struct seq_file *m)
{
	bool first;
	int i;

	sbitmap_show(&sbq->sb, m);

	seq_puts(m, "alloc_hint={");
	first = true;
	for_each_possible_cpu(i) {
		if (!first)
			seq_puts(m, ", ");
		first = false;
		seq_printf(m, "%u", *per_cpu_ptr(sbq->sb.alloc_hint, i));
	}
	seq_puts(m, "}\n");

	seq_printf(m, "wake_batch=%u\n", sbq->wake_batch);
	seq_printf(m, "wake_index=%d\n", atomic_read(&sbq->wake_index));
	seq_printf(m, "ws_active=%d\n", atomic_read(&sbq->ws_active));

	seq_puts(m, "ws={\n");
	for (i = 0; i < SBQ_WAIT_QUEUES; i++) {
		struct sbq_wait_state *ws = &sbq->ws[i];

		seq_printf(m, "\t{.wait_cnt=%d, .wait=%s},\n",
			   atomic_read(&ws->wait_cnt),
			   waitqueue_active(&ws->wait) ? "active" : "inactive");
	}
	seq_puts(m, "}\n");

	seq_printf(m, "round_robin=%d\n", sbq->sb.round_robin);
	seq_printf(m, "min_shallow_depth=%u\n", sbq->min_shallow_depth);
}
EXPORT_SYMBOL_GPL(sbitmap_queue_show);

void sbitmap_add_wait_queue(struct sbitmap_queue *sbq,
			    struct sbq_wait_state *ws,
			    struct sbq_wait *sbq_wait)
{
	if (!sbq_wait->sbq) {
		sbq_wait->sbq = sbq;
		atomic_inc(&sbq->ws_active);
		add_wait_queue(&ws->wait, &sbq_wait->wait);
	}
}
EXPORT_SYMBOL_GPL(sbitmap_add_wait_queue);

void sbitmap_del_wait_queue(struct sbq_wait *sbq_wait)
{
	list_del_init(&sbq_wait->wait.entry);
	if (sbq_wait->sbq) {
		atomic_dec(&sbq_wait->sbq->ws_active);
		sbq_wait->sbq = NULL;
	}
}
EXPORT_SYMBOL_GPL(sbitmap_del_wait_queue);

void sbitmap_prepare_to_wait(struct sbitmap_queue *sbq,
			     struct sbq_wait_state *ws,
			     struct sbq_wait *sbq_wait, int state)
{
	if (!sbq_wait->sbq) {
		atomic_inc(&sbq->ws_active);
		sbq_wait->sbq = sbq;
	}
	prepare_to_wait_exclusive(&ws->wait, &sbq_wait->wait, state);
}
EXPORT_SYMBOL_GPL(sbitmap_prepare_to_wait);

void sbitmap_finish_wait(struct sbitmap_queue *sbq, struct sbq_wait_state *ws,
			 struct sbq_wait *sbq_wait)
{
	finish_wait(&ws->wait, &sbq_wait->wait);
	if (sbq_wait->sbq) {
		atomic_dec(&sbq->ws_active);
		sbq_wait->sbq = NULL;
	}
}
EXPORT_SYMBOL_GPL(sbitmap_finish_wait);
