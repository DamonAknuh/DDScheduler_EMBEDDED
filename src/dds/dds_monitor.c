/*
 * dd_monitor.c
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"


uint8_t _DDS_CountList(void ** pListHead)
{
    uint8_t numberOfNodes = 0; 
    dds_TaskHandle_t * listHead = (*pListHead);
    dds_TaskHandle_t * tempNode;

    if (NULL != listHead)
    {
        tempNode = listHead;

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
    void * pTempNode = NULL;
    uint8_t           activeTasks	= 0;
    uint8_t           compTasks		= 0;
    uint8_t           overdueTasks	= 0;
    uint32_t 		  startTime     = xTaskGetTickCount();
    printf("==> Starting DDS Monitor\n");

    while(1)
    {
        xEventGroupWaitBits(xEVT_DDScheduler,
                    DDS_SORTING,
                    pdFALSE,
                    pdFALSE,
                    MAX_WAIT);

        DDS_GetActiveTasks(&pTempNode);
        activeTasks =  _DDS_CountList(&pTempNode);

        DDS_GetCompletedTasks(&pTempNode);
        compTasks =  _DDS_CountList(&pTempNode);

        DDS_GetOverdueTasks(&pTempNode);
        overdueTasks =  _DDS_CountList(&pTempNode);

        DBG_VALUE(2, "MON: Time: %u\n", pdMS_TO_TICKS(xTaskGetTickCount() - startTime));
        DBG_VALUE(2, "Active: %u\nCompl.: %u\nOvrDue: %u\n\n", activeTasks, compTasks, overdueTasks);
    }
    printf("==> DDS Monitor Task Ended\n");
}
