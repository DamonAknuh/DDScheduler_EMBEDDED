/*
 * sys_project.h
 *
 */

#ifndef SYS_PROJECT_H
#define SYS_PROJECT_H

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include "stm32f4_discovery.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"
#include "../FreeRTOS_Source/include/event_groups.h"


//        ENABLE_DEBUG_BUILD == true to enable debug builds, debug builds output further information to LCD.
//        ENABLE_DEBUG_BUILD == false to enable production builds for best performance
#define ENABLE_DEBUG_BUILD      (1)
// LVL 1: CMD IDs
// LVL 2: TASK Priorities || API debug
// LVL 3: Non-essential   || DDS_CMDS
// LVL 4: Values
#define DBG_VERBOSITY			(2)

#if (ENABLE_DEBUG_BUILD)

#define DBG_VALUE(lvl, format, ...)    \
    do                                  \
    {                                   \
        if(lvl <= DBG_VERBOSITY)  printf(format, __VA_ARGS__); \
    }                                   \
    while(0)

#define DBG_LINE(lvl, string)    \
    do                                  \
    {                                   \
    	if(lvl <= DBG_VERBOSITY)  printf(string);\
    }                                   \
    while(0)

#else
#define DBG_VALUE(lvl, format, ...)
#define DBG_LINE(lvl, string)



#endif // ENABLE_DEBUG_BUILD

#endif // SYS_PROJECT_H
