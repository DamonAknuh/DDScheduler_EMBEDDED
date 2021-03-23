/*
 * dd_api.c
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"
#include "dd_scheduler.h"



uint32_t dd_Init()
{
	// Create Event Groups for processing commands coming into the scheduler
	xEVT_DDScheduler = xEventGroupCreate();

	xTaskCreate(xTrafficSimulator, "dd_Scheduler", 	configMINIMAL_STACK_SIZE, NULL, TASK_PR_2, NULL);

#if DD_ENABLE_MONITOR
	xTaskCreate(xTrafficLightFSM,  "dd_Monitor", 	configMINIMAL_STACK_SIZE, NULL, TASK_PR_2, NULL);
#endif // DD_ENABLE_MONITOR
}



