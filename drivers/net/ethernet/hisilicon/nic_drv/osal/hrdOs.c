/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfT4UoFQvvVFTaQjxGmtlgjwdpbfmqxlhjKyo8pLHEeKcKPEb/nalwStXYIUaLGX7wvun
vNiCv3rRAq1VG1q9p6brNYMa1fWKIZcfL6Tq+eb1m8Cn14AkV5f08GXrZo0uaA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"


/* Mutexes */
typedef struct
{
    int             free;

#ifdef HRD_OS_LINUX
    struct mutex    struMutex;
#elif defined (HRD_OS_SRE)
    UINT64          splLock;
#endif
}OS_mut_sem_record_t;

/* spinlock */
typedef struct
{
    int             free;
#ifdef HRD_OS_LINUX
    spinlock_t      spinlock;
#elif defined (HRD_OS_SRE)
    UINT64          spinlock;
    UINT32          irqSave;
#endif
}OS_spinlock_record_t;


OS_mut_sem_record_t OS_mut_sem_table[OS_MAX_MUTEXES];

OS_spinlock_record_t OS_spinlock_table[OS_MAX_SPINLOCKS];


/*---------------------------------------------------------------------------------------
   Name: OS_API_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: OS_SUCCESS or OS_ERROR
---------------------------------------------------------------------------------------*/
HRD_STATUS OS_API_Init(void)
{
    int                 i;

    /* Initialize Mutex Semaphore Table */
    for(i = 0; i < OS_MAX_MUTEXES; i++)
    {
        OS_mut_sem_table[i].free        = TRUE;
    }

    for ( i = 0 ; i < OS_MAX_SPINLOCKS ; i++ )
    {
        OS_spinlock_table[i].free       = TRUE;
    }

    return OS_SUCCESS;

}


#ifdef HRD_OS_LINUX

HRD_STATUS OS_MutexCreate(UINT32 *mutex_id)
{
    UINT32              possible_mutex_id;

    /* Check Parameters */
    if (mutex_id == NULL)
    {
        return OS_INVALID_POINTER;
    }

    for ( possible_mutex_id = 0 ; possible_mutex_id < OS_MAX_MUTEXES ; possible_mutex_id++ )
    {
        if ( OS_mut_sem_table[possible_mutex_id].free == TRUE)
        {
            break;
        }
    }

    if( (possible_mutex_id == OS_MAX_MUTEXES) ||
        (OS_mut_sem_table[possible_mutex_id].free != TRUE) )
    {
        return OS_ERR_NO_FREE_IDS;
    }


    /* Set the free flag to false to make sure no other task grabs it */
    OS_mut_sem_table[possible_mutex_id].free = FALSE;

    mutex_init(&(OS_mut_sem_table[possible_mutex_id].struMutex));

    /*
    ** Mark mutex as initialized
    */
    *mutex_id = possible_mutex_id;

    OS_mut_sem_table[*mutex_id].free = FALSE;

    return OS_SUCCESS;

}

