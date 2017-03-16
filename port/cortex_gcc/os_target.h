#pragma once
#ifndef _OS_TARGET_H
#define _OS_TARGET_H

#include <setjmp.h>
#include <stdint.h>

#define OS_NAKED    __attribute__((naked)) __attribute__((noinline))
#define OS_INLINE   __attribute__((__always_inline__)) inline static

typedef struct
{
    jmp_buf jmpBuf;
    uint32_t LR;
} Context;  // } Context [1];



//--------------------------------------------------------------------------------------------------

void os_initSysTimer();        // Настройка системного таймера

OS_INLINE void os_switchDispatcherToTask(Context *context)
{
    longjmp(context->jmpBuf, 1);
}

OS_INLINE void os_switchTaskToDispatcher(Context *context)
{
    extern int _os_setjmp(jmp_buf env) __asm("setjmp");
    extern void os_dispatch();

    _os_setjmp(context->jmpBuf) ? : os_dispatch();
}

OS_INLINE void os_saveTaskContext(Context *pointer)
{
    asm volatile("  STR     LR,     [%0]    \n" // Сохранили регистр LR в pointer
                 :: "r"(&pointer->LR));
}

OS_INLINE void os_restoreTaskContext(Context *pointer) // Сохранили регистр LR по адресу первого параметра
{
    asm volatile("  LDR     LR,    [%0]     \n"  // Загрузили в регистр LR адрес из pointer
                 "  BX      LR              \n"  // Вернулись из функции
                 :: "r"(&pointer->LR));
}

OS_INLINE void os_restoreTaskContextVal(Context *pointer, void *data) // Сохранили регистр LR по адресу первого параметра
{
    asm volatile("  LDR     R0,    [%0]     \n"  // Загрузили в регистр R0 данные которые надо вернуть
                 ::"r"(data));
    os_restoreTaskContext(pointer);
}

#endif // _OS_TARGET_H
