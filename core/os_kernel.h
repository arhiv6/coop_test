#pragma once
#ifndef _OS_KERNEL_H
#define _OS_KERNEL_H


typedef void (*pFunction)(void) ;   // тип pFunction - указатель на функцию.


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