HRD_STATUS OS_MutexLock(UINT32 mutex_id)
{
    /*
    ** Check Parameters
    */
    if(mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    mutex_lock(&(OS_mut_sem_table[mutex_id].struMutex));

    return OS_SUCCESS;

}

HRD_STATUS OS_MutexUnLock(UINT32 mutex_id)
{
    /* Check Parameters */

    if(mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /*
    ** Unlock the mutex
    */
    mutex_unlock(&(OS_mut_sem_table[mutex_id].struMutex));

    return OS_SUCCESS;
}

HRD_STATUS OS_MutexDelete(UINT32 mutex_id)
{
    /* Check to see if this sem_id is valid   */
    if (mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Delete its presence in the table */
    OS_mut_sem_table[mutex_id].free = TRUE;

    return OS_SUCCESS;

}


/* SPIN LOCK API */
HRD_STATUS OS_SpinLockCreate(UINT32 *lock_Id)
{
    UINT32              possible_lock_id;

    /* Check Parameters */
    if (lock_Id == NULL )
    {
        return OS_INVALID_POINTER;
    }

    for ( possible_lock_id = 0 ; possible_lock_id < OS_MAX_SPINLOCKS ; possible_lock_id++ )
    {
        if ( OS_spinlock_table[possible_lock_id].free == TRUE)
        {
            break;
        }
    }

    if( (possible_lock_id == OS_MAX_SPINLOCKS) ||
        (OS_spinlock_table[possible_lock_id].free != TRUE) )
    {
        return OS_ERR_NO_FREE_IDS;
    }

    spin_lock_init(&(OS_spinlock_table[possible_lock_id].spinlock));

    /*
    ** Mark mutex as initialized
    */
    *lock_Id = possible_lock_id;

    OS_spinlock_table[*lock_Id].free = FALSE;

    return OS_SUCCESS;

}

HRD_STATUS OS_SpinLockIrqSave(UINT32 lock_id, unsigned long flags)
{
    /*
    ** Check Parameters
    */
    if(lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    spin_lock_irqsave(&(OS_spinlock_table[lock_id].spinlock),flags);

    return OS_SUCCESS;

}

HRD_STATUS OS_SpinUnLockIrqRestore(UINT32 lock_id, unsigned long flags)
{
    /*
    ** Check Parameters
    */
    if(lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    spin_unlock_irqrestore(&(OS_spinlock_table[lock_id].spinlock),flags);

    return OS_SUCCESS;

}


HRD_STATUS OS_SpinLockDelete(UINT32 lock_id)
{
    /* Check to see if this sem_id is valid   */
    if (lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Delete its presence in the table */
    OS_spinlock_table[lock_id].free = TRUE;

    return OS_SUCCESS;

}

#elif defined (HRD_OS_SRE) /* HRD_OS_LINUX */

HRD_STATUS OS_MutexCreate(UINT32 *mutex_id)
{
    UINT32              possible_mutex_id;

    /* Check Parameters */
    if (mutex_id == NULL)
    {
        return OS_INVALID_POINTER;
    }

    for ( possible_mutex_id = 0 ; possible_mutex_id < OS_MAX_MUTEXES ; possible_mutex_id++ )
    {
        if ( OS_mut_sem_table[possible_mutex_id].free == TRUE)
        {
            break;
        }
    }

    if( (possible_mutex_id == OS_MAX_MUTEXES) ||
        (OS_mut_sem_table[possible_mutex_id].free != TRUE) )
    {
        return OS_ERR_NO_FREE_IDS;
    }


    /*
    ** Mark mutex as initialized
    */
    *mutex_id = possible_mutex_id;
    OS_mut_sem_table[*mutex_id].free = FALSE;

    return OS_SUCCESS;

}

HRD_STATUS OS_MutexLock(UINT32 mutex_id)
{
    /*
    ** Check Parameters
    */
    if(mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    SRE_SplLock(&(OS_mut_sem_table[mutex_id].splLock));

    return OS_SUCCESS;

}

HRD_STATUS OS_MutexUnLock(UINT32 mutex_id)
{
    /* Check Parameters */

    if(mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /*
    ** Unlock the mutex
    */
    SRE_SplUnlock(&(OS_mut_sem_table[mutex_id].splLock));

    return OS_SUCCESS;
}

HRD_STATUS OS_MutexDelete(UINT32 mutex_id)
{
    /* Check to see if this sem_id is valid   */
    if (mutex_id >= OS_MAX_MUTEXES || OS_mut_sem_table[mutex_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Delete its presence in the table */
    OS_mut_sem_table[mutex_id].free = TRUE;

    return OS_SUCCESS;

}


/* SPIN LOCK API */
HRD_STATUS OS_SpinLockCreate(UINT32 *lock_Id)
{
    UINT32              possible_lock_id;

    /* Check Parameters */
    if (lock_Id == NULL )
    {
        return OS_INVALID_POINTER;
    }

    for ( possible_lock_id = 0 ; possible_lock_id < OS_MAX_SPINLOCKS ; possible_lock_id++ )
    {
        if ( OS_spinlock_table[possible_lock_id].free == TRUE)
        {
            break;
        }
    }

    if( (possible_lock_id == OS_MAX_SPINLOCKS) ||
        (OS_spinlock_table[possible_lock_id].free != TRUE) )
    {
        return OS_ERR_NO_FREE_IDS;
    }

    /*
    ** Mark mutex as initialized
    */
    *lock_Id = possible_lock_id;

    OS_spinlock_table[*lock_Id].free = FALSE;

    return OS_SUCCESS;

}


HRD_STATUS OS_SpinLockIrqSave(UINT32 lock_id, unsigned long flags)
{
    /*
    ** Check Parameters
    */
    if(lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    OS_spinlock_table[lock_id].irqSave = SRE_SplIrqLock(&(OS_spinlock_table[lock_id].spinlock));

    return OS_SUCCESS;

}


HRD_STATUS OS_SpinUnLockIrqRestore(UINT32 lock_id, unsigned long flags)
{
    /*
    ** Check Parameters
    */
    if(lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
       return OS_ERR_INVALID_ID;
    }

    /*
    ** Lock the mutex - unlike the sem calls, the pthread mutex call
    ** should not be interrupted by a signal
    */
    SRE_SplIrqUnlock(&(OS_spinlock_table[lock_id].spinlock),OS_spinlock_table[lock_id].irqSave);

    return OS_SUCCESS;

}



HRD_STATUS OS_SpinLockDelete(UINT32 lock_id)
{
    /* Check to see if this sem_id is valid   */
    if (lock_id >= OS_MAX_SPINLOCKS || OS_spinlock_table[lock_id].free == TRUE)
    {
        return OS_ERR_INVALID_ID;
    }

    /* Delete its presence in the table */
    OS_spinlock_table[lock_id].free = TRUE;

    return OS_SUCCESS;

}

#else /* HRD_OS_SRE */

#error HRD_OS was not defined!

#endif


