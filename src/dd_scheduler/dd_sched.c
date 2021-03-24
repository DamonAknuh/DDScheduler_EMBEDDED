/*
 * dd_scheduler.c
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"
#include "dd_api.h"
#include "dd_sched.h"

EventGroupHandle_t xEVT_DDScheduler;
xQueueHandle       xQ_DDSCommandQ;

dds_TaskHandle_t dds_TaskList[MAX_DDS_TASKS];

dds_TaskHandle_t * dds_ActiveTaskList;
dds_TaskHandle_t * dds_CompletedTaskList;
dds_TaskHandle_t * dds_OverdueTaskList;
dds_TaskHandle_t * dds_DeadTaskList;

static void _DDS_HandleMSG();
static void _DDS_CalculateSchedule();
static uint32_t _DDS_InitializeTaskNode(uint32_t taskId,
                                    TaskHandle_t taskHandle,
                                    dds_CommandType_e taskType,
                                    uint32_t deadline);

void _DDS_Scheduler(void *pvParameters)
{
    EventBits_t evtBits;

    printf("==> Starting Deadline Driven Scheduler\n");

    while(1)
    {
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
    printf("==> Deadline Driven Scheduler Task Ended\n");
}

void _DDS_HandleMSG()
{
    dds_CreateMsg_t   dataPayload;
    dds_Message_t     rXMessage; 
    dds_CommandType_e rXCommandID;

    if (pdPass == xQueueReceive(xQ_DDSCommandQ,
                                &(rXMessage), 
                                NO_WAIT)) 
    {
        rXCommandID = rXMessage.dds_CID;

        DBG_printf("==> RCVD DDS MSG %d", rXCommandID);
        if (rXCommandID == DDCMD_CREATE)
        {
            dataPayload = rXMessage.pPayload; 

            _DDS_InitializeTaskNode(dataPayload.taskId,
                                    dataPayload.taskHandle,
                                    dataPayload.taskType,
                                    dataPayload.deadline);

            _DDS_AddTaskToActiveList(dataPayload.taskId);

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed. 
            xEventGroupSetBits(xEVT_FSM_Transition,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_DELETE)
        {

            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed. 
            xEventGroupSetBits(xEVT_FSM_Transition,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_RELEASE)
        {
            // TODO
            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed. 
            xEventGroupSetBits(xEVT_FSM_Transition,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_COMPLETE)
        {
            // TODO
            // ==> Set the event bits to signal that aysnch. call has finished
            xEventGroupSetBits(rXMessage.xReturnEvt, ddsSUCCESS);

            // ==> Trigger Scheduler to run as task lists have changed. 
            xEventGroupSetBits(xEVT_FSM_Transition,  DDS_SCHEDULING);
        }
        else if (rXCommandID == DDCMD_GET_ACTIVE)
        {
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

void _DDS_InitializeTaskNode(uint32_t taskId,
                            TaskHandle_t taskHandle,
                            dds_CommandType_e taskType,
                            uint32_t deadline)
{
    if (taskId <= MAX_DDS_TASKS && dds_TaskList[taskId].tState == UNACTIVE)
    {
        dds_TaskList[taskId].ID      = taskId;
        
        dds_TaskList[taskId].deadline= deadline;
        dds_TaskList[taskId].type    = taskType;
        dds_TaskList[taskId].next    = NULL;
        dds_TaskList[taskId].RTime   = 0;
        dds_TaskList[taskId].CTime   = 0;
        dds_TaskList[taskId].tHandle = taskHandle;
        // dds_TaskList[taskId].tHandle = xTaskCreate( )
        dds_TaskList[taskId].TIMHandle  = xTimerCreate("TASKTIMER", 
                                                pdMS_TO_TICKS(deadline), 
                                                (taskType == PERIODIC), 
                                                (void *) taskId, 
                                                TIM_DDS_Period_cb);
    }
    else
    {
        assert(0);
    }
}

void TIM_DDS_Period_cb(xTimerHandle xTimer)
{


}


void _DDS_CalculateSchedule()
{
    DBG_printf("==> Entered Func Calculate Schedule");

    // TODO:
    // --> Sorting task algorithm
    // --> Generation
    // --> Linked List shizznut
    // --> DDS monitor
    // --> Heap 4. 

}

void _DDS_Monitor(void *pvParameters)
{
    printf("==> Starting DDS Monitor\n");

    while(1)
    {




    }
    printf("==> DDS Monitor Task Ended\n");
}
