// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2024 Google LLC.
 */
#include <kunit/test.h>
#include <linux/io-pgtable.h>

#include "arm-smmu-v3.h"

struct arm_smmu_test_writer {
	struct arm_smmu_entry_writer writer;
	struct kunit *test;
	const __le64 *init_entry;
	const __le64 *target_entry;
	__le64 *entry;

	bool invalid_entry_written;
	unsigned int num_syncs;
};

static struct arm_smmu_ste bypass_ste;
static struct arm_smmu_ste abort_ste;

static bool arm_smmu_entry_differs_in_used_bits(const __le64 *entry,
						const __le64 *used_bits,
						const __le64 *target,
						unsigned int length)
{
	bool differs = false;
	unsigned int i;

	for (i = 0; i < length; i++) {
		if ((entry[i] & used_bits[i]) != target[i])
			differs = true;
	}
	return differs;
}

static void
arm_smmu_test_writer_record_syncs(struct arm_smmu_entry_writer *writer)
{
	struct arm_smmu_test_writer *test_writer =
		container_of(writer, struct arm_smmu_test_writer, writer);
	__le64 *entry_used_bits;
	unsigned int num_entry_qwords = writer->ops->num_entry_qwords;

	entry_used_bits = kunit_kzalloc(
		test_writer->test, sizeof(*entry_used_bits) * num_entry_qwords,
		GFP_KERNEL);
	KUNIT_ASSERT_NOT_NULL(test_writer->test, entry_used_bits);

	pr_debug("STE value is now set to: ");
	print_hex_dump_debug("    ", DUMP_PREFIX_NONE, 16, 8,
			     test_writer->entry,
			     num_entry_qwords * sizeof(*test_writer->entry),
			     false);

	test_writer->num_syncs += 1;
	if (!(test_writer->entry[0] & writer->ops->v_bit)) {
		test_writer->invalid_entry_written = true;
	} else {
		/*
		 * At any stage in a hitless transition, the entry must be
		 * equivalent to either the initial entry or the target entry
		 * when only considering the bits used by the current
		 * configuration.
		 */
		writer->ops->get_used(test_writer->entry, entry_used_bits);
		KUNIT_EXPECT_FALSE(
			test_writer->test,
			arm_smmu_entry_differs_in_used_bits(
				test_writer->entry, entry_used_bits,
				test_writer->init_entry, num_entry_qwords) &&
				arm_smmu_entry_differs_in_used_bits(
					test_writer->entry, entry_used_bits,
					test_writer->target_entry,
					num_entry_qwords));
	}
}

static void
arm_smmu_v3_test_ste_debug_print_used_bits(struct arm_smmu_entry_writer *writer,
					   const struct arm_smmu_ste *ste)
{
	struct arm_smmu_ste used_bits = {};

	arm_smmu_get_ste_used(ste->data, used_bits.data);
	pr_debug("STE used bits: ");
	print_hex_dump_debug("    ", DUMP_PREFIX_NONE, 16, 8, used_bits.data,
			     sizeof(used_bits), false);
}

static const struct arm_smmu_entry_writer_ops test_ops = {
	.v_bit = cpu_to_le64(STRTAB_STE_0_V),
	.num_entry_qwords = sizeof(struct arm_smmu_ste) / sizeof(u64),
	.sync = arm_smmu_test_writer_record_syncs,
	.get_used = arm_smmu_get_ste_used,
};

static void arm_smmu_v3_test_ste_expect_transition(
	struct kunit *test, const struct arm_smmu_ste *cur,
	const struct arm_smmu_ste *target, int num_syncs_expected, bool hitless)
{
	struct arm_smmu_ste cur_copy = *cur;
	struct arm_smmu_test_writer test_writer = {
		.writer = {
			.ops = &test_ops,
		},
		.test = test,
		.init_entry = cur->data,
		.target_entry = target->data,
		.entry = cur_copy.data,
		.num_syncs = 0,
		.invalid_entry_written = false,

	};

	pr_debug("STE initial value: ");
	print_hex_dump_debug("    ", DUMP_PREFIX_NONE, 16, 8, cur_copy.data,
			     sizeof(cur_copy), false);
	arm_smmu_v3_test_ste_debug_print_used_bits(&test_writer.writer, cur);
	pr_debug("STE target value: ");
	print_hex_dump_debug("    ", DUMP_PREFIX_NONE, 16, 8, target->data,
			     sizeof(cur_copy), false);
	arm_smmu_v3_test_ste_debug_print_used_bits(&test_writer.writer, target);

	arm_smmu_write_entry(&test_writer.writer, cur_copy.data, target->data);

	KUNIT_EXPECT_EQ(test, test_writer.invalid_entry_written, !hitless);
	KUNIT_EXPECT_EQ(test, test_writer.num_syncs, num_syncs_expected);
	KUNIT_EXPECT_MEMEQ(test, target->data, cur_copy.data, sizeof(cur_copy));
}

