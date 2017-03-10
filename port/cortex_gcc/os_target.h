#pragma once
#ifndef _OS_TARGET_H
#define _OS_TARGET_H

#define NOINLINE        __attribute__((noinline))
#define NAKED           __attribute__((naked))
#define INLINE_STATIC   __attribute__((__always_inline__)) inline static

INLINE_STATIC void saveLr(void *pointer)
{
    asm volatile("  STR     LR,     [%0]    \n" // Сохранили регистр LR в pointer
                 :: "r"(pointer));
}

INLINE_STATIC void restoreLrAdnReturn(void *pointer, void *data) // Сохранили регистр LR по адресу первого параметра
{
    asm volatile("  LDR     LR,    [%0]     \n"  // Загрузили в регистр LR адрес из pointer
                 "  LDR     R0,    [%1]     \n"  // Загрузили в регистр R0 данные которые надо вернуть
                 "  BX      LR              \n"  // Вернулись из функции
                 :: "r"(pointer), "r"(data));
}

void os_initSysTimer();        // Настройка системного таймера

#endif // _OS_TARGET_H
