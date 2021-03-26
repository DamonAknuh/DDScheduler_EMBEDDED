/*
 * dd_monitor.c
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"


uint8_t _DDS_CountList(void ** listHead)
{
    uint8_t numberOfNodes = 0; 
    dds_TaskHandle_t * tempNode;

    if (NULL != (*listHead))
    {
        tempNode = (*listHead);

        while(tempNode != NULL)
        {
            tempNode = tempNode->next;
            numberOfNodes++;
        }
    }

    return numberOfNodes; 
}

void _DDS_Monitor(void *pvParameters)
{
    dds_TaskHandle_t * pTempNode;
    uint32_t          curTime;
    uint8_t           activeTasks;
    uint8_t           compTasks;
    uint8_t           overdueTasks;
    printf("==> Starting DDS Monitor\n");

    while(1)
    {
        TIM_Cmd(DDS_STM_TIMER, DISABLE);

        curTime = TIM_GetCounter(DDS_STM_TIMER);
        DBG_VALUE("MON: Time: %u", DDS_TICKS_2_MS(curTime));


        DDS_GetActiveTasks(&pTempNode);
        activeTasks =  _DDS_CountList(&pTempNode);

        DDS_GetCompletedTasks(&pTempNode);
        compTasks =  _DDS_CountList(&pTempNode);

        DDS_GetOverdueTasks(&pTempNode);
        overdueTasks =  _DDS_CountList(&pTempNode);

        printf("A-Tasks: %u\n
                C-Tasks: %u\n
                O-Tasks: %u\n", activeTasks, compTasks, overdueTasks);
        printf("");

        TIM_Cmd(DDS_STM_TIMER, ENABLE);

        vTaskDelay(pdMS_TO_TICKS(DDS_MON_WAIT_MS));
    }
    printf("==> DDS Monitor Task Ended\n");
}