static void arm_smmu_v3_test_ste_expect_non_hitless_transition(
	struct kunit *test, const struct arm_smmu_ste *cur,
	const struct arm_smmu_ste *target, int num_syncs_expected)
{
	arm_smmu_v3_test_ste_expect_transition(test, cur, target,
					       num_syncs_expected, false);
}

static void arm_smmu_v3_test_ste_expect_hitless_transition(
	struct kunit *test, const struct arm_smmu_ste *cur,
	const struct arm_smmu_ste *target, int num_syncs_expected)
{
	arm_smmu_v3_test_ste_expect_transition(test, cur, target,
					       num_syncs_expected, true);
}

static const dma_addr_t fake_cdtab_dma_addr = 0xF0F0F0F0F0F0;

static void arm_smmu_test_make_cdtable_ste(struct arm_smmu_ste *ste,
					   unsigned int s1dss,
					   const dma_addr_t dma_addr)
{
	struct arm_smmu_device smmu = { .features = ARM_SMMU_FEAT_STALLS };
	struct arm_smmu_master master = {
		.cd_table.cdtab_dma = dma_addr,
		.cd_table.s1cdmax = 0xFF,
		.cd_table.s1fmt = STRTAB_STE_0_S1FMT_64K_L2,
		.smmu = &smmu,
	};

	arm_smmu_make_cdtable_ste(ste, &master, true, s1dss);
}

static void arm_smmu_v3_write_ste_test_bypass_to_abort(struct kunit *test)
{
	/*
	 * Bypass STEs has used bits in the first two Qwords, while abort STEs
	 * only have used bits in the first QWord. Transitioning from bypass to
	 * abort requires two syncs: the first to set the first qword and make
	 * the STE into an abort, the second to clean up the second qword.
	 */
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &bypass_ste, &abort_ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_abort_to_bypass(struct kunit *test)
{
	/*
	 * Transitioning from abort to bypass also requires two syncs: the first
	 * to set the second qword data required by the bypass STE, and the
	 * second to set the first qword and switch to bypass.
	 */
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &abort_ste, &bypass_ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_cdtable_to_abort(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &ste, &abort_ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_abort_to_cdtable(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &abort_ste, &ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_cdtable_to_bypass(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &ste, &bypass_ste,
		/*num_syncs_expected=*/3);
}

static void arm_smmu_v3_write_ste_test_bypass_to_cdtable(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &bypass_ste, &ste,
		/*num_syncs_expected=*/3);
}

static void arm_smmu_v3_write_ste_test_cdtable_s1dss_change(struct kunit *test)
{
	struct arm_smmu_ste ste;
	struct arm_smmu_ste s1dss_bypass;

	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_test_make_cdtable_ste(&s1dss_bypass, STRTAB_STE_1_S1DSS_BYPASS,
				       fake_cdtab_dma_addr);

	/*
	 * Flipping s1dss on a CD table STE only involves changes to the second
	 * qword of an STE and can be done in a single write.
	 */
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &ste, &s1dss_bypass,
		/*num_syncs_expected=*/1);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &s1dss_bypass, &ste,
		/*num_syncs_expected=*/1);
}

static void
arm_smmu_v3_write_ste_test_s1dssbypass_to_stebypass(struct kunit *test)
{
	struct arm_smmu_ste s1dss_bypass;

	arm_smmu_test_make_cdtable_ste(&s1dss_bypass, STRTAB_STE_1_S1DSS_BYPASS,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &s1dss_bypass, &bypass_ste,
		/*num_syncs_expected=*/2);
}

