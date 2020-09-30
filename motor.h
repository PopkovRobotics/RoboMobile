#pragma once
#include <Arduino.h>

// Перечесление, в котором хранится возможные направления
// движения модели
enum Direction {
  Forward = 0,            // Движение модели вперёд
  Backward,               // Движение модели назад
};

class Motor {
private:
  // Пин управления скоростью мотора
  int pwm_motor,
  // Пин управления направлением движения мотора
    dir_motor;

  // Направление движения модели
  Direction dir;
public:
  // Конструктор класса
  Motor(int pwm_motor,int dir_motor);
  
  // Функция меняет направление движения модели
  void direction(Direction dir);
  
  // Функция устаналивает новую скорость движения мотора
  void speed(int speed_motor);
};
