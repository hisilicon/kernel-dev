.. ioasid:

=====================================
IO Address Space ID
=====================================

IOASID is a generic name for PCIe Process Address ID (PASID) or ARM
SMMU sub-stream ID. An IOASID identifies an address space that DMA
requests can target.

The primary use cases for IOASID are Shared Virtual Address (SVA) and
IO Virtual Address (IOVA). However, the requirements for IOASID
management can vary among hardware architectures.

This document covers the generic features supported by IOASID
APIs. Vendor-specific use cases are also illustrated with Intel's VT-d
based platforms as the first example.

.. contents:: :local:

Glossary
========
PASID - Process Address Space ID

IOASID - IO Address Space ID (generic term for PCIe PASID and
sub-stream ID in SMMU)

SVA/SVM - Shared Virtual Addressing/Memory

ENQCMD - New Intel X86 ISA for efficient workqueue submission [1]

DSA - Intel Data Streaming Accelerator [2]

VDCM - Virtual device composition module [3]

SIOV - Intel Scalable IO Virtualization


Key Concepts
============

IOASID Set
-----------
An IOASID set is a group of IOASIDs allocated from the system-wide
IOASID pool. An IOASID set is created and can be identified by a
token of u64. Refer to IOASID set APIs for more details.

IOASID set is particularly useful for guest SVA where each guest could
have its own IOASID set for security and efficiency reasons.

IOASID Set Private ID (SPID)
----------------------------
SPIDs are introduced as IOASIDs within its set. Each SPID maps to a
system-wide IOASID but the namespace of SPID is within its IOASID
set. SPIDs can be used as guest IOASIDs where each guest could do
IOASID allocation from its own pool and map them to host physical
IOASIDs. SPIDs are particularly useful for supporting live migration
where decoupling guest and host physical resources are necessary.

For example, two VMs can both allocate guest PASID/SPID #101 but map to
different host PASIDs #201 and #202 respectively as shown in the
diagram below.
::

 .------------------.    .------------------.
 |   VM 1           |    |   VM 2           |
 |                  |    |                  |
 |------------------|    |------------------|
 | GPASID/SPID 101  |    | GPASID/SPID 101  |
 '------------------'    -------------------'     Guest
 __________|______________________|______________________
           |                      |               Host
           v                      v
 .------------------.    .------------------.
 | Host IOASID 201  |    | Host IOASID 202  |
 '------------------'    '------------------'
 |   IOASID set 1   |    |   IOASID set 2   |
 '------------------'    '------------------'

Guest PASID is treated as IOASID set private ID (SPID) within an
IOASID set, mappings between guest and host IOASIDs are stored in the
set for inquiry.

IOASID APIs
===========
To get the IOASID APIs, users must #include <linux/ioasid.h>. These APIs
serve the following functionalities:

  - IOASID allocation/Free
  - Group management in the form of ioasid_set
  - Private data storage and lookup
  - Reference counting
  - Event notification in case of state change

IOASID Set Level APIs
--------------------------
For use cases such as guest SVA it is necessary to manage IOASIDs at
a group level. For example, VMs may allocate multiple IOASIDs for
guest process address sharing (vSVA). It is imperative to enforce
VM-IOASID ownership such that malicious guest cannot target DMA
traffic outside its own IOASIDs, or free an active IOASID belong to
another VM.
::

 struct ioasid_set *ioasid_alloc_set(void *token, ioasid_t quota, u32 type)

 int ioasid_adjust_set(struct ioasid_set *set, int quota);

 void ioasid_set_get(struct ioasid_set *set)

 void ioasid_set_put(struct ioasid_set *set)

 void ioasid_set_get_locked(struct ioasid_set *set)

 void ioasid_set_put_locked(struct ioasid_set *set)

 int ioasid_set_for_each_ioasid(struct ioasid_set *sdata,
                                void (*fn)(ioasid_t id, void *data),
				void *data)


IOASID set concept is introduced to represent such IOASID groups. Each
IOASID set is created with a token which can be one of the following
types:

 - IOASID_SET_TYPE_NULL (Arbitrary u64 value)
 - IOASID_SET_TYPE_MM (Set token is a mm_struct)

The explicit MM token type is useful when multiple users of an IOASID
set under the same process need to communicate about their shared IOASIDs.
E.g. An IOASID set created by VFIO for one guest can be associated
with the KVM instance for the same guest since they share a common mm_struct.

