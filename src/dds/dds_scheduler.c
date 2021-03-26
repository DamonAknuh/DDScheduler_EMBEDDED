/*
 * dd_scheduler.c
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"

/**********************************************************************/
// ____ ___ ____ ___ _ ____    ____ ___ ____ _  _ ____ ___ 
// [__   |  |__|  |  | |       [__   |  |__/ |  | |     |  
// ___]  |  |  |  |  | |___    ___]  |  |  \ |__| |___  |  
// 
/**********************************************************************/

EventGroupHandle_t xEVT_DDScheduler;
xQueueHandle       xQ_DDSCommandQ;

dds_TaskHandle_t dds_TaskList[MAX_DDS_TASKS];

dds_TaskHandle_t * dds_ReadyTaskList;
dds_TaskHandle_t * dds_CompletedTaskList;
dds_TaskHandle_t * dds_OverdueTaskList;

/**********************************************************************/
// ___  ____ _ _  _ ____ ___ ____    ____ _  _ _  _ ____  
// |__] |__/ | |  | |__|  |  |___    |___ |  | |\ | |     
// |    |  \ |  \/  |  |  |  |___    |    |__| | \| |___ .
//
/**********************************************************************/

void _DDS_TaskBootStrap(void *pvParameters);
static void _DDS_HandleMSG();
static void _DDS_SortTasksEDF();
static void _DDS_PrioritizeTasks();
static void _DDS_AddTaskCompletedList(dds_TaskHandle_t * pTaskHandle,  
                                    dds_TaskHandle_t ** pplistHead);
static void _DDS_AddTaskToReadyList(dds_TaskHandle_t * pTaskHandle,  
                                    dds_TaskHandle_t ** pplistHead);
static void _DDS_AddTaskToOverdueList(dds_TaskHandle_t * pTaskHandle,  
                                    dds_TaskHandle_t ** pplistHead);
static void _DDS_InitializeTaskNode(uint32_t taskId,
                                    TaskFunction_t taskFunc,
                                    dds_TaskType_e taskType,
                                    uint32_t deadline);

/**********************************************************************/
// ___  ___  ____    ___ ____ ____ _  _ 
// |  \ |  \ [__      |  |__| [__  |_/  
// |__/ |__/ ___]     |  |  | ___] | \_ 
//
/**********************************************************************/
void _DDS_Scheduler(void *pvParameters)
{
    EventBits_t evtBits;

    printf("DDS: Starting Deadline Driven Scheduler\n");

    while(1)
    {
        DBG_LINE("\nDDS: Waiting for Event\n");
        // ==> task waits for the light timer callback to finish and assert next state.
        evtBits = xEventGroupWaitBits(xEVT_DDScheduler,
                    (DDS_MESSAGE | DDS_SORTING | DDS_SCHEDULING),
                    pdTRUE,
                    pdFALSE,
                    MAX_WAIT);

        TIM_Cmd(DDS_STM_TIMER, DISABLE);

        if ( evtBits & DDS_MESSAGE )
        {
            _DDS_HandleMSG();
        }
        if (evtBits & DDS_SORTING)
        {
            _DDS_SortTasksEDF();
        }
        if (evtBits & DDS_SCHEDULING)
        {
            _DDS_PrioritizeTasks();
        }

        TIM_Cmd(DDS_STM_TIMER, ENABLE);
    }
    printf("DDS: Deadline Driven Scheduler Task Ended\n");
}

/**********************************************************************/
// _  _ ____ ____    _  _ ____ _  _ ___  _    ____ ____ 
// |\/| [__  | __    |__| |__| |\ | |  \ |    |___ |__/ 
// |  | ___] |__]    |  | |  | | \| |__/ |___ |___ |  \ 
//
/**********************************************************************/


