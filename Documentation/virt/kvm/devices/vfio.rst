.. SPDX-License-Identifier: GPL-2.0

===================
VFIO virtual device
===================

Device types supported:

  - KVM_DEV_TYPE_VFIO

Only one VFIO instance may be created per VM.  The created device
tracks VFIO files (group or device) in use by the VM and features
of those groups/devices important to the correctness and acceleration
of the VM.  As groups/device are enabled and disabled for use by the
VM, KVM should be updated about their presence.  When registered with
KVM, a reference to the VFIO file is held by KVM.

VFIO Files:
  KVM_DEV_VFIO_FILE

KVM_DEV_VFIO_FILE attributes:
  KVM_DEV_VFIO_FILE_ADD: Add a VFIO file (group/device) to VFIO-KVM device
	tracking kvm_device_attr.addr points to an int32_t file descriptor
	for the VFIO file.
  KVM_DEV_VFIO_FILE_DEL: Remove a VFIO file (group/device) from VFIO-KVM device
	tracking kvm_device_attr.addr points to an int32_t file descriptor
	for the VFIO group.
  KVM_DEV_VFIO_FILE_SET_SPAPR_TCE: attaches a guest visible TCE table
	allocated by sPAPR KVM.
	kvm_device_attr.addr points to a struct::

		struct kvm_vfio_spapr_tce {
			__s32	groupfd;
			__s32	tablefd;
		};

	where:

	- @groupfd is a file descriptor for a VFIO group;
	- @tablefd is a file descriptor for a TCE table allocated via
	  KVM_CREATE_SPAPR_TCE.
