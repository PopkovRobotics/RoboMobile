#pragma once
//#define DEBUG_VIDEO           // Этот дефайн отвечает за режим дебага кода Video.
                              // НЕ трогайте этот дефайн.

// Если находимся в режиме дебага, то подключаем OpenCV
#ifdef DEBUG_VIDEO
#include <opencv2/opencv.hpp>
#endif
// Файлы программы
#include "Webcam.hpp"
#include "Timer.hpp"
#include "Config.hpp"

/*
 * Функция для создания потока чтения кадров с вебкамеры.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* CaptureFnc(void* ptr);
