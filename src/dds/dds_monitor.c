/*
 * dd_monitor.c
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "dds_api.h"
#include "dds_private.h"



void _DDS_Monitor(void *pvParameters)
{
    printf("==> Starting DDS Monitor\n");

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("==> DDS Monitor Task Ended\n");
}
