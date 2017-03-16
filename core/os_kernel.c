#include "cmtos.h"
#include "os_kernel.h"
#include "os_target.h"
#include "os_config.h"

//--------------------------------------------------------------------------------------------------
//  PRIORITY MODE
#define OS_PRIORITY_NORMAL      0
#define OS_PRIORITY_DISABLED    1
#define OS_PRIORITY_EXTENDED    2
#ifndef OS_PRIORITY_LEVEL
    #if     defined(OS_DISABLE_PRIORITY)
        #define OS_PRIORITY_LEVEL   OS_PRIORITY_DISABLED
    #elif   defined(OS_EXTENDED_PRIORITY)
        #define OS_PRIORITY_LEVEL   OS_PRIORITY_EXTENDED
    #else
        #define OS_PRIORITY_LEVEL   OS_PRIORITY_NORMAL
    #endif
#endif

// Простейший обработчик исключений, если не найден системный
#if (!defined ASSERT)
    #define ASSERT(e) while(!(e)) {/* infinity loop */}
#endif
//--------------------------------------------------------------------------------------------------
// Типы данных:
typedef struct                  // структура с описанием задачи
{
    pFunction pFunc;            // указатель на функцию
    uint32_t delay;             // задержка перед первым запуском задачи
    Context context;            // контекст задачи
#if OS_PRIORITY_LEVEL == OS_PRIORITY_EXTENDED
    uint32_t priority;        //
#endif
} Task;

Task taskArray[MAX_TASKS] = {} ;            // очередь задач
volatile uint8_t currentTaskNumber = -1;    // номер (в function) активной задачи
Task *currentTask;                 // указатель на активную задачу
static uint32_t readyTasksMap = 0;          // карта готовых к работе задач

void setReadyTask(uint32_t taskNum)
{
    readyTasksMap |= (1 << taskNum);
}

void clearReadyTask(uint32_t taskNum)
{
    readyTasksMap &= ~(1 << taskNum);
}

bool isReadyTask(uint32_t taskNum)
{
    return (readyTasksMap & (1 << taskNum));
}

OS_NAKED void os_delay(uint32_t ticks)
{
    os_saveTaskContext(&(currentTask->context));
    clearReadyTask(currentTaskNumber);
    currentTask->delay = ticks;
    os_switchTaskToDispatcher(&currentTask->context);
    os_restoreTaskContextVal(&(currentTask->context), 0);
}

OS_NAKED  void os_yield()
{
    os_saveTaskContext(&(currentTask->context));
    os_switchTaskToDispatcher(&currentTask->context);
    os_restoreTaskContextVal(&(currentTask->context), 0);
}

inline void os_sysTimer_isr()
{
    for (uint_fast8_t i = 0; i < MAX_TASKS; i++) // проходим по списку задач
    {
        Task *p = &taskArray[i];

        if (p->delay > 0)
        {
            if (--p->delay == 0)
            {
                setReadyTask(i);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
OS_NAKED void os_dispatch()
{
    //#if OS_PRIORITY_LEVEL == OS_PRIORITY_DISABLED
    //#if OS_PRIORITY_LEVEL == OS_PRIORITY_EXTENDED
    //#if OS_PRIORITY_LEVEL == OS_PRIORITY_NORMAL
    //#endif

    for (;;)
    {
        currentTaskNumber++;
        if (currentTaskNumber == MAX_TASKS)
        {
            currentTaskNumber = 0;
        }
        currentTask =  &taskArray[currentTaskNumber];
//..................................................................................................
        if (isReadyTask(currentTaskNumber))
        {
            if (currentTask->pFunc)
            {
                //currentTask->state = ACTIVE; // TODO del
                pFunction function = currentTask->pFunc;
                currentTask->pFunc = 0;
                function(); // простой запуск функции
                ASSERT(0);
            }
            else
            {
                os_switchDispatcherToTask(&currentTask->context); // возвращаемся в задачу
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
// Добавление задачи в список

void os_setTask(pFunction function, uint8_t priority)
{
    ASSERT(priority < MAX_TASKS);   // bad priority!
    ASSERT(function != 0);          // reinit task!

    taskArray[priority].pFunc  = function;
    taskArray[priority].delay  = 0;

#if OS_PRIORITY_LEVEL == OS_PRIORITY_EXTENDED
    taskArray[priority].priority = priority;
#endif
}

inline void os_run() // TODO inline naked
{
    for (uint8_t i = 0; i < MAX_TASKS ; i++) // +1 for idle
    {
        ASSERT(taskArray[i].pFunc != 0); // noinit task!
        setReadyTask(i);
    }

    os_initSysTimer();

    os_dispatch(); // запускаем диспетчер
}
