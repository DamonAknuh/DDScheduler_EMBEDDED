/*
 * task_generator.c
 *
 *  Created on: Mar 25, 2021
 *      Author: damonaknuh
 */


#include "dds_testing.h"

#if DDS_TEST_BENCH == 1

#define TB_TASK_NUM	(3)
const uint32_t testBench [TB_TASK_NUM][4] =
{
// T_ID			T_TYPE			PERIOD		EXEC
	{1,  		PERIODIC,		500,		95},
	{2, 		PERIODIC,		500,		150},
	{3,			PERIODIC,		750,		750}
};

#elif DDS_TEST_BENCH == 2

#define TB_TASK_NUM	(3)
const uint32_t testBench [TB_TASK_NUM][4] =
{
// T_ID			T_TYPE			PERIOD		EXEC
	{1,  		PERIODIC,		250,		95},
	{2, 		PERIODIC,		500,		150},
	{3,			PERIODIC,		750,		750}
};

#elif DDS_TEST_BENCH == 3

#define TB_TASK_NUM	(3)
const uint32_t testBench [TB_TASK_NUM][4] =
{
// T_ID			T_TYPE			PERIOD		EXEC
	{1,  		PERIODIC,		500,		100},
	{2, 		PERIODIC,		500,		200},
	{3,			PERIODIC,		500,		200}
};

#endif // DDS_TEST_BENCH


void _DDS_TBTaskFunction(void *pvParameters)
{
	uint32_t  taskId = (uint32_t)pvParameters;
	uint32_t  taskExecTime = testBench[taskId-1][3];

    for (uint32_t i =0; i < (taskExecTime / 5); i++)
    {
    	for (uint32_t ii = 0; ii < 64222; ii++)
    	{
    		asm("nop");
    	}
    }
}


void _DDS_TBTaskGenerator(void *pvParameters)
{
	DBG_TRACE_VALUE("==> Starting the DDS Testbench %u\n", DDS_TEST_BENCH);

	for(uint32_t i = 0; i < TB_TASK_NUM; i++)
	{
		DDS_CreateTask(_DDS_TBTaskFunction,
						testBench[i][1],
						testBench[i][0],
						testBench[i][2]);
	}

	printf("==> DDS Testbench Created\n");
	vTaskDelete(NULL);
}







