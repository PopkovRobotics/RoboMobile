#include "Timer.hpp"
#include <stdio.h>

// Функции класса Timer
// Функция запускает работу таймера
void Timer::start() {
    // Получаем текущие время
    clock_gettime(CLOCK_REALTIME, &start_t);
}
// Функция останаливает работу таймера
void Timer::stop() {
    // Получаем текущие время
    clock_gettime(CLOCK_REALTIME, &end_t);
}
// Функция обнуляет таймер
void Timer::zero() {
    // Обнуляем переменную start_t
    start_t.tv_sec = 0;
    start_t.tv_nsec = 0;
    // Обнуляем переменную end_t
    end_t.tv_sec = 0;
    end_t.tv_nsec = 0;
}
/* 
 * Функция возвращает время между вызовами start() и stop()
 * в миллисекундах 
 */
long int Timer::millis() {
    long int seconds = end_t.tv_sec - start_t.tv_sec,
        nano_s = end_t.tv_nsec - start_t.tv_nsec;
    return ((seconds * 1000) + (nano_s / 1000000));
}
/* 
 * Функция возвращает время между вызовами start() и stop()
 * в микросекундах 
 */
long Timer::micros() {
    long int seconds = end_t.tv_sec - start_t.tv_sec,
        nano_s = end_t.tv_nsec - start_t.tv_nsec;
    return ((seconds * 1000000) + (nano_s / 1000));
}
