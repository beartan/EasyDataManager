/*
 * This file is part of the EasyDataManager Library.
 *
 * Copyright (C) 2013 by Armink <armink.ztl@gmail.com>
 *
 * Function: a logger in this library
 * Created on: 2013-11-10
 */

#include "log.h"

#if defined(EDM_USING_PTHREAD)
static pthread_mutex_t _printLock;
static pthread_mutex_t* printLock = &_printLock;
#elif defined(EDM_USING_RTT)
static struct rt_mutex _printLock;
static rt_mutex_t printLock = &_printLock;
//static rt_mutex_t printLock;
#endif


static uint8_t isOpenPrint = FALSE;
static uint8_t isInitLog = FALSE;

static void printTime(void);
static void printThreadInfo(void);
static void threadMutexInit(void);
static void threadMutexLock(void);
static void threadMutexUnlock(void);
static void threadMutexDestroy(void);


/**
 * This function will initialize logger.
 *
 */
void initLogger(uint8_t isOpen) {
    isOpenPrint = isOpen;
    if (isOpen) {
        threadMutexInit();
    }
    isInitLog = TRUE;
    printf("Wellcom to ues Easy Data Manager(V%ld.%02ld.%02ld) by armink. E-Mail:armink.ztl@gmail.com \n",
            EDM_VERSION,EDM_SUBVERSION,EDM_REVISION);
    printf("Logger initialize success.\n");
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void debug(const char *file, const long line, const char *format, ...) {
    va_list args;
#if defined(EDM_USING_RTT)
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
#endif
    if (!isOpenPrint || !isInitLog) {
        return;
    }
    va_start(args, format);
    /* args point to the first variable parameter */
    /* lock the print ,make sure the print data full */
    threadMutexLock();
    printTime();
    printThreadInfo();
#if defined(EDM_USING_PTHREAD)
    printf("(%s:%ld) ", file, line);
    /* must use vprintf to print */
    vprintf(format,args);
    printf("\n");
#elif defined(EDM_USING_RTT)
    rt_kprintf("(%s:%ld) ", file, line);
    /* must use vprintf to print */
    rt_vsprintf(rt_log_buf,format,args);
    rt_kprintf("%s\n",rt_log_buf);
#endif
    threadMutexUnlock();
    va_end(args);
}

/**
 * This function destroy the logger.
 *
 */
void destroyLogger(void) {
    if (isOpenPrint) {
        threadMutexDestroy();
    }
    isOpenPrint = FALSE;
    isInitLog = FALSE;
}

/**
 * This function is print thread info.
 *
 */
void printThreadInfo(void){

#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
    printf("tid:%04ld ",GetCurrentThreadId());
#else
    printf("tid:%#x ",pthread_self());
#endif
#elif defined(EDM_USING_RTT)
    rt_kprintf("thread_name:%s ",rt_thread_self()->name);
#endif

}

/**
 * This function is print time info.
 *
 */
void printTime(void) {
#if defined(EDM_USING_PTHREAD)
#if defined(WIN32) || defined(WIN64)
    SYSTEMTIME currTime;
    GetLocalTime(&currTime);
    printf("%02d-%02d %02d:%02d:%02d.%03d ", currTime.wMonth, currTime.wDay,
            currTime.wHour, currTime.wMinute, currTime.wSecond,
            currTime.wMilliseconds);
#else
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep);
    if(p==NULL) {
        return;
    }
    printf("%02d-%02d %02d:%02d:%02d.%03d ",p->tm_mon+1 ,p->tm_mday ,p->tm_hour ,p->tm_min,p->tm_sec);
#endif

#elif defined(EDM_USING_RTT)
    rt_kprintf("tick: %010ld ",rt_tick_get());
#endif
}

/**
 * This function is initialize the printLock mutex.
 *
 */
static void threadMutexInit(void){
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_init(printLock, NULL);

#elif defined(EDM_USING_RTT)
    rt_mutex_init(printLock,"printLock",RT_IPC_FLAG_PRIO);
#endif
}

/**
 * This function is lock the printLock mutex.
 *
 */
static void threadMutexLock(void){
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_lock(printLock);

#elif defined(EDM_USING_RTT)
    rt_mutex_take(printLock,RT_WAITING_FOREVER);
#endif
}

/**
 * This function is unlock the printLock mutex.
 *
 */
static void threadMutexUnlock(void){
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_unlock(printLock);

#elif defined(EDM_USING_RTT)
    rt_mutex_release(printLock);
#endif
}

/**
 * This function is destroy the printLock mutex.
 *
 */
static void threadMutexDestroy(void){
#if defined(EDM_USING_PTHREAD)
    pthread_mutex_destroy(printLock);

#elif defined(EDM_USING_RTT)
    rt_mutex_detach(printLock);
#endif
}
