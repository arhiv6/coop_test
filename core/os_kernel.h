#pragma once
#ifndef _OS_KERNEL_H
#define _OS_KERNEL_H

#include <setjmp.h>

#define __CALL_DISPATCH longjmp(_os_context, 1);

// Типы данных:
typedef enum { NOINIT, READY, ACTIVE, DELAYED, WAITING, /*WAIT_TIMEOUT*/} TaskState; // состояния задачи
typedef void (*pFunction)(void) ;   // тип pFunction - указатель на функцию.
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
    //jmp_buf context;                        // контекст задачи
    struct
      {
        jmp_buf context;
        uint32_t LR;
      };
    Objects *lockObject;                    // указатель на event задачи
} Task;
typedef Objects Mutex;
typedef Objects Semaphore;
typedef Objects Message;
extern  Task taskArray[] ;                  // очередь задач
extern volatile uint8_t currentTask ;       // "хвост" очереди (количество задач)
extern jmp_buf _os_context;


//==================================================================================================
//                                       ПРОТОТИПЫ ФУНКЦИЙ

//--------------------------------------------------------------------------------------------------
// Системные сервисы

void os_run();                                          // Запускает ядро операционки в работу. Вызывается в конце main
void os_delay(uint32_t ticks);            // Выдерживаем паузу внутри задачи Разрешен вызов только в контексте задачи Переключает контекст Использует системный таймер
void os_yield();                          // Передача управления планировщику Разрешен вызов только в контексте задачи Переключает контекст
void os_sysTimer_isr();                  // Обработка всех таймеров, вызывается из прерывания // TODO //#define SCMRTOS_USE_CUSTOM_TIMER 0

//--------------------------------------------------------------------------------------------------
// Управление задачами

void os_setTask(pFunction function, uint8_t priority);  // Инициализируем конкретную задачу. Нельзя вызывать из прерывания

#endif // _OS_KERNEL_H
