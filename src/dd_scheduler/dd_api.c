/*
 * dd_api.c
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"
#include "dd_api.h"
#include "dd_sched.h"

/**************************************************************************************
* DESC: This function initializes the message header used to communicate with 
*       the DDS scheduler. 
* RETURNS: None
*/
void _DDS_InitializeMsg(dds_Message_t *     pMsg, 
                        void *              pMsgData,
                        dds_CommandType_e   commandID)
{
    pMsg->dds_CID            = commandID;
    pMsg->pPayload              = pMsgData;
    pMsg->dds_ReturnVal      = 0;
    pMsg->xReturnEvt         = xEventGroupCreate();
}


/**************************************************************************************
* DESC: This function send a message to the DDS and waits on an event for a the DDS to 
*       to signal a response. 
* RETURNS:  ddsSUCCESS: if function successfull, otherwise ddsFAILURE
*/
uint32_t _DDS_SendAndWait(dds_Message_t *  pMsg)
{
    uint32_t result = ddsSUCCESS;

    // ==> Push message into the queue for DDS to process
    if (pdTRUE = xQueueSend(xQ_DDSCommandQ, pMsg, MAX_WAIT))
    {
        // ==> Signal to the DDS scheduler that there is a message waiting
        xEventGroupSetBits(xEVT_DDScheduler, DDS_MESSAGE);

        // ==> Wait for the DDS scheduler to respond to the message
        if (ddsFAILURE == xEventGroupWaitBits(pMsg->xReturnEvt, 
                            ddsSUCCESS,
                            pdTRUE,
                            pdFALSE,
                            MAX_WAIT))
        {
            result = ddsFAILURE; 
        }

        // ==> Delete the event group after using to free memory
        vEventGroupDelete(pMsg->xReturnEvt);
    }
    else
    {
        result =  ddsFAILURE;
    }

    return result;
}


/**************************************************************************************
* DESC: This function receives all of the information necessary to create a new dd_task 
*       struct (excluding the release time and completion time). The struct is packaged 
*       as a message and sent to a queue for the DDS to receive.
* RETURNS: ddsSUCCESS: if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_CreateTask(TaskHandle_t    taskHandle,
                        dds_TaskType_e  taskType,
                        uint32_t        taskId,
                        uint32_t        deadline)
{
    uint32_t result     = ddsSUCCESS;

    dds_Message_t   msg;
    dds_CreateMsg_t msgData;
    msgData.deadline    = deadline;
    msgData.taskHandle  = taskHandle;
    msgData.taskType    = taskType;
    msgData.taskId      = taskId; 

    _DDS_InitializeMsg(&msg, &msgData, DDCMD_CREATE);

    // ==> Send and wait for a response from the DDS
    result = _DDS_SendAndWait(&msg);

    return result;
}


/**************************************************************************************
* DESC: This function receives all of the information necessary to delete a dd_task 
* RETURNS: ddsSUCCESS: if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_DeleteTask(uint32_t taskId)
{
    uint32_t      result = ddsSUCCESS;
    dds_Message_t msg;

    _DDS_InitializeMsg(&msg, &taskId, DDCMD_DELETE);

    // ==> Send and wait for a response from the DDS
    result = _DDS_SendAndWait(&msg);
    
    return result;
}



/**************************************************************************************
* DESC: This function receives all of the information necessary to create a new dd_task 
*       struct (excluding the release time and completion time). The struct is packaged 
*       as a message and sent to a queue for the DDS to receive.
* RETURNS: ddsSUCCESS: if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_ReleaseTask(uint32_t taskId)
{
    uint32_t result = ddsSUCCESS;
    dds_Message_t   msg;

    _DDS_InitializeMsg(&msg, &taskId, DDCMD_RELEASE);

    // ==> Send and wait for a response from the DDS
    result = _DDS_SendAndWait(&msg);

    return result;
}

/**************************************************************************************
* DESC: This function receivesthe ID of the DD-Task which has completed its execution. 
*       The ID is packaged as a message and sent to a queue for the DDS to receive.
* RETURNS: ddsSUCCESS:if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_CompleteTask(uint32_t taskId)
{
    uint32_t result = ddsSUCCESS;
    dds_Message_t   msg;

    _DDS_InitializeMsg(&msg, &taskId, DDCMD_COMPLETE);

    // ==> Send and wait for a response from the DDS
    result = _DDS_SendAndWait(&msg);

    return result;
}

/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Active Task List from the
*        DDS. Once a response is received from the DDS, the function returns the list.
* RETURNS: ddsSUCCESS:if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_GetActiveTasks()
{
    uint32_t result = ddsSUCCESS;

    return ddsSUCCESS;
}

/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Completed Task List from 
*       the DDS. Once a response is received from the DDS, the function returns the list.
* RETURNS: ddsSUCCESS:if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_GetCompletedTasks()
{
    uint32_t result = ddsSUCCESS;

    return ddsSUCCESS;
}


/**************************************************************************************
* DESC: This function sends a message to a queue requesting the Overdue Task List from 
*       the DDS. Once a response is received from the DDS, the function returns the list.
* RETURNS: ddsSUCCESS:if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_GetOverdueTasks()
{
    uint32_t result = ddsSUCCESS;

    return ddsSUCCESS;
}


/**************************************************************************************
* DESC: This function Initializes the Deadline Driven Scheduler
* RETURNS: ddsSUCCESS:if function successfull, otherwise ddsFAILURE
*/
uint32_t DDS_Init()
{
    // Create Event Groups for processing commands coming into the scheduler
    xEVT_DDScheduler = xEventGroupCreate();

    // Create queue for messages to the DDS
    xQ_DDSCommandQ = xQueueCreate(DDS_Q_LEN, DDS_Q_ITEMSIZE);

    xTaskCreate(_DDS_Scheduler, 
                "_DDS_Scheduler",
                configMINIMAL_STACK_SIZE, 
                NULL,
                TASK_MAX_PR,
                NULL);

#if DD_ENABLE_MONITOR
    xTaskCreate(_DDS_Monitor,  
                "_DDS_Monitor",
                configMINIMAL_STACK_SIZE,
                NULL,
                TASK_MAX_PR -1,
                NULL);
#endif // DD_ENABLE_MONITOR

    return ddsSUCCESS;
}

