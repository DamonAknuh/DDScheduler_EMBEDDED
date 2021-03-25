/*
 * dds_private.h
 *
 *  Created on: Mar 24, 2021
 *      Author: damonaknuh
 */

#ifndef DDS_DDS_PRIVATE_H_
#define DDS_DDS_PRIVATE_H_

#include "sys_project.h"
#include "dds_api.h"


/**********************************************************************/
// ___  ___  ____    ____ ____ _  _ ____ ____ ____ _
// |  \ |  \ [__     | __ |___ |\ | |___ |__/ |__| |
// |__/ |__/ ___]    |__] |___ | \| |___ |  \ |  | |___
//
/**********************************************************************/
#define DDS_ENABLE_MONITOR          (1)
#define DDS_TESTING                 (1)

#define DDS_STM_TIMER               (TIM2)
#define CNT_TICKS_TO_MS             1.276 // 1.5604
#define DDS_MS_TICKS(TIME_MS)       ((uint32_t)((float) (TIME_MS / 1.276)))

#define MAX_DDS_TASKS               (10U)
#define TASK_LOWEST_PR              (tskIDLE_PRIORITY + 1U)
#define DD_TASK_START_PR            (TASK_LOWEST_PR)

#define DDS_MON_PR                  (TASK_LOWEST_PR + MAX_DDS_TASKS)
#define DDS_SCHD_PR                 (DDS_MON_PR + 1U)
#define DDS_MAX_T_PR                (DDS_MON_PR - 1U)

#define MAX_WAIT                    0xffffffffUL
#define NO_WAIT                     0x0UL
#define DDS_RESULT_WAIT             (pdMS_TO_TICKS(500))

typedef enum
{
    DDS_MESSAGE         = 0b00000001,
    DDS_SCHEDULING      = 0b00000010,
    DDS_SORTING         = 0b00000100,
} dds_EventType_e;

extern EventGroupHandle_t xEVT_DDScheduler;


/**********************************************************************
// ___  ___  ____    _  _ ____ ____ ____ ____ ____ ____
// |  \ |  \ [__     |\/| |___ [__  [__  |__| | __ |___
// |__/ |__/ ___]    |  | |___ ___] ___] |  | |__] |___
**********************************************************************/
typedef enum
{
    DDCMD_CREATE = 1,
    DDCMD_RELEASE,
    DDCMD_DELETE,
    DDCMD_COMPLETE,
    DDCMD_GET_ACTIVE,
    DDCMD_GET_COMPL,
    DDCMD_GET_OVERDUE,
} dds_CommandType_e;

typedef struct
{
    TaskFunction_t  taskFunc;
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

#define DDS_Q_LEN       (3)
#define DDS_Q_ITEMSIZE  (sizeof(dds_Message_t))

extern xQueueHandle     xQ_DDSCommandQ;

/**********************************************************************/
// ___  ___  ____    ____ ____ _  _ ___ ____ _  _ ___
// |  \ |  \ [__     |    |  | |\ |  |  |___  \/   |
// |__/ |__/ ___]    |___ |__| | \|  |  |___ _/\_  |
/**********************************************************************/
typedef enum
{
    DEAD            = 0b0001,
    READY           = 0b0010,
    COMPLETED       = 0b0100,
    OVERDUE         = 0b1000,
} dds_TaskStates_e;

typedef struct dds_TaskHandle_s
{
    union 
    {
        struct 
        {
            uint32_t taskId     : 8;
            uint32_t tState     : 8;
            uint32_t resv       : 8;
            uint32_t type       : 8;
            uint32_t deadline   : 16;
            uint32_t period     : 16;
        }; 
        uint64_t HeaderBits;
    };

    TaskHandle_t         tHandle;
    TaskFunction_t       tFunc;
    uint32_t             RTime;
    uint32_t             CTime;

    EventGroupHandle_t   taskEvt;
    xTimerHandle         TIMHandle;

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

#endif /* DDS_DDS_PRIVATE_H_ */
