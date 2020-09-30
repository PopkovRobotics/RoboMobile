/*
*  Программа работы контроллера на основе Arduino NANO по управлению моделью
*  беспилотного автомобиля SmartCar. Программа взаимодействует с одноплатным компьютером 
*  по последовательному каналу в интерфейсе UART (Rx, Tx) на скорости 115200 бит/с.
*  Программа контролирует уровень напряжения на двух LiPo батареях (номинал 7,4 В) и 
*  включает звуковой сигнал (buzzer), одновременно выключая тяговый мотор.
*  Программа включает на постоянно передние фары и задние габариты, в мигающем режиме правый и 
*  левый указатели поворотов
*  при повороте передних колес вправо или влево соответственно, включает задние стоп-сигналы при 
*  остановке машинки и выключает их при возобновлении движения.
*  Программа взаимодействует с энкодером (оптическим или магнитным) и вычисляет текущую скорость.
*  Угол поворота передних колес задает сервомотор. Скорость вращения задних колес регулируется 
*  широтно-импульсной модуляцией (ШИМ = PWM) питающего напряжения.
*  Программа следит, чтобы напряжение на батареях LiPo на оказалось ниже 5,1 В.
*
*/

// Подключаем библиотеку для работы с сервоприводом
#include <Servo.h>
// Подключаем библиотеку для работы с индикацией модели
#include "indicators.h"
// Подключаем библиотеку для работы с мотором
#include "motor.h"

// Тип энкодера
// Магнитный энкодер
#define POLOLU_MAGNETIC_ENCODER    
// Оптический энкодер           
//#define POLOLU_OPTICAL_ENCODER

// Если раскомментировано, то Arduino будет следить 
// за напряжением штатных LiPo батарей
//#define BATTARY_TEST

#if defined(BATTARY_TEST)
// Пин, к которому подключён делитель напряжения батареи Arduino, Jetson Nano
#define BATTARY1_PIN          A5
// Пин, к которому подключён делитель напряжения батареи мотора
#define BATTARY2_PIN          A6

#define VOLTAGEFILTER         0.3
#define BATTARY1_RK           5.09 * 2
#define BATTARY2_RK           5.09 * 2
#endif

// Пин, к которому подключен энкодер мотора
#define ENCODER_PINA          3
#define ENCODER_PINB          2
// Пин для управления скоростью мотора
#define PWM_PIN               6
// Пин для управления направлением вращения мотора
#define DIR_PIN               5
// Пин, к которому подключён сервопривод
#define SERVO_PIN             9
// Пин, к которому подключены светодиоды передних фар
#define HEAD_LIGH_PIN         A0
// Пин, к которому подключён левый поворотник
#define LEFT_INDICATOR_PIN    A1 
// Пин, к которому подключён правый поворотник
#define RIGHT_INDICATOR_PIN   A2 
// Пин, к которому подключены светодиоды стоп сигнала
#define STOP_INDICATOR_PIN    A3 
// Пин, к которому подключены светодиоды габаритов
#define REAR_LIGHT_PIN        A4

// Тип отправляемой информации на одноплатник
enum DataInfo {
  Distance = 0,               // Пройденное расстояние в см
  BattaryLOW,                 // Батарея разряжена
};

// Угол поворота передних колёс в градусах
int corner = 90, 
// Прошлый угол поворота передних колёс в градусах
  corner_old = 0,
// Скорость движения модели в см/c
  speed = 0,
// Прошлая скорость движения модели в см/с
  speed_old = 0,
// Направление движения модели (вперёд, назад)
  direction = 1,
  regulator = 0,
  regulator_old = 0;

// Посдений полученный символ с одноплатника
char cur_state = 0;
  
float error = 0.0,
  integral = 0.0,
  // Реальная скорость движения модели в сантиметрах в секунду
  real_speed = 0;

// Для реализации таймеров
unsigned long time = 0,
          time_current = 0,
          time_encoder = 0,
          last_speed_update = 0;
          
// Счетчик импульсов энкодера для расчёта скорости модели         
volatile unsigned int encoder0Pulse = 0;
// Счетчик импульсов энкодера для расчёта пройденного расстояния
volatile unsigned long int encoder0TotalPulse = 0;

