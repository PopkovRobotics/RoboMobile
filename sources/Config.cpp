#include "Config.hpp"

Object<bool> is_play;

// Функции класса System
// Конструктор класса
System::System() {
    // Устаналиваем стандартные значения для параметров модели
    // Размер кадра
    frame_size.write(Point(640, 480));
    // Скорость модели
    normal_speed.write(30);
    // Порт Arduino
    snprintf(arduino_port, 60, "/dev/ttyS3");
    // Сервер порта
    server_port.write(1111);
    // Флаг аудио оповещений
    model_audio.write(true);
    // Флаг завершения программы
    program_end.write(false);

    is_play.write(false);

    lock_port = PTHREAD_MUTEX_INITIALIZER;
}
// Выводим основную информацию о модели
void System::print() {
    printf("Model information\n");
    // Выводим информацию о модели
    // Порт, к которому подключена Arduino
    printf("Arduino port: %s\n", arduino_port);
    // Скорость движения модели
    printf("Normal speed: %dsm/s\n", normal_speed.get());
    // Сервер порта
    printf("Server port: %d\n", server_port.get());
    // Флаг аудио оповещений
    printf("Audio mobile flag: %s\n", (model_audio.get() ? "true" : "false"));
    // Размер кадра
    Point size = frame_size.get();
    printf("Frame size:\n"
        "\tWidth: %dpx\n"
        "\tHeight: %dpx\n", 
        size.x, size.y);
}
/*
 * Функция записывает в destination, значение параметра arduino_port.
 * destination - массив типа char, в который будет 
 * записываться  значение параметра arduino_port.
 */
void System::getArduinoPort(char* destination) {
	pthread_mutex_lock(&(lock_port));
	snprintf(destination, 60, arduino_port);
	pthread_mutex_unlock(&(lock_port));
}
/*
 * Функция устаналивает новое значения для параметра arduino_port.
 * destination - массив типа char, новое значение для arduino_port.
 */
void System::setArduinoPort(char* source) {
    pthread_mutex_lock(&(lock_port));
	snprintf(arduino_port, 60, source);
    pthread_mutex_unlock(&(lock_port));
}

/*
 * Функция проигрывает аудиофайл в формате .wav.
 * audio_file - параметр типа string, путь до аудиофайла.
 */
bool System::play_audio(std::string audio_file) {
    // Если флаг использования аудио оповещений является true, 
    // то воспроизводим файл
    if(model_audio.get()) {
        this->audio_file = audio_file;
	if(!is_play.get()) {
		pthread_create(&thread_new, NULL, audio_thread, static_cast<void*>(&this->audio_file));
		return true;
	}
    }

    return false;
}

void* audio_thread(void* ptr) {
    is_play.write(true);
    std::string &path = *(static_cast<std::string*>(ptr));
    // Для проигрывания аудиофайлов используется утилита aplay
    // --device=surround50:CARD=Generic_1,DEV=0 - указываем через какое устройство воспроизводим аудиофайл
    // --file-type wav - указываем, то что проигрываем аудиофайл в формате .wav
    // path - путь до аудиофайла для воспроизведения
    std::system((std::string("aplay --device=hw:CARD=Device,DEV=0 --file-type wav ") + path).c_str());
    is_play.write(false);
    return NULL;
}
