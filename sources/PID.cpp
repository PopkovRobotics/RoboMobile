#include "PID.hpp"

// Функция класса PID
// Конструктор класса
PID::PID(float kp, float kd, float ki, 
    float set_point,
    std::pair<float, float> output_border,
    std::pair<float, float> integral_border) {
    // Устаналиваем начальные значения для параметров
    this->kp = kp;
    this->kd = kd;
    this->ki = ki;
    this->set_point = set_point;
    this->output_border = output_border;
    this->integral_border = integral_border;
    this->integral = 0;
    this->prev = 0;
}
/*
 * Функция ограничивает входное значение input в пределах border.
 * input - параметр типа float, входное значение для ограничения,
 * border - параметр типа std::pair<float, float>, значения для ограничения (1 значение - минимальная граница, 2 значение - максимальная граница).
 */
float PID::constrain(float input, std::pair<float, float> border) {
    return (input > border.second ? border.second : (input < border.first ? border.first : input));
}
/*
 * Функция вычисляет корректировку входного значения для приближения к идеальному.
 * input - параметр типа float, входное значения для корректировки.
 */
float PID::calc(float input) {
    //if(kp == 0 || kd == 0 || ki == 0)
    //    WARNING("One or more coefficients are equal to zero");

    float error = set_point - input;    // Вычесляем разницу между входным и идеальным значением
    float delta = prev - input;        // Вычесляем разницу между ошибкой и прошлой ошибкой расчётов

    // Выводим информацию, если находимся в режиме дебага кода
    #ifdef DEBUG_PID
    printf("Input data: %f\nSet point: %f\n", input, set_point);
    printf("Error: %f\nDelta: %f\n", error, delta);
    #endif

    // Обновляем прошлую ошибку
    prev = input;
    output = 0;
    // Вычесляем пропорциональная составляющая
    output += error * kp;
    // Вычесляем дифференциальная составляющая
    output += delta * kd;
     // расчёт интегральной составляющей
    integral += error * ki;
    // Ограничиваем выход регулятора
    integral = constrain(integral, integral_border);
    output += integral;
    // Ограничиваем выход регулятора
    output = constrain(output, output_border);

    // Выводим информацию, если находимся в режиме дебага кода
    #ifdef DEBUG_PID
    printf("PID: %f\n", output);
    #endif
    return output;
}
