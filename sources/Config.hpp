#pragma once
// Библиотеки C++
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <vector>
// Файлы программы
#include "Mat.hpp"
#include "Signs.hpp"
#include "Lines.hpp"
#include "Object.hpp"
#include "Engine.hpp"

class System {
private:
	char arduino_port[60];					// arduino_port - массив, в котором хранится порт,
											// к которому подключена Arduino
	pthread_mutex_t lock_port;				// lock_port - структура для защиты arduino_port от одновременного использования

	pthread_t thread_new;

	std::string audio_file;

public:
	Object<Mat> frame;						// frame - кадр считанный с вебкамеры
	Object<Point> frame_size;				// frame_size - размер кадра считанного с вебкамеры
	Object<LineInfo> line;					// line - структура, в которой храниться 
											// информация о линии, по которой двигается модель
	Object<Engine> engine;					// engine - структура, в которой хранится информация о модели.
											// Такой как, скорость, угол поворота передних колёс и т.п.
	Object<SignData> sign;					// sign - распознанный дорожный знак
	Object<uint8_t> normal_speed;			// normal_speed - скорость в см/с, с которой двигается модель
	Object<uint32_t> server_port;			// server_port - порт, на котором будет запускаться сервер
	Object<bool> program_end,				// program_end - флаг, определяющий остановку программы
				model_audio;				// model_audio - флаг, определяющий использовать ли аудио оповещение в программе

	// Конструктор класса
	System();
	// Выводим основную информацию о модели
    void print();
	/*
	 * Функция записывает в destination, значение параметра arduino_port.
	 * destination - массив типа char, в который будет 
	 * записываться  значение параметра arduino_port.
	 */
	void getArduinoPort(char* destination);
	/*
	 * Функция устаналивает новое значения для параметра arduino_port.
	 * destination - массив типа char, новое значение для arduino_port.
	 */
	void setArduinoPort(char* source);

	/*
 	 * Функция проигрывает аудиофайл в формате .wav.
 	 * audio_file - параметр типа string, путь до аудиофайла.
 	 */
	bool play_audio(std::string audio_file);
};

void* audio_thread(void* ptr);
