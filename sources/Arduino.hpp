#pragma once
// Библиотеки C++
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
// Файлы программы
#include "Engine.hpp"
#include "Config.hpp"
#include "LogInfo.hpp"

struct ArduinoData {
    double distance;
    bool charge_bat;
    
    // Конструктор класса
    ArduinoData(double distance = 0.0, bool charge_bat = true) {
        this->distance = distance;
        this->charge_bat = charge_bat;
    }
};

// Класс ArduinoCtrl реализует "общения" с Arduino
class ArduinoCtrl {
private:
	// arduino_fd - параметр типа int, сокет для общения с Arduino. 
	int arduino_fd;			
	// connection - параметр типа bool, статус подключения к Arduino 
	// (true - Arduino поделючена, false - Arduino не подключена).
	bool connection;
public:
	/*
 	 * Конструктор класса, в котором подключаемся к Arduino
 	 * system - ссылка на параметр типа System, для получения порта,
 	 * к которому подключена Arduino.
 	 */
	ArduinoCtrl(System& system);
	/* 
 	 * Функция для отправки сообщения на Arduino.
 	 * message - констатный указатель на параметр типа char, сообщение для отправки,
 	 * size - параметр типа size_t, размер отправляемого сообщения.
 	 */
	void SendCommand(const char* message, size_t size);
    // Функция получает информацию с Arduino и возвращает ее.
	ArduinoData Feedback();
	// Функция для очищения буфера общения с Arduino
	void Clean();
	// Функция для закрытия соеденения с Arduino
	void DeInit();
	// Функция, которая возвращает статус подключения к Arduino.
	bool IsConnected();
};

/*
 * Функция для создания потока общения с Arduino.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* ArduinoFnc(void* ptr);