The IOASID set APIs serve the following purposes:

 - Ownership/permission enforcement
 - Take collective actions, e.g. free an entire set
 - Event notifications within a set
 - Look up a set based on token
 - Quota enforcement

Individual IOASID APIs
----------------------
Once an ioasid_set is created, IOASIDs can be allocated from the set.
Within the IOASID set namespace, set private ID (SPID) is supported. In
the VM use case, SPID can be used for storing guest PASID.

::

 ioasid_t ioasid_alloc(struct ioasid_set *set, ioasid_t min, ioasid_t max,
                       void *private);

 int ioasid_get(struct ioasid_set *set, ioasid_t ioasid);

 void ioasid_put(struct ioasid_set *set, ioasid_t ioasid);

 int ioasid_get_locked(struct ioasid_set *set, ioasid_t ioasid);

 void ioasid_put_locked(struct ioasid_set *set, ioasid_t ioasid);

 void *ioasid_find(struct ioasid_set *set, ioasid_t ioasid,
                   bool (*getter)(void *));

 ioasid_t ioasid_find_by_spid(struct ioasid_set *set, ioasid_t spid)

 int ioasid_attach_data(struct ioasid_set *set, ioasid_t ioasid,
                        void *data);
 int ioasid_attach_spid(struct ioasid_set *set, ioasid_t ioasid,
                        ioasid_t ssid);


Notifications
-------------
An IOASID may have multiple users, each user may have hardware context
associated with an IOASID. When the status of an IOASID changes,
e.g. an IOASID is being freed, users need to be notified such that the
associated hardware context can be cleared, flushed, and drained.

::

 int ioasid_register_notifier(struct ioasid_set *set, struct
                              notifier_block *nb)

 void ioasid_unregister_notifier(struct ioasid_set *set,
                                 struct notifier_block *nb)

 int ioasid_register_notifier_mm(struct mm_struct *mm, struct
                                 notifier_block *nb)

 void ioasid_unregister_notifier_mm(struct mm_struct *mm, struct
                                    notifier_block *nb)

 int ioasid_notify(ioasid_t ioasid, enum ioasid_notify_val cmd,
                   unsigned int flags)


Events
~~~~~~
Notification events are pertinent to individual IOASIDs, they can be
one of the following:

 - ALLOC
 - FREE
 - BIND
 - UNBIND

Ordering
~~~~~~~~
Ordering is supported by IOASID notification priorities as the
following (in ascending order):

::

 enum ioasid_notifier_prios {
	IOASID_PRIO_LAST,
	IOASID_PRIO_IOMMU,
	IOASID_PRIO_DEVICE,
	IOASID_PRIO_CPU,
 };

The typical use case is when an IOASID is freed due to an exception, DMA
source should be quiesced before tearing down other hardware contexts
in the system. This will reduce the churn in handling faults. DMA work
submission is performed by the CPU which is granted higher priority than
devices.


Scopes
~~~~~~
There are two types of notifiers in IOASID core: system-wide and
ioasid_set-wide.

System-wide notifier is catering for users that need to handle all
IOASIDs in the system. E.g. The IOMMU driver handles all IOASIDs.

Per ioasid_set notifier can be used by VM specific components such as
KVM. After all, each KVM instance only cares about IOASIDs within its
own set.


Atomicity
~~~~~~~~~
IOASID notifiers are atomic due to spinlocks used inside the IOASID
core. For tasks cannot be completed in the notifier handler, async work
can be submitted to complete the work later as long as there is no
ordering requirement.

Reference counting
------------------
IOASID lifecycle management is based on reference counting. Users of
IOASID intend to align lifecycle with the IOASID need to hold
reference of the IOASID. IOASID will not be returned to the pool for
allocation until all references are dropped. Calling ioasid_free()
will mark the IOASID as FREE_PENDING if the IOASID has outstanding
reference. ioasid_get() is not allowed once an IOASID is in the
FREE_PENDING state.

Event notifications are used to inform users of IOASID status change.
IOASID_FREE event prompts users to drop their references after
clearing its context.

For example, on VT-d platform when an IOASID is freed, teardown
actions are performed on KVM, device driver, and IOMMU driver.
KVM shall register notifier block with::

 static struct notifier_block pasid_nb_kvm = {
	.notifier_call = pasid_status_change_kvm,
	.priority      = IOASID_PRIO_CPU,
 };

