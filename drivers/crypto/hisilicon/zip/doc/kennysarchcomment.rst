Kenneth Lee's comments on the current architecture
=================================================

The QM is the facility for the Accelerator. It provides basic PCIE interface to
the CPU, and setup the protocol between the CPU and the ACC core with queues.

So it is better to leave the QM to discover the device and create VFs in the
same logic. 

From the ACC perspective, it can do it like this: ::

        QM_PF_DEFINITION(match_data, acc_private_data, acc_pf_ops);
        QM_VF_DEFINITION(match_data, acc_private_data, acc_vf_ops);

match_data can be the device id, while the acc_private_data is the data used by the ACC core itself.
and the acc_xx_ops can be expected to be: ::

        h = handle = pre_init(num_vf, para)     //data init before the io space is probed
                                                //vf need no num_vf
        init(h)                                 //init within the io space
        finit()                                 //reversed init and pre_init

        //if the pf have no queue assigned, it call no start
        start(h)                                //start the function
        stop(h)                                 //reversed start

        notify()                                //interrupt event (error or device-ready)

So if we match the PF, we can create VF according to the num_vf parameter. And
then the VFs are discovered and match to the VF definition again. Then we have
start() for register to the other framework, e.g. WrapDrive or crypto.

After we have all these ready, we can start send and receive message with the following interfaces: ::

        qm_send(h, msg)
        qm_recv(h, q_type, buf)        

The some msg format (plain/sql/...) can be defined in qm.h, the acc core add
extra part on it. Any runtime parameter such as SQC parameters can be stored in
the handle. So the qm_sned/recv and have enough information to finish the deal.