#if defined(BATTARY_TEST)
// Напряжение на первой батареи
float battary1_u = 7.4,
// Напряжение на второй батареи
  battary2_u = 7.4;

// Время во время последней проверке напряжения на батареях
unsigned long last_battary_test = 0;
#endif
             
// Создам экземпляр класса для работы с сервоприводом
Servo myservo;
// Создам экземпляр класса для работы с мотором
Motor motor(PWM_PIN, DIR_PIN);
// Создам экземпляр класса для работы с световой индикацией
Indicators indicators(LEFT_INDICATOR_PIN, RIGHT_INDICATOR_PIN, 
          HEAD_LIGH_PIN, REAR_LIGHT_PIN, STOP_INDICATOR_PIN);

void setup() { 
  // Устанавливаем скорость отправки/приёма данных на/от одноплатника
  Serial.begin(115200); 
  // Устанавливаем максимальное время ожидания сигнала
  Serial.setTimeout(30);
  
  // Указываем, к какому порту подключен сервопривод модели
  myservo.attach(SERVO_PIN);

  #if defined(BATTARY_TEST)
  pinMode(BATTARY1_PIN, INPUT);
  pinMode(BATTARY2_PIN, INPUT);

  // Получаем напряжения на батареях
  battary1_u = (analogRead(BATTARY1_PIN) * BATTARY1_RK) / 1024.0;
  battary2_u = (analogRead(BATTARY2_PIN) * BATTARY2_RK) / 1024.0;
  #endif

  
  pinMode(ENCODER_PINA, INPUT);
  pinMode(ENCODER_PINB, INPUT);
  // Когда энкодер срабатывает, он отправляет напряжение на порт, к которому подключён
  // Чтобы ловить это напряжение, необходимо использовать аппаратное прерывание
  // Прерывание позволяет ловить изменение напряжение на порту и говорить об этом программе
  // Чтобы активировать прерывание, необходимо использовать функцию attachInterrupt
  attachInterrupt(0,                            // Номер прерывания
                  update_encoder,               // Функция, вызываемая прерыванием
                                                // То есть, когда напряжение на порту измениться он вызовет эту функцию
                                                // Функция должна быть без параметров и не возвращать значений
                  FALLING);                     // Прерывание вызывается только при смене значения на порту с HIGH на LOW
  attachInterrupt(1, update_encoder, FALLING);
  // Более подробно об этой функции можете прочитать на http://arduino.ru/Reference/AttachInterrupt
}

void loop() {
  time_current = millis();
  
  #if defined(BATTARY_TEST)
  // Если прошла 1 секунда, то проверяем напряжение на батареи
  if(time_current - last_battary_test > 1000) {
    battary_test_fnc();
    last_battary_test = millis();
  }
  #endif

  // Устаналиваем угол поворота передних колёс
  // Если используем проверку заряда батареи
  #if defined(BATTARY_TEST)
  if(corner != corner_old && battary1_u > 5.1) 
    myservo.write(corner);
  // В противном случае
  #else
  if(corner != corner_old) 
    myservo.write(corner);
  #endif
  corner_old = corner;

  // Если прошла 1 секунда, с момента последнего получения данных с одноплатника
  if(time_current - time > 1000) {
    speed = 0;   
  }

  // Если скорость является нулевой, то останаливаем модель
  if(speed == 0)
    motor.speed(speed);

  // Вызываем функцию для управления внешней иллюминацией
  indicators.upd_indicators(corner, speed);

  // Получаем параметры движения модели по последовательному порту
  serial_get_data();
  
  if(time_current - last_speed_update > 300) {
    // Рассчитываем реальную скорость движения модели в см/с
    update_speed();

    if(speed != 0) {
      regulator_old = regulator;
      error = speed - real_speed;
      
      integral += error * (float)0.3;
      if(integral > 200) 
        integral = 200;
        
      regulator = (int)(error * (float)2.5 + integral);
      regulator = (regulator > 255 ? 255 : (regulator < 0 ? 0 : regulator)); 
      regulator = (regulator + regulator_old) * (float)0.5;
      
      // Устаналиваем направление движения
      motor.direction(direction ? Forward : Backward);
      
      // Устаналиваем скорость движения
      motor.speed(regulator);
    }
  }
}

