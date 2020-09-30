#pragma once
#include <Arduino.h>

// Значение отклонения от угла поворота 90, 
// при котором будут включены поворотники
#define DEVIATION             14
// Продолжительность свечения/выключенности поворотников 
// в миллисекундах. (По полсекунды вкл/выкл).
#define TURN_SIGNAL_FREQ      500

class Indicators {
private:
  // Пин, к которому подключён левый поворотникs
  int left_indicator,
  // Пин, к которому подключён правый поворотник
    right_indicator,
  // Пин, к которому подключены светодиоды стоп сигнала
    stop_indicator,
  // Пин, к которому подключены светодиоды передних фар
    head_ligh,
  // Пин, к которому подключены светодиоды габаритов
    rear_ligh;

  // Время, которое включен правый поворотник в миллисекундах
  unsigned long right_time_indicator,
  // Время, которое включен левый поворотник в миллисекундах
    left_time_indicator;

  // Флаг, определяющий включен ли правый поворотник
  bool turn_right_light,
  // Флаг, определяющий включен ли левый поворотник
      turn_left_light;
    
public:
  // Конструктор класса
  Indicators(int left_indicator, int right_indicator, 
      int head_ligh, int rear_ligh, int stop_indicator);

  // Функция включает/выключает фары и фонари-габариты
  void start_indicators(bool on = true);

  // Функция выключает всю световую индекацию
  void lights_off();

  // Функция управляет светофовй индекацией модели,
  // в зависимости от угла поворота передних колёс и скорости модели
  void upd_indicators(int corner, int speed);
  
};
