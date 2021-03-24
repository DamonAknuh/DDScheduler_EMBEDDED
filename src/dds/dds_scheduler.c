/*
 * dd_scheduler.c
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"

EventGroupHandle_t xEVT_DDScheduler;
xQueueHandle       xQ_DDSCommandQ;

dds_TaskHandle_t dds_TaskList[MAX_DDS_TASKS];

dds_TaskHandle_t * dds_ActiveTaskList;
dds_TaskHandle_t * dds_CompletedTaskList;
dds_TaskHandle_t * dds_OverdueTaskList;
dds_TaskHandle_t * dds_DeadTaskList;

void _DDS_TaskBootStrap(void *pvParameters);
static void _DDS_HandleMSG();
static void _DDS_CalculateSchedule();
static void _DDS_AddTaskToActiveList(uint32_t taskId);
static void _DDS_InitializeTaskNode(uint32_t taskId,
                                    TaskFunction_t taskFunc,
                                    dds_TaskType_e taskType,
                                    uint32_t deadline);

void _DDS_Scheduler(void *pvParameters)
{
    EventBits_t evtBits;

    printf("DDS: Starting Deadline Driven Scheduler\n");

    while(1)
    {
        DBG_LINE("DDS:  Waiting for Event\n");
        // ==> task waits for the light timer callback to finish and assert next state.
        evtBits = xEventGroupWaitBits(xEVT_DDScheduler,
                    DDS_MESSAGE | DDS_SCHEDULING,
                    pdTRUE,
                    pdFALSE,
                    MAX_WAIT);

        if ( evtBits & DDS_MESSAGE )
        {
            _DDS_HandleMSG();
        }
        if (evtBits & DDS_SCHEDULING)
        {
            _DDS_CalculateSchedule();
        }
    }
    printf("DDS: Deadline Driven Scheduler Task Ended\n");
}

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

            _DDS_AddTaskToActiveList(pDataPayload->taskId);

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_DELETE)
        {
            DBG_LINE("DDS: CMD: DDCMD_DELETE\n");

            taskId =  *(uint32_t *)rXMessage.pPayload;

            dds_TaskList[taskId].tState = DEAD;

            // ==> Set the event bits to signal that async. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_RELEASE)
        {
            DBG_LINE("DDS: CMD: DDCMD_RELEASE\n");

            taskId =  *(uint32_t *)rXMessage.pPayload;

            dds_TaskList[taskId].tState = READY;

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_COMPLETE)
        {
            DBG_LINE("DDS: CMD: DDCMD_COMPLETE");

            dds_TaskList[taskId].tState = COMPLETE;

            // TODO dds_TaskList[taskId].CTime  =

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed.
            xEventGroupSetBits(xEVT_DDScheduler,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_GET_ACTIVE)
        {
            DBG_LINE("DDS: CMD: DDCMD_GET_ACTIVE\n");
            // ==> Store the head of the Active tasks in the payload
            rXMessage.pPayload = dds_ActiveTaskList;

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);
        }
        else if (rXCommandID == DDCMD_GET_COMPL)
        {
            // ==> Store the head of the completed tasks in the payload
            rXMessage.pPayload = dds_CompletedTaskList;

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);
        }
        else if (rXCommandID == DDCMD_GET_OVERDUE)
        {
            // ==> Store the head of the Overdue tasks in the payload
            rXMessage.pPayload = dds_OverdueTaskList;

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);
        }
    }
}

void _DDS_AddTaskToActiveList(uint32_t taskId)
{
    taskId++;
}

void TIM_DDS_Period_cb(xTimerHandle xTimer)
{


}

void _DDS_TaskBootStrap(void *pvParameters)
{
    uint32_t         taskId = (uint32_t)pvParameters;
    EventBits_t     evtBits;
    dds_TaskHandle_t tContext = dds_TaskList[taskId];
    uint8_t repeat = (tContext.type == PERIODIC);

    do
    {
        evtBits = xEventGroupWaitBits(tContext.taskEvt,
                        (DEAD | READY | OVERDUE),
                        pdTRUE,
                        pdFALSE,
                        MAX_WAIT);
        if(evtBits == DEAD)
        {
            break;
        }
        else if (evtBits & ( READY | OVERDUE))
        {
            DBG_TRACE_VALUE("TASK: Starting Task: %d\n", taskId);

            tContext.tFunc((void *) 1);

            DDS_CompleteTask(taskId);
        }
    }
    while(repeat);

    // Delete Task
}


void _DDS_CalculateSchedule()
{
    DBG_LINE("DDS: Entered Func Calculate Schedule\n");

    // TODO:
    // --> Sorting task algorithm
    // --> Generation
    // --> Linked List shizznut
    // --> DDS monitor
}



void _DDS_InitializeTaskNode(uint32_t taskId,
                            TaskFunction_t taskFunc,
                            dds_TaskType_e taskType,
                            uint32_t deadline)
{
    if (taskId <= MAX_DDS_TASKS && dds_TaskList[taskId].tState <= DEAD)
    {
        dds_TaskList[taskId].ID      = taskId;
        dds_TaskList[taskId].tState  = READY;
        dds_TaskList[taskId].deadline= deadline;
        dds_TaskList[taskId].type    = taskType;
        dds_TaskList[taskId].next    = NULL;
        dds_TaskList[taskId].RTime   = 0;
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
    }
    else
    {
        printf("==> EXCEEDED MAX TASKS NUMBER");
        while(1);
    }
}

