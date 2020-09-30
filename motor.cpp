#include "motor.h"

// Конструктор класса
Motor::Motor(int pwm_motor,int dir_motor) {
  // Пин управления скоростью мотора
  this->pwm_motor = pwm_motor;
   // Пин управления направлением движения мотора
  this->dir_motor = dir_motor;
  // Направление движения модели
  this->dir = Forward;
  
  pinMode(pwm_motor, OUTPUT); 
  pinMode(dir_motor, OUTPUT);
  
  // Выключаем движение мотора
  digitalWrite(pwm_motor, LOW);
  digitalWrite(dir_motor, LOW);
}

// Функция меняет направление движения модели
void Motor::direction(Direction dir) {
  if(this->dir != dir) {
    // Выключаем движение мотора
    digitalWrite(pwm_motor, LOW);
    digitalWrite(dir_motor, LOW);
    
    // Меняем направление движения мотора
    int tmp = dir_motor;
    dir_motor = pwm_motor;
    pwm_motor = tmp;
       
    this->dir = dir;
  }
}

// Функция устаналивает новую скорость движения мотора
void Motor::speed(int speed_motor) {
  analogWrite(pwm_motor, speed_motor);
}
