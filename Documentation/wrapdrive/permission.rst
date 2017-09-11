Permission Consideration
========================
The *WrapDrive* is intended to be used by general purpose application. But
*vfio_type_group* is created for root by default. Luckily, it can be changed
with udev rules.

For example: ::

        SUBSYSTEM=="wrapdrive", PROGRAM="/bin/sh -c 'chown -R root:wrapdrive /sys/class/wrapdrive && chmod -R 770 /sys/...<wd dev>.../create && ...<grant params permission>...'"

Then it can be created accordingly.

.. vim: tw=78
