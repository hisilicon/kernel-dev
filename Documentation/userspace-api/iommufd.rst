.. SPDX-License-Identifier: GPL-2.0+

=======
IOMMUFD
=======

:Author: Jason Gunthorpe
:Author: Kevin Tian

Overview
========

IOMMUFD is the user API to control the IOMMU subsystem as it relates to managing
IO page tables that point at user space memory. It intends to be general and
consumable by any driver that wants to DMA to userspace. Those drivers are
expected to deprecate any proprietary IOMMU logic, if existing (e.g.
vfio_iommu_type1.c).

At minimum iommufd provides a universal support of managing I/O address spaces
and I/O page tables for all IOMMUs, with room in the design to add non-generic
features to cater to specific hardware functionality.

In this context the capital letter (IOMMUFD) refers to the subsystem while the
small letter (iommufd) refers to the file descriptors created via /dev/iommu to
run the user API over.

Key Concepts
============

User Visible Objects
--------------------

Following IOMMUFD objects are exposed to userspace:

- IOMMUFD_OBJ_IOAS, representing an I/O address space (IOAS) allowing map/unmap
  of user space memory into ranges of I/O Virtual Address (IOVA).

  The IOAS is a functional replacement for the VFIO container, and like the VFIO
  container copies its IOVA map to a list of iommu_domains held within it.

- IOMMUFD_OBJ_DEVICE, representing a device that is bound to iommufd by an
  external driver.

- IOMMUFD_OBJ_HW_PAGETABLE, wrapping an actual hardware I/O page table (i.e. a
  single struct iommu_domain) managed by the iommu driver.

  The IOAS has a list of HW_PAGETABLES that share the same IOVA mapping and the
  IOAS will synchronize its mapping with each member HW_PAGETABLE.

All user-visible objects are destroyed via the IOMMU_DESTROY uAPI.

Linkage between user-visible objects and external kernel datastructures are
reflected by dotted line arrows below, with numbers referring to certain
operations creating the objects and links::

  _________________________________________________________
 |                         iommufd                         |
 |       [1]                                               |
 |  _________________                                      |
 | |                 |                                     |
 | |                 |                                     |
 | |                 |                                     |
 | |                 |                                     |
 | |                 |                                     |
 | |                 |                                     |
 | |                 |        [3]                 [2]      |
 | |                 |    ____________         __________  |
 | |      IOAS       |<--|            |<------|          | |
 | |                 |   |HW_PAGETABLE|       |  DEVICE  | |
 | |                 |   |____________|       |__________| |
 | |                 |         |                   |       |
 | |                 |         |                   |       |
 | |                 |         |                   |       |
 | |                 |         |                   |       |
 | |                 |         |                   |       |
 | |_________________|         |                   |       |
 |         |                   |                   |       |
 |_________|___________________|___________________|_______|
           |                   |                   |
           |              _____v______      _______v_____
           | PFN storage |            |    |             |
           |------------>|iommu_domain|    |struct device|
                         |____________|    |_____________|

1. IOMMUFD_OBJ_IOAS is created via the IOMMU_IOAS_ALLOC uAPI. One iommufd can
   hold multiple IOAS objects. IOAS is the most generic object and does not
   expose interfaces that are specific to single IOMMU drivers. All operations
   on the IOAS must operate equally on each of the iommu_domains that are inside
   it.

2. IOMMUFD_OBJ_DEVICE is created when an external driver calls the IOMMUFD kAPI
   to bind a device to an iommufd. The external driver is expected to implement
   proper uAPI for userspace to initiate the binding operation. Successful
   completion of this operation establishes the desired DMA ownership over the
   device. The external driver must set driver_managed_dma flag and must not
   touch the device until this operation succeeds.

3. IOMMUFD_OBJ_HW_PAGETABLE is created when an external driver calls the IOMMUFD
   kAPI to attach a bound device to an IOAS. Similarly the external driver uAPI
   allows userspace to initiate the attaching operation. If a compatible
   pagetable already exists then it is reused for the attachment. Otherwise a
   new pagetable object (and a new iommu_domain) is created. Successful
   completion of this operation sets up the linkages among an IOAS, a device and
   an iommu_domain. Once this completes the device could do DMA.

   Every iommu_domain inside the IOAS is also represented to userspace as a
   HW_PAGETABLE object.

   NOTE: Future additions to IOMMUFD will provide an API to create and
   manipulate the HW_PAGETABLE directly.

