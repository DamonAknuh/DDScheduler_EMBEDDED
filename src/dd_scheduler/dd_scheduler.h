/*
 * dd_scheduler.h
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#ifndef SRC_DD_SCHEDULER_DD_SCHEDULER_H_
#define SRC_DD_SCHEDULER_DD_SCHEDULER_H_

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"

#define DD_ENABLE_MONITOR 		(0)

typedef enum
{
	DDCMD_CREATE,
	DDCMD_RELEASE,
	DDCMD_DELETE,
	DDCMD_GET_ACTIVE,
	DDCMD_GET_COMPL,
	DDCMD_GET_OVERDUE,
} dd_CommandType;

typedef enum
{
	PERIODIC,
	APERIODIC
} dd_TaskType_e;

typedef struct
{
	uint32_t 		dd_TaskId;
	TaskHandle_t 	dd_TaskHandle;
	dd_TaskType_e   dd_TaskType;
	uint32_t 		dd_RelTime;
	uint32_t 		dd_AbsDeadline;
	uint32_t 		dd_CompTime;
	xTimerHandle	dd_TIMGenerator;
} dd_TaskHandle_t;

typedef struct
{
	// Linked List 1
	// LL 2
	// LL 3
	xE

}dd_SchedulerCTX_t;

// API Functions



#endif /* SRC_DD_SCHEDULER_DD_SCHEDULER_H_ */
