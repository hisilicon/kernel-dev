/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Arm Ltd. */

#include <asm/mpam.h>

#include <linux/jump_label.h>
#include <linux/percpu.h>

DEFINE_STATIC_KEY_FALSE(arm64_system_has_mpam);
DEFINE_STATIC_KEY_FALSE(arm64_mpam_has_hcr);
DEFINE_PER_CPU(struct mpam_cpu_state, mpam_cpu_state);
