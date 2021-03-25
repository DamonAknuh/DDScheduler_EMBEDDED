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

#if (ENABLE_DEBUG_BUILD)

#define DBG_VALUE(format, ...)    \
    do                                  \
    {                                   \
        printf(format, __VA_ARGS__); \
    }                                   \
    while(0)

#define DBG_LINE(string)    \
    do                                  \
    {                                   \
        printf(string);                    \
    }                                   \
    while(0)

#else

#define DBG_VALUE(format, ...) 
#define DBG_LINE(string) 

#endif // ENABLE_DEBUG_BUILD

#endif // SYS_PROJECT_H
