#pragma once
//#define DEBUG_PID                   // Этот дефайн отвечает за режим дебага кода для класса PID.
                                    // НЕ трогайте этот дефайн.
// Библиотеки C++
#include <utility>
// Файлы программы
#include "Timer.hpp"
#include "LogInfo.hpp"

// Класс реализует работу PID регулятора
class PID {
private:
    float integral,       // integral - вспомогательная переменная, которая хранит рассчитанный интеграл
        prev,             // prev - прошлая ошибка расчёта регулятора
        output;           // output - выходное значение регулятора

public:
    float kp,           // kp - пропорциональный коэффициент регулятора
        kd,             // kd - дифференциальный коэффициент регулятора
        ki,             // ki - интегральный коэффициент регулятора
        set_point;      // set_point - значение, к которому стремится приблизиться регулятор
    std::pair<float, float> output_border,      // output_border - ограничение выходного значения регулятора
                            integral_border;    // integral_border - ограничение выходного значение интеграла

    // Конструктор класса
    PID(float kp = 0, float kd = 0, float ki = 0, 
        float set_point = 0, 
        std::pair<float, float> output_border = std::make_pair<float, float>(0, 0),
        std::pair<float, float> integral_border = std::make_pair<float, float>(0, 0));
    /*
     * Функция вычисляет корректировку входного значения для приближения к идеальному.
     * input - параметр типа float, входное значения для корректировки.
     */
    float calc(float input);
    /*
     * Функция ограничивает входное значение input в пределах border.
     * input - параметр типа float, входное значение для ограничения,
     * border - параметр типа std::pair<float, float>, значения для ограничения (1 значение - минимальная граница, 2 значение - максимальная граница).
     */
    float constrain(float input, std::pair<float, float> border);
};
