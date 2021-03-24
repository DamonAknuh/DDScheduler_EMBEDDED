/*
 * dds_api.h
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#ifndef DDS_DDS_API_H_
#define DDS_DDS_API_H_

#include "sys_project.h"

typedef enum
{
    PERIODIC = 0,
    APERIODIC,
    UNKNOWN,
} dds_TaskType_e;

#define ddsSUCCESS     (1)
#define ddsFAILURE     (0)

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
***********************************************************************/

/**************************************************************************************
* DESC: This function Initializes the Deadline Driven Scheduler
*/
uint32_t DDS_Init();


/**************************************************************************************
* DESC: This function receives all of the information necessary to create a new dd_task struct (excluding
*       the release time and completion time). The struct is packaged as a message and sent to a queue
*       for the DDS to receive.
*/
uint32_t DDS_CreateTask(TaskFunction_t  taskFunc,
                        dds_TaskType_e  taskType,
                        uint32_t        taskId,
                        uint32_t        deadline);

/**************************************************************************************
* DESC: This function receives all of the information necessary to delete a dd_task
* RETURNS: ddsSUCCESS: if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_DeleteTask(uint32_t taskId);

/**************************************************************************************
* DESC: This function receives all of the information necessary to create a new dd_task struct (excluding
*       the release time and completion time). The struct is packaged as a message and sent to a queue
*       for the DDS to receive.
*/
uint32_t DDS_ReleaseTask(uint32_t taskId);

/**************************************************************************************
* DESC: This function receivesthe ID of the DD-Task which has completed its execution. The ID is packaged
*       as a message and sent to a queue for the DDS to receive.
*/
uint32_t DDS_CompleteTask(uint32_t taskId);

/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Active Task List from the DDS. Once a
*       response is received from the DDS, the function returns the list.
*/
uint32_t DDS_GetActiveTasks();

/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Completed Task List from the DDS. Once
*       a response is received from the DDS, the function returns the list.
*/
uint32_t DDS_GetCompletedTasks();


/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Overdue Task List from the DDS. Once a
*       response is received from the DDS, the function returns the list.
*/
uint32_t DDS_GetOverdueTasks();



#endif /* DDS_DDS_API_H_ */