VDCM driver shall register notifier block with::

 static struct notifier_block pasid_nb_vdcm = {
	.notifier_call = pasid_status_change_vdcm,
	.priority      = IOASID_PRIO_DEVICE,
 };

In both cases, notifier blocks shall be registered on the IOASID set
such that *only* events from the matching VM is received.

If KVM attempts to register notifier block before the IOASID set is
created for the MM token, the notifier block will be placed on a
pending list inside IOASID core. Once the token matching IOASID set
is created, IOASID will register the notifier block automatically.
IOASID core does not replay events for the existing IOASIDs in the
set. For IOASID set of MM type, notification blocks can be registered
on empty sets only. This is to avoid lost events.

IOMMU driver shall register notifier block on global chain::

 static struct notifier_block pasid_nb_vtd = {
	.notifier_call = pasid_status_change_vtd,
	.priority      = IOASID_PRIO_IOMMU,
 };

Custom allocator APIs
---------------------

::

 int ioasid_register_allocator(struct ioasid_allocator_ops *allocator);

 void ioasid_unregister_allocator(struct ioasid_allocator_ops *allocator);

Allocator Choices
~~~~~~~~~~~~~~~~~
IOASIDs are allocated for both host and guest SVA/IOVA usage. However,
allocators can be different. For example, on VT-d guest PASID
allocation must be performed via a virtual command interface which is
emulated by VMM.

IOASID core has the notion of "custom allocator" such that guest can
register virtual command allocator that precedes the default one.

Namespaces
~~~~~~~~~~
IOASIDs are limited system resources that default to 20 bits in
size. Since each device has its own table, theoretically the namespace
can be per device also. However, for security reasons sharing PASID
tables among devices are not good for isolation. Therefore, IOASID
namespace is system-wide.

There are also other reasons to have this simpler system-wide
namespace. Take VT-d as an example, VT-d supports shared workqueue
and ENQCMD[1] where one IOASID could be used to submit work on
multiple devices that are shared with other VMs. This requires IOASID
to be system-wide. This is also the reason why guests must use an
emulated virtual command interface to allocate IOASID from the host.


Life cycle
==========
This section covers IOASID lifecycle management for both bare-metal
and guest usages. In bare-metal SVA, MMU notifier is directly hooked
up with IOMMU driver, therefore the process address space (MM)
lifecycle is aligned with IOASID.

However, guest MMU notifier is not available to host IOMMU driver,
when guest MM terminates unexpectedly, the events have to go through
VFIO and IOMMU UAPI to reach host IOMMU driver. There are also more
parties involved in guest SVA, e.g. on Intel VT-d platform, IOASIDs
are used by IOMMU driver, KVM, VDCM, and VFIO.

Native IOASID Life Cycle (VT-d Example)
---------------------------------------

The normal flow of native SVA code with Intel Data Streaming
Accelerator(DSA) [2] as example:

1. Host user opens accelerator FD, e.g. DSA driver, or uacce;
2. DSA driver allocate WQ, do sva_bind_device();
3. IOMMU driver calls ioasid_alloc(), then bind PASID with device,
   mmu_notifier_get()
4. DMA starts by DSA driver userspace
5. DSA userspace close FD
6. DSA/uacce kernel driver handles FD.close()
7. DSA driver stops DMA
8. DSA driver calls sva_unbind_device();
9. IOMMU driver does unbind, clears PASID context in IOMMU, flush
   TLBs. mmu_notifier_put() called.
10. mmu_notifier.release() called, IOMMU SVA code calls ioasid_free()*
11. The IOASID is returned to the pool, reclaimed.

::

   * With ENQCMD, PASID used on VT-d is not released in mmu_notifier() but
     mmdrop(). mmdrop comes after FD close. Should not matter.
     If the user process dies unexpectedly, Step #10 may come before
     Step #5, in between, all DMA faults discarded. PRQ responded with
     code INVALID REQUEST.

During the normal teardown, the following three steps would happen in
order:

1. Device driver stops DMA request
2. IOMMU driver unbinds PASID and mm, flush all TLBs, drain in-flight
   requests.
3. IOASID freed

Exception happens when process terminates *before* device driver stops
DMA and call IOMMU driver to unbind. The flow of process exists are as
follows:

