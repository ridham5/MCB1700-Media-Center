#ifndef HOST_CMSIS_OS_H
#define HOST_CMSIS_OS_H

/*
 * Host stand-in for the CMSIS-RTOS (RTX) calls the firmware uses, built on pthreads, so the
 * threaded build runs in the host tests. Same names and shapes as Keil's cmsis_os.h.
 */
#include <stdint.h>

typedef enum {
  osPriorityIdle = -3, osPriorityLow = -2, osPriorityBelowNormal = -1, osPriorityNormal = 0,
  osPriorityAboveNormal = 1, osPriorityHigh = 2, osPriorityRealtime = 3
} osPriority;

typedef enum { osOK = 0, osEventMessage = 0x10, osEventTimeout = 0x40, osErrorResource = 0x81 } osStatus;

#define osWaitForever 0xFFFFFFFFu

typedef void (*os_pthread)(void const *argument);

typedef struct { os_pthread pthread; osPriority tpriority; uint32_t instances; uint32_t stacksize; } osThreadDef_t;
typedef struct host_thread *osThreadId;
#define osThreadDef(name, priority, instances, stacksz) \
  const osThreadDef_t os_thread_def_##name = { (name), (priority), (instances), (stacksz) }
#define osThread(name) (&os_thread_def_##name)

typedef struct { uint32_t queue_sz; } osMessageQDef_t;
typedef struct host_queue *osMessageQId;
#define osMessageQDef(name, queue_sz, type) const osMessageQDef_t os_messageQ_def_##name = { (queue_sz) }
#define osMessageQ(name) (&os_messageQ_def_##name)

typedef struct { int dummy; } osMutexDef_t;
typedef struct host_mutex *osMutexId;
#define osMutexDef(name) const osMutexDef_t os_mutex_def_##name = { 0 }
#define osMutex(name) (&os_mutex_def_##name)

typedef struct {
  osStatus status;
  union { uint32_t v; void *p; } value;
} osEvent;

osStatus osKernelInitialize(void);
osStatus osKernelStart(void);
osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument);
osStatus osDelay(uint32_t millisec);
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id);
osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec);
osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec);
osMutexId osMutexCreate(const osMutexDef_t *mutex_def);
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec);
osStatus osMutexRelease(osMutexId mutex_id);

#endif
