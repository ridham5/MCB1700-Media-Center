#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "cmsis_os.h"

/* pthreads implementation of the CMSIS-RTOS calls in stubs/cmsis_os.h (host tests only). */

extern volatile uint32_t msTicks;

void game_clock_init(void) { }   /* on the host, fake_clock_start() drives msTicks */

osStatus osKernelInitialize(void) { return osOK; }
osStatus osKernelStart(void) { return osOK; }

struct host_thread { pthread_t t; os_pthread fn; void *arg; };

static void *trampoline(void *p) {
  struct host_thread *th = p;
  th->fn(th->arg);
  return NULL;
}

osThreadId osThreadCreate(const osThreadDef_t *def, void *argument) {
  struct host_thread *th = calloc(1, sizeof *th);
  th->fn = def->pthread;
  th->arg = argument;
  pthread_create(&th->t, NULL, trampoline, th);
  pthread_detach(th->t);
  return th;
}

/* Waits on the same 1 ms clock the firmware uses, so tests run in "board time". */
osStatus osDelay(uint32_t ms) {
  uint32_t start = msTicks;
  while ((uint32_t)(msTicks - start) < ms) usleep(200);
  return osEventTimeout;
}

struct host_queue {
  pthread_mutex_t m;
  uint32_t *buf;
  uint32_t size, head, count;
};

osMessageQId osMessageCreate(const osMessageQDef_t *def, osThreadId owner) {
  struct host_queue *q = calloc(1, sizeof *q);
  (void)owner;
  pthread_mutex_init(&q->m, NULL);
  q->size = def->queue_sz;
  q->buf = calloc(q->size, sizeof *q->buf);
  return q;
}

osStatus osMessagePut(osMessageQId q, uint32_t info, uint32_t ms) {
  osStatus st = osErrorResource;
  (void)ms;
  pthread_mutex_lock(&q->m);
  if (q->count < q->size) {
    q->buf[(q->head + q->count) % q->size] = info;
    q->count++;
    st = osOK;
  }
  pthread_mutex_unlock(&q->m);
  return st;
}

osEvent osMessageGet(osMessageQId q, uint32_t ms) {
  osEvent e;
  (void)ms;
  e.status = osOK;
  e.value.v = 0;
  pthread_mutex_lock(&q->m);
  if (q->count) {
    e.value.v = q->buf[q->head];
    q->head = (q->head + 1) % q->size;
    q->count--;
    e.status = osEventMessage;
  }
  pthread_mutex_unlock(&q->m);
  return e;
}

struct host_mutex { pthread_mutex_t m; };

osMutexId osMutexCreate(const osMutexDef_t *def) {
  struct host_mutex *mx = calloc(1, sizeof *mx);
  pthread_mutexattr_t a;
  (void)def;
  pthread_mutexattr_init(&a);
  pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);   /* RTX mutexes are recursive too */
  pthread_mutex_init(&mx->m, &a);
  return mx;
}

osStatus osMutexWait(osMutexId mx, uint32_t ms) { (void)ms; pthread_mutex_lock(&mx->m); return osOK; }
osStatus osMutexRelease(osMutexId mx) { pthread_mutex_unlock(&mx->m); return osOK; }