void _DDS_HandleMSG()
{
    dds_CreateMsg_t * pDataPayload;
    uint32_t          taskId;
    dds_Message_t     rXMessage;
    dds_CommandType_e rXCommandID;

    if (pdTRUE == xQueueReceive(xQ_DDSCommandQ,
                                &(rXMessage),
                                NO_WAIT))
    {
        rXCommandID = rXMessage.dds_CID;
        if (rXCommandID == DDCMD_CREATE)
        {
            DBG_LINE("DDS: CMD: DDCMD_CREATE\n");

            pDataPayload = (dds_CreateMsg_t *)rXMessage.pPayload;

            _DDS_InitializeTaskNode(pDataPayload->taskId,
                                    pDataPayload->taskFunc,
                                    pDataPayload->taskType,
                                    pDataPayload->deadline);
            xEventGroupSetBits(dds_TaskList[pDataPayload->taskId].taskEvt,
                                    READY);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,    (DDS_SORTING | DDS_SCHEDULING));
        }
        else if (rXCommandID == DDCMD_DELETE)
        {
            DBG_LINE("DDS: CMD: DDCMD_DELETE\n");

            taskId =  *(uint32_t *)rXMessage.pPayload;

            dds_TaskList[taskId].tState = DEAD;

            // ==> Delete the Task first, so that deleting the timer, and events
            //      don't cause unforeseen consequences. 
            vTaskDelete(dds_TaskList[taskId].tHandle);

            // ==> Delete the event group after using to free memory
            vEventGroupDelete(dds_TaskList[taskId].taskEvt);
     
            // ==> Delete the Timer group after using to free memory   
            xTimerDelete(dds_TaskList[taskId].TIMHandle, MAX_WAIT); 

            // ==> Trigger only task sorting as delete doesn't change the 
            //      tasks priority order
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SORTING);
        }
        else if (rXCommandID == DDCMD_RELEASE)
        {
            DBG_LINE("DDS: CMD: DDCMD_RELEASE\n");

            taskId =  *(uint32_t *)rXMessage.pPayload;

            dds_TaskList[taskId].tState = READY;
            dds_TaskList[taskId].RTime  = TIM_GetCounter(DDS_STM_TIMER);
            xEventGroupSetBits(dds_TaskList[taskId].taskEvt,
                                    READY);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,  (DDS_SORTING | DDS_SCHEDULING));
        }
        else if (rXCommandID == DDCMD_COMPLETE)
        {
            DBG_LINE("DDS: CMD: DDCMD_COMPLETE\n");

            taskId =  *(uint32_t *)rXMessage.pPayload;

            dds_TaskList[taskId].tState = COMPLETED;
            dds_TaskList[taskId].CTime  = TIM_GetCounter(DDS_STM_TIMER);
            vTaskPrioritySet(dds_TaskList[taskId].tHandle, TASK_LOWEST_PR);


            // ==> Trigger only task sorting as delete doesn't change the 
            //      tasks priority order
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SORTING);
        }
        else if (rXCommandID == DDCMD_GET_ACTIVE)
        {
            DBG_LINE("DDS: CMD: DDCMD_GET_ACTIVE\n");

            // ==> Store the head of the Active tasks in the payload
            rXMessage.pPayload = dds_ReadyTaskList;
        }
        else if (rXCommandID == DDCMD_GET_COMPL)
        {
            DBG_LINE("DDS: CMD: DDCMD_GET_COMPL\n");

            // ==> Store the head of the completed tasks in the payload
            rXMessage.pPayload = dds_CompletedTaskList;
        }
        else if (rXCommandID == DDCMD_GET_OVERDUE)
        {
            DBG_LINE("DDS: CMD: DDCMD_GET_OVERDUE\n");

            // ==> Store the head of the Overdue tasks in the payload
            rXMessage.pPayload = dds_OverdueTaskList;
        }
        
        // ==> Set the event bits to signal that aysnch. call has finished
        xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);
    }
}

/**********************************************************************/
// ___ ____ ____ _  _    ____ ____ ____ ___ 
//  |  |__| [__  |_/     [__  |  | |__/  |  
//  |  |  | ___] | \_    ___] |__| |  \  |  
//
/**********************************************************************/


void _DDS_SortTasksEDF()
{
    DBG_LINE("DDS: Sorting Tasks\n");
    uint8_t taskState; 

    dds_ReadyTaskList        = NULL;
    dds_CompletedTaskList    = NULL;
    dds_OverdueTaskList      = NULL;

    for (uint8_t i = 0; i < MAX_DDS_TASKS; i ++)
    {
        taskState = dds_TaskList[i].tState; 
        if (taskState & COMPLETED)
        {
            _DDS_AddTaskCompletedList(&dds_TaskList[i], &(dds_CompletedTaskList));
        }
        else if (taskState & READY)
        {
            _DDS_AddTaskToReadyList(&dds_TaskList[i], &(dds_ReadyTaskList));
        }
        else if (taskState & OVERDUE)
        {
            _DDS_AddTaskToOverdueList(&dds_TaskList[i], &(dds_OverdueTaskList));
        }
    }
}

/**********************************************************************/
// ___ ____ ____ _  _    ___  ____ _ ____ ____ _ ___ _ ___  ____ 
//  |  |__| [__  |_/     |__] |__/ | |  | |__/ |  |  |   /  |___ 
//  |  |  | ___] | \_    |    |  \ | |__| |  \ |  |  |  /__ |___ 
//
/**********************************************************************/