One device can only bind to one iommufd (due to DMA ownership claim) and attach
to at most one IOAS object (no support of PASID yet).

Currently only PCI device is allowed.

Kernel Datastructure
--------------------

User visible objects are backed by following datastructures:

- iommufd_ioas for IOMMUFD_OBJ_IOAS.
- iommufd_device for IOMMUFD_OBJ_DEVICE.
- iommufd_hw_pagetable for IOMMUFD_OBJ_HW_PAGETABLE.

Several terminologies when looking at these datastructures:

- Automatic domain, referring to an iommu domain created automatically when
  attaching a device to an IOAS object. This is compatible to the semantics of
  VFIO type1.

- Manual domain, referring to an iommu domain designated by the user as the
  target pagetable to be attached to by a device. Though currently no user API
  for userspace to directly create such domain, the datastructure and algorithms
  are ready for that usage.

- In-kernel user, referring to something like a VFIO mdev that is accessing the
  IOAS and using a 'struct page \*' for CPU based access. Such users require an
  isolation granularity smaller than what an iommu domain can afford. They must
  manually enforce the IOAS constraints on DMA buffers before those buffers can
  be accessed by mdev. Though no kernel API for an external driver to bind a
  mdev, the datastructure and algorithms are ready for such usage.

iommufd_ioas serves as the metadata datastructure to manage how IOVA ranges are
mapped to memory pages, composed of:

- struct io_pagetable holding the IOVA map
- struct iopt_areas representing populated portions of IOVA
- struct iopt_pages representing the storage of PFNs
- struct iommu_domain representing the IO page table in the IOMMU
- struct iopt_pages_user representing in-kernel users of PFNs
- struct xarray pinned_pfns holding a list of pages pinned by
   in-kernel Users

The iopt_pages is the center of the storage and motion of PFNs. Each iopt_pages
represents a logical linear array of full PFNs. PFNs are stored in a tiered
scheme:

 1) iopt_pages::pinned_pfns xarray
 2) An iommu_domain
 3) The origin of the PFNs, i.e. the userspace pointer

PFN have to be copied between all combinations of tiers, depending on the
configuration (i.e. attached domains and in-kernel users).

An io_pagetable is composed of iopt_areas pointing at iopt_pages, along with a
list of iommu_domains that mirror the IOVA to PFN map.

Multiple io_pagetable's, through their iopt_area's, can share a single
iopt_pages which avoids multi-pinning and double accounting of page consumption.

iommufd_ioas is sharable between subsystems, e.g. VFIO and VDPA, as long as
devices managed by different subsystems are bound to a same iommufd.

IOMMUFD User API
================

.. kernel-doc:: include/uapi/linux/iommufd.h

IOMMUFD Kernel API
==================

The IOMMUFD kAPI is device-centric with group-related tricks managed behind the
scene. This allows the external driver calling such kAPI to implement a simple
device-centric uAPI for connecting its device to an iommufd, instead of
explicitly imposing the group semantics in its uAPI (as VFIO does).

.. kernel-doc:: drivers/iommu/iommufd/device.c
   :export:

VFIO and IOMMUFD
----------------

Connecting VFIO device to iommufd can be done in two approaches.

First is a VFIO compatible way by directly implementing the /dev/vfio/vfio
container IOCTLs by mapping them into io_pagetable operations. Doing so allows
the use of iommufd in legacy VFIO applications by symlinking /dev/vfio/vfio to
/dev/iommufd or extending VFIO to SET_CONTAINER using an iommufd instead of a
container fd.

The second approach directly extends VFIO to support a new set of device-centric
user API based on aforementioned IOMMUFD kernel API. It requires userspace
change but better matches the IOMMUFD API semantics and easier to support new
iommufd features when comparing it to the first approach.

Currently both approaches are still work-in-progress.

There are still a few gaps to be resolved to catch up with VFIO type1, as
documented in iommufd_vfio_check_extension().

Future TODOs
============

Currently IOMMUFD supports only kernel-managed I/O page table, similar to VFIO
type1. New features on the radar include:

 - Binding iommu_domain's to PASID/SSID
 - Userspace page tables, for ARM, x86 and S390
 - Kernel bypass'd invalidation of user page tables
 - Re-use of the KVM page table in the IOMMU
 - Dirty page tracking in the IOMMU
 - Runtime Increase/Decrease of IOPTE size
 - PRI support with faults resolved in userspace
