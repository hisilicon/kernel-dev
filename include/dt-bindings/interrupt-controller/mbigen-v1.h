/*
 * This header provides constants for the ARM GIC.
 */

#ifndef _DT_BINDINGS_INTERRUPT_CONTROLLER_MBIGEN_V1_H
#define _DT_BINDINGS_INTERRUPT_CONTROLLER_MBIGEN_V1_H

/**
 * because of the bug in current ITS driver,
 * this value is 0x10 for now. When ITS driver bug fixed,
 * this value should be changed to 0xffcc
 */
#define MBIGEN_V1_DEVID	0xFFCC

#endif
