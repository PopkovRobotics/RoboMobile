#pragma once
// Библиотеки C++
#include <jsoncpp/json/json.h>
#include <fstream>
// Файлы программы
#include "Mat.hpp"
#include "Config.hpp"
#include "LogInfo.hpp"

class Decoder {
private:
    Json::Value root;                   // root - класс, в котором находится декодированный .json файл
    Json::CharReaderBuilder builder;
    Json::CharReader * reader;          // reader - класс для декодирования .json файлов

    /*
     * Функция декодирует файл .json
     * json_file - параметр типа string, путь до .json файла,
     * status - ссылка на параметр типа bool, в который будет записываться 
     * статус выполнения кода.
     */
    Json::Value decode(std::string json_file, bool& status);

public:
    /*
     * Конструктор класса
     * path - параметр типа string, путь до конфигурационного файла
     * system - ссылка на параметр типа System, класс, 
     * в который будут записываться данные из .json файла
     */
    Decoder(std::string path, System& system);
    // Дескриптор класса
    ~Decoder();

};
