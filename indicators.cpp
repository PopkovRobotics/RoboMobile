#include "indicators.h"

// Конструктор класса
Indicators::Indicators(int left_indicator, int right_indicator, 
    int head_ligh, int rear_ligh, int stop_indicator) {
      
  // Пины для управления световой индикацией
  this->left_indicator = left_indicator;
  this->right_indicator = right_indicator;
  this->head_ligh = head_ligh;
  this->rear_ligh = rear_ligh;
  this->stop_indicator = stop_indicator;

  pinMode(left_indicator, OUTPUT);
  pinMode(right_indicator, OUTPUT);
  pinMode(head_ligh, OUTPUT);
  pinMode(rear_ligh, OUTPUT);
  pinMode(stop_indicator, OUTPUT);

  // Включаем фары и фонари-габариты
  digitalWrite(head_ligh, HIGH);
  digitalWrite(rear_ligh, HIGH);
  // Выключаем поворотники и стоп сигнал
  digitalWrite(left_indicator, LOW);
  digitalWrite(right_indicator, LOW);
  digitalWrite(stop_indicator, LOW);

  // Таймеры для для работы поворотников
  this->right_time_indicator = 0;
  this->left_time_indicator = 0;

  // Флаги для работы поворотников
  this->turn_right_light = false;
  this->turn_left_light = false;
}

// Функция включает/выключает фары и фонари-габариты
void Indicators::start_indicators(bool on) {
  // Если флаг on является true, то включаем индикацию
  if(on) {
    // Включаем фары и фонари-габариты
    digitalWrite(head_ligh, HIGH);
    digitalWrite(rear_ligh, HIGH);
  } else {
    // Выключаем фары и фонари-габариты
    digitalWrite(head_ligh, LOW);
    digitalWrite(rear_ligh, LOW); 
  }
}

// Функция выключает всю световую индекацию
void Indicators::lights_off() {
  // Выключаем поворотники
  digitalWrite(left_indicator, LOW);
  digitalWrite(right_indicator, LOW);
  // Выключаем фары
  digitalWrite(head_ligh, LOW);
  // Выключаем фонари-габариты
  digitalWrite(rear_ligh, LOW);
  // Выключаем стоп индекацию
  digitalWrite(stop_indicator, LOW);
}

// Функция управляет светофовй индекацией модели,
// в зависимости от угла поворота передних колёс и скорости модели
void Indicators::upd_indicators(int corner, int speed) {

  unsigned long time_current = millis() + 1;
  
  // Если скорость движения модели является нулевой,
  // то включаем стоп сигнал
  if(speed == 0)
    digitalWrite(stop_indicator, HIGH);
  // В противном случае выключаем стоп сигнал
  else
    digitalWrite(stop_indicator, LOW);

  // Если угол corner передних колес от 
  // положения "прямо" больше DEVIATION (порога),
  // включается правый индикатор поворота
  if(corner <= (90 - DEVIATION)) {
    if(!turn_right_light) {
      if(right_time_indicator == 0) {
        turn_right_light = true;
        right_time_indicator = time_current;
        
        // Включаем правый поворотник
        digitalWrite(right_indicator, HIGH);
      }
    }
  }
      
  if(turn_right_light && time_current - right_time_indicator > 2 * TURN_SIGNAL_FREQ) {
        turn_right_light = false;
        right_time_indicator = 0;
        
        // Выключаем правый поворотник
        digitalWrite(right_indicator, LOW);
  }else if(turn_right_light && time_current - right_time_indicator > TURN_SIGNAL_FREQ) {
      // Выключаем правый поворотник
      digitalWrite(right_indicator, LOW);       
  }
  
  // Если угол отклонения corner передних колес 
  // от положения "прямо" больше "90 + DEVIATION" (порога),
  // включается левый индикатор поворота
  if(corner >= (90 + DEVIATION)) {
    if(!turn_left_light) {
      if(left_time_indicator == 0) {
        turn_left_light = true;
        left_time_indicator = time_current;
        Serial.println("HIGH left");
        Serial.println(turn_left_light);
        Serial.println(left_time_indicator);
        // Включаем левый поворотник
        digitalWrite(left_indicator, HIGH);
      }
    }
  }
    
  if(turn_left_light && time_current - left_time_indicator > 2 * TURN_SIGNAL_FREQ) {
    turn_left_light = false;
    left_time_indicator = 0;
        
    // Выключаем правый поворотник
    digitalWrite(left_indicator, LOW);
  }else if(turn_left_light && time_current - left_time_indicator > TURN_SIGNAL_FREQ) {        
      // Выключаем левый поворотник
      digitalWrite(left_indicator, LOW);   
      Serial.println("LOW left");    
  }
}
