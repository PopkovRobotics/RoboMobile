#include "Decoder.hpp"

// Функции класса Decoder
/*
 * Конструктор класса
 * path - параметр типа string, путь до конфигурационного файла
 * system - ссылка на параметр типа System, класс, 
 * в который будут записываться данные из .json файла
 */
Decoder::Decoder(std::string path, System& system) {
    reader = builder.newCharReader();
    bool ok;
    root = decode(path, ok);
    if(ok) {
        // Устаналиваем порт, к которому подключена Arduino
        system.setArduinoPort((char*)std::string(root["arduino_port"].asString()).c_str());
        // Устаналиваем размер кадра
        Point size_frame(root["frame_size"]["width"].asInt(),
                    root["frame_size"]["height"].asInt());
        system.frame_size.write(size_frame);
        // Устаналиваем скорость для модели
        system.normal_speed.write(root["speed"].asInt());
        // Порт сервера
        system.server_port.write(root["server_port"].asInt());
        //
        system.model_audio.write(root["mobile_audio"].asInt());
    }
}
// Дескриптор класса
Decoder::~Decoder() {
    delete reader;
}
/*
 * Функция декодирует файл .json
 * json_file - параметр типа string, путь до .json файла,
 * status - ссылка на параметр типа bool, в который будет записываться 
 * статус выполнения кода.
 */
Json::Value Decoder::decode(std::string json_file, bool& status) {
    Json::Value json_data;
    // Открываем .json файл
    std::ifstream file(json_file, std::ifstream::binary);
    std::string file_string((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();
    // Декодируем .json файл
    std::string errors;
    status = reader->parse(file_string.c_str(), file_string.c_str() + file_string.size(), &json_data, &errors);
    // Проверяем удачно ли декодировали файл, если нет, то выводим ошибку
    if (!status) {
        std::string error = "Error parsing .json file from path " + json_file;
        ERROR(error.c_str());
    }
    return json_data;
}