void _DDS_PrioritizeTasks()
{
    DBG_LINE("DDS: Prioritizing Tasks\n");

    int8_t priority = DDS_MAX_T_PR;
    dds_TaskHandle_t * tempNode;

    if (NULL != dds_OverdueTaskList)
    {
        tempNode = dds_OverdueTaskList;
        while(tempNode != NULL && (priority > 0))
        {
            DBG_VALUE("  DDS: OT%u P%u\n", tempNode->taskId, priority);
            vTaskPrioritySet(tempNode->tHandle, priority);

            tempNode = tempNode->next;
            priority--;
        }

    }

    if(NULL != dds_ReadyTaskList)
    {
        tempNode = dds_ReadyTaskList;

        while(tempNode != NULL && (priority > 0))
        {
            DBG_VALUE("  DDS: RT%u P%u %u\n", tempNode->taskId, priority, configMAX_PRIORITIES);
            vTaskPrioritySet(tempNode->tHandle, priority);
            tempNode = tempNode->next;
            priority--;
        }
    }
}

/**********************************************************************/
// ____ ____ _    _    ___  ____ ____ _  _ ____ 
// |    |__| |    |    |__] |__| |    |_/  [__  
// |___ |  | |___ |___ |__] |  | |___ | \_ ___] 
//
/**********************************************************************/

void TIM_DDS_Period_cb(xTimerHandle xTimer)
{
    uint32_t taskId = (uint32_t )pvTimerGetTimerID(xTimer);

    // ==> Task is either running currently, or is already tagged overdue
    if( dds_TaskList[taskId].tState & (READY | OVERDUE))
    {
        DBG_VALUE("==> TASK %u OD\n", taskId);
        dds_TaskList[taskId].tState = OVERDUE; 
        xEventGroupSetBits(dds_TaskList[taskId].taskEvt,
                                OVERDUE);
        xEventGroupSetBits(xEVT_DDScheduler,  (DDS_SORTING | DDS_SCHEDULING));
    }
    // ==> check if task is periodic and needs to be released again. 
    else if (dds_TaskList[taskId].type == PERIODIC)
    {
        DDS_ReleaseTask(taskId);
    }
}

void _DDS_TaskBootStrap(void *pvParameters)
{
    uint32_t        taskId = (uint32_t)pvParameters;
    EventBits_t     evtBits;
    dds_TaskHandle_t tContext = dds_TaskList[taskId];
    uint8_t repeat = (tContext.type == PERIODIC);

    do
    {
        evtBits = xEventGroupWaitBits(tContext.taskEvt,
                        (READY | OVERDUE),
                        pdTRUE,
                        pdFALSE,
                        MAX_WAIT);
        if(evtBits == DEAD)
        {
            break;
        }
        else if (evtBits & ( READY | OVERDUE))
        {
            DBG_VALUE("TASK: Starting Task: %d\n", taskId);

            tContext.tFunc((void *) taskId);

            DDS_CompleteTask(taskId);
        }
    }
    while(repeat);

    dds_TaskList[taskId].tState = DEAD;

    // ==> Delete the event group after using to free memory
    vEventGroupDelete(dds_TaskList[taskId].taskEvt);
     
    // ==> Delete the Timer group after using to free memory   
    xTimerDelete(dds_TaskList[taskId].TIMHandle, MAX_WAIT); 

    vTaskDelete(NULL);
}

/**********************************************************************/
// _  _ ___ _ _    _ ___ _   _    ____ _  _ _  _ ____ ___ _ ____ _  _ ____ 
// |  |  |  | |    |  |   \_/     |___ |  | |\ | |     |  | |  | |\ | [__  
// |__|  |  | |___ |  |    |      |    |__| | \| |___  |  | |__| | \| ___] 
//
/**********************************************************************/


