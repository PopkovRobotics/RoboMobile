/*
 * Программа реализует движение модели по чёрной линии, через перекрёстки и стоп-линии. 
 * Также программа выделяет знаки на изображнии.
 * Автор: Одышев А.С. 10.07.2020
 */
// Библиотеки C++
#include <csignal>
#include <unistd.h>
#include <pthread.h>
// Файлы программы
#include "Loop.hpp"
#include "Video.hpp"
#include "Server.hpp"
#include "Config.hpp"
#include "Arduino.hpp"
#include "Decoder.hpp"
#include "RecognitionLine.hpp"
#include "RecognitionSign.hpp"

#define CONFIG_FILE "../configs/config.json"

static bool program_end = false;

void signal_upd(int signal) {
  // Если пользователь нажал сочетание клавиш CRTL+С, то
  // устаналиваем флаг для завершения программы в true
  program_end = true;
}

int main() {
  System system;
  // Загружаем файл конфигурации
  Decoder decoder(CONFIG_FILE, system);
  // Выводим основную информацию о модели
  system.print();

  // Устаналиваем, то что при нажатие сочетания клавиш CRTL+C вызывалась функция signal_upd
  std::signal(SIGINT, signal_upd);

  // Поток чтения видео с Вебкамеры
	pthread_t CaptureThr;
	pthread_create(&CaptureThr, NULL, CaptureFnc, &system);

  // Поток нахождения линии на изображение, по которой двигается модель
	pthread_t RecognitionLineThr;
	pthread_create(&RecognitionLineThr, NULL, RecognitionLineFnc, &system);
	
  // Поток распознавания дорожных знаков на изображение
	pthread_t RecognitionSignThr;
	pthread_create(&RecognitionSignThr, NULL, RecognitionSignFnc, &system);

  // Поток расчёта параметров движения модели (скорость, угол поворота передних колёс и т.п.)
	pthread_t LoopThr;
	pthread_create(&LoopThr, NULL, LoopFnc, &system);

  // Поток "общения" с Arduino
	pthread_t ArduinoThr;
	pthread_create(&ArduinoThr, NULL, ArduinoFnc, &system);

  // Поток отправки изображения с камеры, информации о линии движения и распознанных знаков на клиент
	pthread_t ServerThr;
	pthread_create(&ServerThr, NULL, ServerFnc, &system);

  //pthread_t WriterThr;
  //bool video_flag = system.video_params.get().write_video;
  //if(video_flag) {
  //	  pthread_create(&WriterThr, NULL, VideoWriter, &system);
  //}

  while(true) {
    if(program_end) {
      system.program_end.write(true);
      break;
    }
  }

	// Ждём окончания всех потоков
	pthread_join(CaptureThr, NULL);
  pthread_join(RecognitionLineThr, NULL);
  pthread_join(RecognitionSignThr, NULL);
  pthread_join(LoopThr, NULL);
  //if(video_flag) {
  //	  pthread_join(WriterThr, NULL);
  //}
  pthread_join(ArduinoThr, NULL);
  pthread_detach(ServerThr);
  //pthread_join(CaptureThr, NULL);
  //pthread_join(ServerThr, NULL);
  return 0;
}
