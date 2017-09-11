Architecture V2
===============
The initiative version, so called V1, of *WrapDrive* assumes that multiple
process support of IOMMU would be added by supporting multiple iommu_domain
in the same iommu unit (see [1]_ for detail).

But the current direction of the community is to add iommu_process-es to
iommu_domain, so the iommu_domain can assign multiple ASID/PASID to all
devices in the iommu_group (see [2]_ for detail). So we restructure the design
in V1 to V2 to match the situation  V2.

The iommu_process approach is still on RFC stage, but the *WrapDrive* will
still be valuable without it. For example, it can be used to share the ring
buffer of a NIC to the user space for some application such as ODP. So V2 is
designed to work on both situation.


How does it work
================
In the iommu_process approach, the multiple asid support is bound with the SVM
feature. The iommu_process is allocated only when the process is bound to the
device with VFIO_IOMMU_BIND_PROCESS. So the process space is dedicated to the
device as a whole.

We don't think this is a good idea. Because the SVM performance is still a big
risk to many devices. So we assume the design can be changed soon to:

1. The iommu_process is allocated when the iommu_group is attached
2. The VFIO_IOMMU_BIND_PROCESS is only to dedicate the whole process space
   to the iommu_group

If this is done, the *WrapDrive* can use the VFIO_IOMMU_BIND_PROCESS only for
SVM purpose.

With this approach, there is nothing more to changed. But the mdev framework
still need to be aware of the existence of *WrapDrive* mdev, and use its
parent device/bus to do iommu operation.


References
==========
.. [1] https://zhuanlan.zhihu.com/p/28853405

.. vim: tw=78
