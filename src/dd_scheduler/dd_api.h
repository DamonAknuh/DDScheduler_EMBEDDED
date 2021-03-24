/*
 * dd_api.h
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#ifndef SRC_DD_SCHEDULER_DD_API_H_
#define SRC_DD_SCHEDULER_DD_API_H_

#include "dd_scheduler.h"

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
***********************************************************************/

#define ddsSUCCESS (1)
#define ddsFAILURE (0)

/**************************************************************************************
* DESC: This function Initializes the Deadline Driven Scheduler
*/
uint32_t DDS_Init();


/**************************************************************************************
* DESC: This function receives all of the information necessary to create a new dd_task struct (excluding
*       the release time and completion time). The struct is packaged as a message and sent to a queue
*       for the DDS to receive.
*/
uint32_t DDS_CreateTask(TaskHandle_t    taskHandle,
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




#endif /* SRC_DD_SCHEDULER_DD_API_H_ */
