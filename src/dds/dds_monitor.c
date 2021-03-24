/*
 * dd_monitor.c
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"


void samplecb(void *pvParameters)
{
    for (uint32_t i; i < 10000; i++)
    {
    }
}

void _DDS_Monitor(void *pvParameters)
{
    printf("==> Starting DDS Monitor\n");

    DDS_CreateTask(samplecb, PERIODIC, 3, 500);

    DDS_CreateTask(samplecb, PERIODIC, 1, 500);

    DDS_CreateTask(samplecb, PERIODIC, 2, 500);

    while(1)
    {

        DDS_ReleaseTask(1);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("==> DDS Monitor Task Ended\n");
}