void _DDS_InitializeTaskNode(uint32_t taskId,
                            TaskFunction_t taskFunc,
                            dds_TaskType_e taskType,
                            uint32_t deadline)
{
    if (taskId <= MAX_DDS_TASKS && dds_TaskList[taskId].tState <= DEAD)
    {
        dds_TaskList[taskId].taskId  = taskId;
        dds_TaskList[taskId].tState  = READY;
        dds_TaskList[taskId].deadline= DDS_MS_2_TICKS(deadline);
        dds_TaskList[taskId].type    = taskType;
        dds_TaskList[taskId].next    = NULL;
        dds_TaskList[taskId].RTime   = TIM_GetCounter(DDS_STM_TIMER);
        dds_TaskList[taskId].CTime   = 0;
        dds_TaskList[taskId].tFunc   = taskFunc;
        dds_TaskList[taskId].taskEvt = xEventGroupCreate();
        xTaskCreate(_DDS_TaskBootStrap,
                    "dd_task",
                    configMINIMAL_STACK_SIZE,
                    (void*) taskId,
                    DD_TASK_START_PR,
                    &(dds_TaskList[taskId].tHandle));

        dds_TaskList[taskId].TIMHandle  = xTimerCreate("dd_t_timer",
                                                pdMS_TO_TICKS(deadline),
                                                (taskType == PERIODIC),
                                                (void *) taskId,
                                                TIM_DDS_Period_cb);

        xTimerStart(dds_TaskList[taskId].TIMHandle, NO_WAIT);
    }
    else
    {
        printf("==> EXCEEDED MAX TASKS NUMBER");
        while(1);
    }
}

void _DDS_AddTaskToReadyList(dds_TaskHandle_t * pTaskHandle,  
                                dds_TaskHandle_t ** pplistHead)
{
    dds_TaskHandle_t * curNode  = (*pplistHead);
    dds_TaskHandle_t * nextNode;
    uint32_t currentCNT = TIM_GetCounter(DDS_STM_TIMER);
    uint32_t priority = (pTaskHandle->RTime + pTaskHandle->deadline) - currentCNT;
    uint32_t nextPriority; 

    pTaskHandle->next = NULL;

    // ==> List is currently empty so add task handle to front. 
    if(curNode == NULL)
    {
        (*pplistHead) = pTaskHandle;
        return;
    }
    
    // ==>  Grab first nodes priority
    nextPriority = (curNode->RTime + curNode->deadline) - currentCNT;

    // ==> Check if need to add at the front of the list. 
    if (priority < nextPriority)
    {
        pTaskHandle->next = curNode;
        (*pplistHead)     = pTaskHandle;
    }
    else
    {
        while(NULL != curNode)
        {
            nextNode = curNode->next;

            // ==> Check to see if this is the end of the list. 
            if (NULL != nextNode)
            {
                // ==> if priority is lower than the next nodes priority
                //      need to insert node halfway into list
                if (priority < nextPriority)
                {
                    pTaskHandle->next = nextNode;
                    curNode->next = pTaskHandle;
                    break;
                }

                curNode = nextNode;
            }
            else
            {
                // ==> If end of list, set last node to taskHandle, and break
                curNode->next = pTaskHandle;
                break;
            }
        }
    }
}


void _DDS_AddTaskToOverdueList(dds_TaskHandle_t * pTaskHandle,  
                                dds_TaskHandle_t ** pplistHead)
{
    dds_TaskHandle_t * curNode  = (*pplistHead);
    dds_TaskHandle_t * nextNode;
    uint32_t currentCNT = TIM_GetCounter(DDS_STM_TIMER);
    uint32_t priority = (pTaskHandle->RTime + pTaskHandle->deadline) - currentCNT;
    uint32_t nextPriority; 

    pTaskHandle->next = NULL;

    // ==> List is currently empty so add task handle to front. 
    if(curNode == NULL)
    {
        (*pplistHead) = pTaskHandle;
        return;
    }
    
    // ==>  Grab first nodes priority
    nextPriority = (curNode->RTime + curNode->deadline) - currentCNT;

    // ==> Check if need to add at the front of the list. 
    if (priority > nextPriority)
    {
        pTaskHandle->next = curNode;
        (*pplistHead)     = pTaskHandle;
    }
    else
    {
        while(NULL != curNode)
        {
            nextNode = curNode->next;

            // ==> Check to see if this is the end of the list. 
            if (NULL != nextNode)
            {
                // ==> if priority is higher than the next nodes priority
                //      need to insert node halfway into list
                if (priority > nextPriority)
                {
                    pTaskHandle->next = nextNode;
                    curNode->next = pTaskHandle;
                    break;
                }

                curNode = nextNode;
            }
            else
            {
                // ==> If end of list, set last node to taskHandle, and break
                curNode->next = pTaskHandle;
                break;
            }
        }
    }
}

void _DDS_AddTaskCompletedList(dds_TaskHandle_t * pTaskHandle,  
                        dds_TaskHandle_t ** pplistHead)
{
    dds_TaskHandle_t * curNode = (*pplistHead);
    pTaskHandle->next = NULL;

    if (curNode != NULL)
    {
        pTaskHandle->next = curNode->next;
    }

    (*pplistHead) = pTaskHandle;
}