::

   do_exit() {
	exit_mm() {
		mm_put();
		exit_mmap() {
			intel_invalidate_range() //mmu notifier
			tlb_finish_mmu()
			mmu_notifier_release(mm) {
				intel_iommu_release() {
   [2]					intel_iommu_teardown_pasid();
                                        intel_iommu_flush_tlbs();
				}
				// tlb_invalidate_range cb removed
			}
			unmap_vmas();
                        free_pgtables(); // IOMMU cannot walk PGT after this
		};
	}
	exit_files(tsk) {
		close_files() {
			dsa_close();
   [1]			dsa_stop_dma();
                        intel_svm_unbind_pasid(); //nothing to do
		}
	}
   }

   mmdrop() /* some random time later, lazy mm user */ {
   	mm_free_pgd();
        destroy_context(mm); {
   [3]	        ioasid_free();
	}
   }

As shown in the list above, step #2 could happen before
#1. Unrecoverable(UR) faults could happen between #2 and #1.

Also notice that TLB invalidation occurs at mmu_notifier
invalidate_range callback as well as the release callback. The reason
is that release callback will delete IOMMU driver from the notifier
chain which may skip invalidate_range() calls during the exit path.

To avoid unnecessary reporting of UR fault, IOMMU driver shall disable
fault reporting after free and before unbind.

Guest IOASID Life Cycle (VT-d Example)
--------------------------------------
Guest IOASID life cycle starts with guest driver open(), this could be
uacce or individual accelerator driver such as DSA. At FD open,
sva_bind_device() is called which triggers a series of actions.

The example below is an illustration of *normal* operations that
involves *all* the SW components in VT-d. The flow can be simpler if
no ENQCMD is supported.

::

     VFIO        IOMMU        KVM        VDCM        IOASID       Ref
   ..................................................................
   1             ioasid_register_notifier/_mm()
   2 ioasid_alloc()                                               1
   3 bind_gpasid()
   4             iommu_bind()->ioasid_get()                       2
   5             ioasid_notify(BIND)
   6                          -> ioasid_get()                     3
   7                          -> vmcs_update_atomic()
   8 mdev_write(gpasid)
   9                                    hpasid=
   10                                   find_by_spid(gpasid)      4
   11                                   vdev_write(hpasid)
   12 -------- GUEST STARTS DMA --------------------------
   13 -------- GUEST STOPS DMA --------------------------
   14 mdev_clear(gpasid)
   15                                   vdev_clear(hpasid)
   16                                   ioasid_put()               3
   17 unbind_gpasid()
   18            iommu_ubind()
   19            ioasid_notify(UNBIND)
   20                          -> vmcs_update_atomic()
   21                          -> ioasid_put()                     2
   22 ioasid_free()                                                1
   23            ioasid_put()                                      0
   24                                                 Reclaimed
   -------------- New Life Cycle Begin ----------------------------
   1  ioasid_alloc()                                  ->           1

   Note: IOASID Notification Events: FREE, BIND, UNBIND

Exception cases arise when a guest crashes or a malicious guest
attempts to cause disruption on the host system. The fault handling
rules are:

1. IOASID free must *always* succeed.
2. An inactive period may be required before the freed IOASID is
   reclaimed. During this period, consumers of IOASID perform cleanup.
3. Malfunction is limited to the guest owned resources for all
   programming errors.

The primary source of exception is when the following are out of
order:

1. Start/Stop of DMA activity
   (Guest device driver, mdev via VFIO)
2. Setup/Teardown of IOMMU PASID context, IOTLB, DevTLB flushes
   (Host IOMMU driver bind/unbind)
3. Setup/Teardown of VMCS PASID translation table entries (KVM) in
   case of ENQCMD
4. Programming/Clearing host PASID in VDCM (Host VDCM driver)
5. IOASID alloc/free (Host IOASID)

VFIO is the *only* user-kernel interface, which is ultimately
responsible for exception handlings.

#1 is processed the same way as the assigned device today based on
device file descriptors and events. There is no special handling.

#3 is based on bind/unbind events emitted by #2.

#4 is naturally aligned with IOASID life cycle in that an illegal
guest PASID programming would fail in obtaining reference of the
matching host IOASID.

#5 is similar to #4. The fault will be reported to the user if PASID
used in the ENQCMD is not set up in VMCS PASID translation table.

Therefore, the remaining out of order problem is between #2 and
#5. I.e. unbind vs. free. More specifically, free before unbind.

