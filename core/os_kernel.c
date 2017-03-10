#include "cmtos.h"
#include "os_kernel.h"

// Простейший обработчик исключений, если не найден системный
#if (!defined ASSERT)
    #define ASSERT(e) while(!(e)) {/* infinity loop */}
#endif

uint32_t dd;

NAKED NOINLINE void os_delay(uint32_t ticks)
{
    saveLr(&(taskArray[currentTask].LR));

    taskArray[currentTask].delay = ticks;

    taskArray[currentTask].state = DELAYED;

    setjmp(taskArray[currentTask].context) ? : __CALL_DISPATCH;

    restoreLrAdnReturn(&(taskArray[currentTask].LR), 0);
}

NAKED NOINLINE  void os_yield()
{
    saveLr(&(taskArray[currentTask].LR));

    setjmp(taskArray[currentTask].context) ? : __CALL_DISPATCH;

    restoreLrAdnReturn(&(taskArray[currentTask].LR), 0);
}

Task taskArray[MAX_TASKS] = {} ;   // очередь задач
volatile uint8_t currentTask = 0;           // "хвост" очереди (количество задач)
jmp_buf _os_context ;

void os_sysTimer_isr() // TODO inline
{
    for (uint8_t i = 0; i < MAX_TASKS; i++)       // проходим по списку задач
    {
        if (taskArray[i].delay)
        {
            taskArray[i].delay--;
        }
    }
}

//--------------------------------------------------------------------------------------------------
uint32_t checkEvent(Objects *object)
{
    switch (object->type)
    {
        case WAIT_FALSE:                     // ждём мьютекса
            if (object->check == false) // мьютекс освободился
            {
                return true;            // возвращаемся в задачу
            }
            break;

        case WAIT_TRUE:                   // ждём сообщения
            if (object->check == true)  // появилось сообщение
            {
                return true;            // возвращаемся в задачу
            }
            break;

        default:
            ASSERT(0);
    }

    return false; // ждём дальше
}

//--------------------------------------------------------------------------------------------------
// Диспетчер РТОС

void os_dispatch() // TODO NAKED INLINE
{
    //currentTask = 1;//IDLE_NUM; // // idle
    // uint32_t maxTaskCost = 0;

    //uint32_t respone = 0;

    for (currentTask = 0;  ; currentTask++)
    {
        if (currentTask == MAX_TASKS)
        {
            currentTask = 0;
        }

        if (setjmp(_os_context)) // Сохранить метку, куда вернуться
        {
            continue;           // Если вернулись из задачи - начинаем работу со следующией задачей
        }

//..................................................................................................
//    for (uint32_t i = 0; i < MAX_TASKS; i++)	// проверяем статус всех задач
//    {

        //uint32_t respone = 0;
        switch (taskArray[currentTask].state)               // оцениваем состояние задачи
        {
            case NOINIT:
                taskArray[currentTask].state = ACTIVE; // TODO del
                taskArray[currentTask].pFunc(); // простой запуск функции
                ASSERT(0);

            case WAITING:
                if (checkEvent(taskArray[currentTask].lockObject))
                {
                    taskArray[currentTask].state = READY;
                    //respone = taskArray[currentTask].lockObject->data;
                }
                break;

            case DELAYED:
                if (taskArray[currentTask].delay == 0)
                {
                    taskArray[currentTask].state = READY;
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
                taskArray[currentTask].state = READY;
            case READY: // cost +++
                break;

            default:
                ASSERT(0);
        }
//..................................................................................................
        // здесь - выбор из задач с READY, назначаем currentTask
        if (taskArray[currentTask].state != READY)
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
        // возвращение в выбранную функцию
        taskArray[currentTask].state = ACTIVE;
        longjmp(taskArray[currentTask].context, 1); // возвращаемся в задачу
        //longjmp(taskArray[currentTask].context, respone); // возвращаемся в задачу
        //RESTORE_CONTEXT(taskArray[currentTask].context, respone);
        //RESTORE_CONTEXT(taskArray[currentTask].context, 1);

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
    //   NVIC_DisableIRQ(SysTick_IRQn);
    taskArray[priority].delay  = 0;
    // NVIC_EnableIRQ(SysTick_IRQn);
    taskArray[priority].state  = NOINIT;
    //taskArray[priority].cost = 0;
}

void os_run() // TODO inline naked
{
    for (uint8_t i = 0; i < MAX_TASKS ; i++) // +1 for idle
    {
        ASSERT(taskArray[i].pFunc != 0); // noinit task!
    }

    os_initSysTimer();

//    if (SysTick_Config(SystemCoreClock / ticksFreq))    // Настраиваем частоту системного таймера
//    {
//        ASSERT(0);
//    }
//    __enable_irq();

    os_dispatch();
}