static void
arm_smmu_v3_write_ste_test_stebypass_to_s1dssbypass(struct kunit *test)
{
	struct arm_smmu_ste s1dss_bypass;

	arm_smmu_test_make_cdtable_ste(&s1dss_bypass, STRTAB_STE_1_S1DSS_BYPASS,
				       fake_cdtab_dma_addr);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &bypass_ste, &s1dss_bypass,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_test_make_s2_ste(struct arm_smmu_ste *ste,
				      bool ats_enabled)
{
	struct arm_smmu_device smmu = { .features = ARM_SMMU_FEAT_STALLS };
	struct arm_smmu_master master = {
		.smmu = &smmu,
	};
	struct io_pgtable io_pgtable = {};
	struct arm_smmu_domain smmu_domain = {
		.pgtbl_ops = &io_pgtable.ops,
	};

	io_pgtable.cfg.arm_lpae_s2_cfg.vttbr = 0xdaedbeefdeadbeefULL;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.ps = 1;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.tg = 2;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.sh = 3;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.orgn = 1;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.irgn = 2;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.sl = 3;
	io_pgtable.cfg.arm_lpae_s2_cfg.vtcr.tsz = 4;

	arm_smmu_make_s2_domain_ste(ste, &master, &smmu_domain, ats_enabled);
}

static void arm_smmu_v3_write_ste_test_s2_to_abort(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_s2_ste(&ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &ste, &abort_ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_abort_to_s2(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_s2_ste(&ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &abort_ste, &ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_s2_to_bypass(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_s2_ste(&ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &ste, &bypass_ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_bypass_to_s2(struct kunit *test)
{
	struct arm_smmu_ste ste;

	arm_smmu_test_make_s2_ste(&ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &bypass_ste, &ste,
		/*num_syncs_expected=*/2);
}

static void arm_smmu_v3_write_ste_test_s1_to_s2(struct kunit *test)
{
	struct arm_smmu_ste s1_ste;
	struct arm_smmu_ste s2_ste;

	arm_smmu_test_make_cdtable_ste(&s1_ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_test_make_s2_ste(&s2_ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &s1_ste, &s2_ste,
		/*num_syncs_expected=*/3);
}

static void arm_smmu_v3_write_ste_test_s2_to_s1(struct kunit *test)
{
	struct arm_smmu_ste s1_ste;
	struct arm_smmu_ste s2_ste;

	arm_smmu_test_make_cdtable_ste(&s1_ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_test_make_s2_ste(&s2_ste, true);
	arm_smmu_v3_test_ste_expect_hitless_transition(
		test, &s2_ste, &s1_ste,
		/*num_syncs_expected=*/3);
}

static void arm_smmu_v3_write_ste_test_non_hitless(struct kunit *test)
{
	struct arm_smmu_ste ste;
	struct arm_smmu_ste ste_2;

	/*
	 * Although no flow resembles this in practice, one way to force an STE
	 * update to be non-hitless is to change its CD table pointer as well as
	 * s1 dss field in the same update.
	 */
	arm_smmu_test_make_cdtable_ste(&ste, STRTAB_STE_1_S1DSS_SSID0,
				       fake_cdtab_dma_addr);
	arm_smmu_test_make_cdtable_ste(&ste_2, STRTAB_STE_1_S1DSS_BYPASS,
				       0x4B4B4b4B4B);
	arm_smmu_v3_test_ste_expect_non_hitless_transition(
		test, &ste, &ste_2,
		/*num_syncs_expected=*/3);
}

static struct kunit_case arm_smmu_v3_test_cases[] = {
	KUNIT_CASE(arm_smmu_v3_write_ste_test_bypass_to_abort),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_abort_to_bypass),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_cdtable_to_abort),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_abort_to_cdtable),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_cdtable_to_bypass),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_bypass_to_cdtable),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_cdtable_s1dss_change),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_s1dssbypass_to_stebypass),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_stebypass_to_s1dssbypass),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_s2_to_abort),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_abort_to_s2),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_s2_to_bypass),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_bypass_to_s2),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_s1_to_s2),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_s2_to_s1),
	KUNIT_CASE(arm_smmu_v3_write_ste_test_non_hitless),
	{},
};

static int arm_smmu_v3_test_suite_init(struct kunit_suite *test)
{
	arm_smmu_make_bypass_ste(&bypass_ste);
	arm_smmu_make_abort_ste(&abort_ste);
	return 0;
}

static struct kunit_suite arm_smmu_v3_test_module = {
	.name = "arm-smmu-v3-kunit-test",
	.suite_init = arm_smmu_v3_test_suite_init,
	.test_cases = arm_smmu_v3_test_cases,
};
kunit_test_suites(&arm_smmu_v3_test_module);