IOASID notifier and refcounting are used to ensure order. Following
a publisher-subscriber pattern where:

- Publishers: VFIO & IOMMU
- Subscribers: KVM, VDCM, IOMMU

IOASID notifier is atomic which requires subscribers to do quick
handling of the event in the atomic context. Workqueue can be used for
any processing that requires thread context. IOASID reference must be
acquired before receiving the FREE event. The reference must be
dropped at the end of the processing in order to return the IOASID to
the pool.

Let's examine the IOASID life cycle again when free happens *before*
unbind. This could be a result of misbehaving guests or crash. Assuming
VFIO cannot enforce unbind->free order. Notice that the setup part up
until step #12 is identical to the normal case, the flow below starts
with step 13.

::

     VFIO        IOMMU        KVM        VDCM        IOASID       Ref
   ..................................................................
   13 -------- GUEST STARTS DMA --------------------------
   14 -------- *GUEST MISBEHAVES!!!* ----------------
   15 ioasid_free()
   16                                             ioasid_notify(FREE)
   17                                             mark_ioasid_inactive[1]
   18                          kvm_nb_handler(FREE)
   19                          vmcs_update_atomic()
   20                          ioasid_put_locked()   ->           3
   21                                   vdcm_nb_handler(FREE)
   22            iomm_nb_handler(FREE)
   23 ioasid_free() returns[2]          schedule_work()           2
   24            schedule_work()        vdev_clear_wk(hpasid)
   25            teardown_pasid_wk()
   26                                   ioasid_put() ->           1
   27            ioasid_put()                                     0
   28                                                 Reclaimed
   29 unbind_gpasid()
   30            iommu_unbind()->ioasid_find() Fails[3]
   -------------- New Life Cycle Begin ----------------------------

Note:

1. By marking IOASID inactive at step #17, no new references can be
   held. ioasid_get/find() will return -ENOENT;
2. After step #23, all events can go out of order. Shall not affect
   the outcome.
3. IOMMU driver fails to find private data for unbinding. If unbind is
   called after the same IOASID is allocated for the same guest again,
   this is a programming error. The damage is limited to the guest
   itself since unbind performs permission checking based on the
   IOASID set associated with the guest process.

KVM PASID Translation Table Updates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Per VM PASID translation table is maintained by KVM in order to
support ENQCMD in the guest. The table contains host-guest PASID
translations to be consumed by CPU ucode. The synchronization of the
PASID states depends on VFIO/IOMMU driver, where IOCTL and atomic
notifiers are used. KVM must register IOASID notifier per VM instance
during launch time. The following events are handled:

1. BIND/UNBIND
2. FREE

Rules:

1. Multiple devices can bind with the same PASID, this can be different PCI
   devices or mdevs within the same PCI device. However, only the
   *first* BIND and *last* UNBIND emit notifications.
2. IOASID code is responsible for ensuring the correctness of H-G
   PASID mapping. There is no need for KVM to validate the
   notification data.
3. When UNBIND happens *after* FREE, KVM will see error in
   ioasid_get() even when the reclaim is not done. IOMMU driver will
   also avoid sending UNBIND if the PASID is already FREE.
4. When KVM terminates *before* FREE & UNBIND, references will be
   dropped for all host PASIDs.

VDCM PASID Programming
~~~~~~~~~~~~~~~~~~~~~~
VDCM composes virtual devices and exposes them to the guests. When
the guest allocates a PASID then program it to the virtual device, VDCM
intercepts the programming attempt then program the matching host
PASID on to the hardware.
Conversely, when a device is going away, VDCM must be informed such
that PASID context on the hardware can be cleared. There could be
multiple mdevs assigned to different guests in the same VDCM. Since
the PASID table is shared at PCI device level, lazy clearing is not
secure. A malicious guest can attack by using newly freed PASIDs that
are allocated by another guest.

By holding a reference of the PASID until VDCM cleans up the HW context,
it is guaranteed that PASID life cycles do not cross within the same
device.


Reference
====================================================
1. https://software.intel.com/sites/default/files/managed/c5/15/architecture-instruction-set-extensions-programming-reference.pdf

2. https://01.org/blogs/2019/introducing-intel-data-streaming-accelerator

3. https://software.intel.com/en-us/download/intel-data-streaming-accelerator-preliminary-architecture-specification