// Функция рассчитывает реальную скорость движения модели в см/с
void update_speed() {
  // Рассчитываем реальную скорость движения модели в см/с
  // Если используем оптический энкодер
  #if defined(POLOLU_OPTICAL_ENCODER)
  real_speed = (encoder0Pulse * (float)1.3) / (float)0.3;
  // Если используем магнитный энкодер
  #elif defined(POLOLU_MAGNETIC_ENCODER)
  real_speed = (encoder0Pulse * (float)(22.62/20.0)) / (float)0.3;
  #endif
  
  encoder0Pulse = 0;
  last_speed_update = millis();
}

// Функция отправляет пройденное расстояние модели в см
void send_info(DataInfo data_type) {
  switch(data_type) {
    case Distance: 
    {
      //unsigned long time_now = millis();
      //encoder0TotalPulse++;
      //if(time_now - time_encoder >= 30) {
        // Отправляем пройденное расстояние в см
        Serial.print('F');
        // Если используем оптический энкодер
        #if defined(POLOLU_OPTICAL_ENCODER)
        float distance = (float)encoder0TotalPulse * (float)1.3;
        Serial.print(distance);
        // Если используем магнитный энкодер
        #elif defined(POLOLU_MAGNETIC_ENCODER)
        float distance = (float)encoder0TotalPulse * (float)(22.64 / 20.0);
        Serial.print(distance);
        #endif

        //time_encoder = time_now;
      //}
      break;
    }
    case BattaryLOW:
    {
      // Отправляем информацию о том, что батарея модели разряжена
      Serial.print('B');
      break;
    }
  }
  Serial.print("E");
  Serial.flush();
}

// Функция получает данные с одноплатника
void serial_get_data() {
  
  if (Serial.available() > 0) {
    // Считываем символ
    char c = Serial.read();
       
    switch(cur_state) {
      case 1:
      {
        if(c == 'P') cur_state = 2;
        else cur_state = 0;
        break;
      }
      case 2:
      {
        if(c == 'D') {
          // Получаем угол поворота передних колёс
          corner = Serial.parseInt();
          // Получаем направление движения 
          direction = Serial.parseInt();
          // Получаем скорость движения модели
          speed = Serial.parseInt();

          send_info(Distance);
          
          cur_state = 0;
          time = time_current;
        }else cur_state = 0;
        break;
      }
      default:
      {
        if(c == 'S') cur_state = 1;
        else cur_state = 0;
        break;
      }
    }
  }
}

// Функция обновляет счетчик импульсов энкодера 
void update_encoder() {
  // Счетчик импульсов энкодера для расчёта скорости модели
  encoder0Pulse++;
  // Счетчик импульсов энкодера для расчёта пройденного расстояния
  encoder0TotalPulse++;
}

// Если используем проверку заряда батареи
#if defined(BATTARY_TEST)

void battary_test_fnc() {

  unsigned long time_stop = millis();
  
  // Получаем напряжение с батарей
  get_battary_u();

  // Если уровень напряжения на любой батарее попал в 
  // диапазон 5.15...6.9В, машинка останавливается, внешняя 
  // иллюминация гаснет.
  while((battary1_u > 5.15 && battary1_u < 6.9) || 
        (battary2_u > 5.15 && battary2_u < 6.9)) {

    // Отправляем информацию о том, что батарея модели разряжена
    send_info(BattaryLOW);
    
    // Останаливаем движение модели 
    speed = 0;
    motor.speed(speed);
    
    // Выключаем световую индикацию
    indicators.lights_off();

    // Получаем напряжение с батарей
    get_battary_u();

    delay(20);
  }
  
  // Включаем фонари-габариты и фары
  indicators.start_indicators(true);
}

// Функция получает напряжение батарей
void get_battary_u() {
  float bt1_tmp = (analogRead(BATTARY1_PIN) * BATTARY1_RK) / 1024.0,
    bt2_tmp = (analogRead(BATTARY2_PIN) * BATTARY2_RK) / 1024.0;

  battary1_u = 8.0;
  battary2_u = 8.0;
  
  battary1_u = battary1_u - VOLTAGEFILTER * (battary1_u - bt1_tmp);
  battary2_u = battary2_u - VOLTAGEFILTER * (battary2_u - bt2_tmp);
}
#endif
