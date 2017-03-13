#include "cmtos.h"
#include "os_kernel.h"
#include "os_target.h"

// Типы данных:
typedef enum { NOINIT, READY, ACTIVE, DELAYED, WAITING, /*WAIT_TIMEOUT*/} TaskState; // состояния задачи

typedef enum {WAIT_FALSE, WAIT_TRUE,  } ObjectsTypes;
typedef struct
{
    ObjectsTypes type;
    bool check;
    uint32_t data_plus_one;
} Objects;
typedef struct                              // структура с описанием задачи
{
    pFunction pFunc;                        // указатель на функцию
    uint32_t delay;                         // задержка перед первым запуском задачи
    //uint32_t cost;                          //
    TaskState state;                        // период запуска задачи
    Context context;              // контекст задачи
    Objects *lockObject;                    // указатель на event задачи
} Task;
typedef Objects Mutex;
typedef Objects Semaphore;
typedef Objects Message;

// Context _os_context;

 Task taskArray[MAX_TASKS] = {} ;   // очередь задач
 volatile uint8_t currentTaskNumber = -1;           // "хвост" очереди (количество задач)
Task *currentTask;

// Простейший обработчик исключений, если не найден системный
#if (!defined ASSERT)
    #define ASSERT(e) while(!(e)) {/* infinity loop */}
#endif

OS_NAKED void os_delay(uint32_t ticks)
{
    os_saveTaskContext(&(currentTask->context));

    currentTask->delay = ticks;
    currentTask->state = DELAYED;

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
    for (uint8_t i = 0; i < MAX_TASKS; i++) // проходим по списку задач
    {
        if (taskArray[i].delay)
        {
            taskArray[i].delay--;
        }
    }
}

//--------------------------------------------------------------------------------------------------
//uint32_t checkEvent(Objects *object)
//{
//    switch (object->type)
//    {
//        case WAIT_FALSE:                     // ждём мьютекса
//            if (object->check == false) // мьютекс освободился
//            {
//                return true;            // возвращаемся в задачу
//            }
//            break;

//        case WAIT_TRUE:                   // ждём сообщения
//            if (object->check == true)  // появилось сообщение
//            {
//                return true;            // возвращаемся в задачу
//            }
//            break;

//        default:
//            ASSERT(0);
//    }

//    return false; // ждём дальше
//}

//--------------------------------------------------------------------------------------------------
// Диспетчер РТОС

OS_NAKED void os_dispatch()
{
    for (;;)
    {
        currentTaskNumber++;
        if (currentTaskNumber == MAX_TASKS)
        {
            currentTaskNumber = 0;
        }
        currentTask =  &taskArray[currentTaskNumber];
//..................................................................................................

//    for (uint32_t i = 0; i < MAX_TASKS; i++)	// проверяем статус всех задач
//    {

        //uint32_t respone = 0;
        switch (currentTask->state)               // оцениваем состояние задачи
        {
            case NOINIT:
                currentTask->state = ACTIVE; // TODO del
                currentTask->pFunc(); // простой запуск функции
                ASSERT(0);

//            case WAITING:
//                if (checkEvent(currentTask->lockObject))
//                {
//                    currentTask->state = READY;
//                    //respone = taskArray[currentTask].lockObject->data;
//                }
//                break;

            case DELAYED:
                if (currentTask->delay == 0)
                {
                    currentTask->state = READY;
                }
                break;

//            case WAIT_TIMEOUT:
//                if (taskArray[currentTask].delay == 0)
//                {
//                    taskArray[currentTask].state = READY;
//                    respone = false;
//                }
//                if (checkObject(taskArray[currentTask].lockObject))
//                {
//                    taskArray[currentTask].state = READY;
//                    respone = true;
//                    //respone = taskArray[currentTask].lockObject->data;
//                }
//                break;

            case ACTIVE:
                currentTask->state = READY;
            case READY: // cost +++
                break;

            default:
                ASSERT(0);
        }
//..................................................................................................
        // здесь - выбор из задач с READY, назначаем currentTask
        if (currentTask->state != READY)
        {
            continue;
        }
        //        if (taskArray[i].state == ACTIVE)		// Если задача готова к запуску
        //        {
        //            taskArray[i].cost += (MAX_TASKS - i); // повышаем ей вес
        //            if (taskArray[i].cost > maxTaskCost) // и если он правысил старый
        //            {
        //                maxTaskCost = taskArray[i].cost; // обновляем
        //                currentTask = i;
        //            }
        //        }
        //      taskArray[currentTask].cost = 0;
//..................................................................................................
        currentTask->state = ACTIVE;
        os_switchDispatcherToTask(&currentTask->context); // возвращаемся в задачу

        ASSERT(0);
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
    taskArray[priority].state  = NOINIT;
    //taskArray[priority].cost = 0;
}

inline void os_run() // TODO inline naked
{
    for (uint8_t i = 0; i < MAX_TASKS ; i++) // +1 for idle
    {
        ASSERT(taskArray[i].pFunc != 0); // noinit task!
    }

    os_initSysTimer();

    os_dispatch(); // запускаем диспетчер
}
