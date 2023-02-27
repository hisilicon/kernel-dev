/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __KVM_VFIO_H
#define __KVM_VFIO_H

#ifdef CONFIG_KVM_VFIO
int kvm_vfio_ops_init(void);
void kvm_vfio_ops_exit(void);
struct kvm *kvm_vfio_get_kvm(struct device *dev);
#else
static inline int kvm_vfio_ops_init(void)
{
	return 0;
}
static inline void kvm_vfio_ops_exit(void)
{
}

static inline struct kvm *kvm_vfio_get_kvm(struct device *dev)
{
	return NULL;
}
#endif

#endif
