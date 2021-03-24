/*
 * dds_scheduler.h
 *
 *  Created on: Mar 23, 2021
 *      Author: damonaknuh
 */

#ifndef SRC_dds_SCHEDULER_dds_SCHEDULER_H_
#define SRC_dds_SCHEDULER_dds_SCHEDULER_H_

#include "sys_project.h"
#include "sys_timer.h"
#include "sys_tasks.h"


/**********************************************************************/
// ___  ___  ____    ____ ____ _  _ ____ ____ ____ _    
// |  \ |  \ [__     | __ |___ |\ | |___ |__/ |__| |    
// |__/ |__/ ___]    |__] |___ | \| |___ |  \ |  | |___ 
/**********************************************************************/

#define dds_ENABLE_MONITOR         (0)

#define TASK_LOWEST_PR (tskIDLE_PRIORITY + 1U)
#define MAX_DDS_TASKS   (60U)
#define TASK_MAX_PR    (TASK_LOWEST_PR + MAX_DDS_TASKS)

#define MAX_WAIT        0xffffffffUL
#define NO_WAIT         0x0UL
#define DDS_RESULT_WAIT (pdMS_TO_TICKS(500))

typedef enum
{
    DDS_MESSAGE         = 0b00000001, 
    DDS_SCHEDULING      = 0b00000010,
} dds_EventType_e;

extern EventGroupHandle_t xEVT_DDScheduler;

typedef enum
{
    PERIODIC,
    APERIODIC,
    UNKNOWN,
} dds_TaskType_e;

/**********************************************************************
// ___  ___  ____    _  _ ____ ____ ____ ____ ____ ____ 
// |  \ |  \ [__     |\/| |___ [__  [__  |__| | __ |___ 
// |__/ |__/ ___]    |  | |___ ___] ___] |  | |__] |___ 
/**********************************************************************/

typedef enum
{
    DDCMD_CREATE,
    DDCMD_RELEASE,
    DDCMD_DELETE,
    DDCMD_COMPLETE,
    DDCMD_GET_ACTIVE,
    DDCMD_GET_COMPL,
    DDCMD_GET_OVERDUE,
} dds_CommandType_e;

typedef struct
{
    TaskHandle_t    taskHandle;
    dds_TaskType_e  taskType;
    uint32_t        taskId;
    uint32_t        deadline;
} dds_CreateMsg_t;

typedef struct
{
    dds_CommandType_e   dds_CID;
    EventGroupHandle_t  xReturnEvt; 
    void *              pPayload;
} dds_Message_t;

#define DDS_Q_LEN       (10)
#define DDS_Q_ITEMSIZE  (sizeof(dds_Message_t))

extern xQueueHandle     xQ_DDSCommandQ;

/**********************************************************************/
// ___  ___  ____    ____ ____ _  _ ___ ____ _  _ ___ 
// |  \ |  \ [__     |    |  | |\ |  |  |___  \/   |  
// |__/ |__/ ___]    |___ |__| | \|  |  |___ _/\_  |  
/**********************************************************************/
typedef enum
{
    UNACTIVE = 0,
    READY,
    COMPLETED,
    OVERDUE,
} dds_TaskStates_e;

typedef struct dds_TaskHandle_s
{
    uint32_t         ID;
    TaskHandle_t     tHandle;
    uint32_t         RTime;
    uint32_t         CTime;
    uint32_t         deadline;
    xTimerHandle     TIMHandle;
    dds_TaskType_e   type;
    dds_TaskStates_e tState;

    struct dds_TaskHandle_s * next;
} dds_TaskHandle_t;

/**********************************************************************
** ____ _  _ _  _ ____ ___ _ ____ _  _ ____
** |___ |  | |\ | |     |  | |  | |\ | [__
** |    |__| | \| |___  |  | |__| | \| ___]
***********************************************************************/

void _DDS_Scheduler(void *pvParameters);
void _DDS_Monitor(void *pvParameters);
void  TIM_DDS_Period_cb(xTimerHandle xTimer);

#endif /* SRC_dds_SCHEDULER_dds_SCHEDULER_H_ */
