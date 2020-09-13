#pragma once
// Библиотеки C++
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
// Файлы программы
#include "Timer.hpp"

// Класс для защиты от одновременной записи объекта
template<typename Type>
class Object {
private:
    Timer timer;
    Type obj;                   // obj - объект, который хранит класс
    pthread_mutex_t lock;       // lock - структура для защиты obj от одновременной записи/чтения
    bool use;                   // use - параметр, который определяет инициализирован ли объект obj.
public:
    // Констуктор класса
    Object();
    /*
     * Функция обновляет параметр obj.
     * newObj - параметр типа Type, новое значения для параметра obj.
     */
    void write(Type newObj);
    // Функция возвращает параметр obj
    Type get();
    /*
     * Функция ждёт обновление параметра obj.
     * objNow - параметр типа Type, текущий объект
     */
    Type waitNew(Type objNow);
};

#include "Object.cppd"