#pragma once
// Файлы программы
#include "Webcam.hpp"
#include "Timer.hpp"
#include "Config.hpp"

/*
 * Функция для создания потока чтения кадров с вебкамеры.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* CaptureFnc(void* ptr);
