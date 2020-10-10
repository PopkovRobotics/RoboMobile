#include "Arduino.hpp"
#include <iostream>

// Функции класса ArduinoCtrl
/*
 * Конструктор класса, в котором подключаемся к Arduino
 * system - ссылка на параметр типа System, для получения порта,
 * к которому подключена Arduino.
 */
ArduinoCtrl::ArduinoCtrl(System& system) {
	arduino_fd = -1;
	// Получаем порт, к которому подключена Arduino
	char arduino_port[60];
	system.getArduinoPort(arduino_port);
	// Открываем соединение с Arduino
	arduino_fd = open(arduino_port, O_RDWR | O_NOCTTY/* | O_NDELAY*/ | O_NONBLOCK);
	if (arduino_fd < 0) {
        ERROR("Can't open serial port.");
		connection = false;
		return;
	}

	usleep(1000000);

	// Задаём параметры для "общения" с Arduino
	struct termios options;                     
	tcgetattr(arduino_fd, &options);	
	// Скорость отправки/чтения данных (115200 бод)							
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CREAD | CLOCAL;
	options.c_cflag &= ~OPOST;

	options.c_lflag = 0;

	options.c_cc[VMIN] = 1;
	options.c_cc[VTIME] = 0;

	// Устаналиваем параметры для "общения" с Arduino
	tcsetattr(arduino_fd, TCSANOW, &options);

	tcflush(arduino_fd, TCIOFLUSH);
	tcflush(arduino_fd, TCIFLUSH);

	usleep(500000);

	connection = true;
	return;
}
/* 
 * Функция для отправки сообщения на Arduino.
 * message - констатный параметр типа char*, сообщение для отправки,
 * size - параметр типа size_t, размер отправляемого сообщения.
 */
void ArduinoCtrl::SendCommand(const char* message, size_t size) {
	// Отправляет команду на Arduino
	int bytes = write(arduino_fd, message, size);
    // Если данные отправились не польностью, то выводим ошибку
	if (bytes < (int)size) 
        ERROR("Arduino sending data error.");
	ioctl(arduino_fd, TCSBRK, 1);
	return;
}
/*
 * Функция получает информацию с Arduino и возвращает её.
 */
ArduinoData ArduinoCtrl::Feedback() {
	ArduinoData arduino_data;
	char char_status;
    while(true) {
		char_status = 0;
        int bytes = read(arduino_fd, &char_status, 1);

        if(bytes > 0) {
			switch(char_status) {
               	// Получаем пройденное расстояние с Arduino
                case 'F':
                {
                    // Пример сообщения о дитсанции, которое Arduino отправляет:
                    // F10E, F - начальный символ для того, чтобы понять какие данные пришли,
                    // 10 - пройденная дистанция в см., E - закрывающий символ для окончания чтения дистанции.

                    char buffer[1023];
                    int i = 0;
                    bool end_read = false;
                    for(; !end_read; i++){
                        bytes = read(arduino_fd, &char_status, 1);
                        // Если пришёл символ, закрывающий чтение дистанции, то выходим из цикла
                        if(bytes > 0 && char_status == 'E') end_read = true;
                        else if(bytes > 0) buffer[i] = char_status;
						else if (bytes <= 0) end_read = true;
                    }
                    buffer[i + 1] = '\0';

                    double distance = std::atof(buffer);

                    if(distance > arduino_data.distance) arduino_data.distance = distance;
                    break;
                }
                // Получаем информацию о том, что батарея разряжена
                case 'B':
                {
                    bytes = read(arduino_fd, &char_status, 1);
                    if(bytes > 0 && char_status == 'E') arduino_data.charge_bat = false;
                    break;
                }
            }
        }else break;
    }
    return arduino_data;
}
// Функция для очищения буфера общения с Arduino
void ArduinoCtrl::Clean() {
	// Очищаем буфер общения с Arduino, посредством считывания 
    // всех данных из буфера
	char data;
    while(true) {
	if(read(arduino_fd, &data, 1) <= 0) break;
    }
}
// Функция для закрытия соеденения с Arduino
void ArduinoCtrl::DeInit() {
	// Отключаемся от Arduino, если Arduino подключена
	if (connection)
		connection = close(connection) == 0 ? true : false;
	return;
}
// Функция, которая возвращает статус подключения к Arduino.
bool ArduinoCtrl::IsConnected() {
	return connection;
}

/*
 * Функция для создания потока общения с Arduino.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* ArduinoFnc(void* ptr) {
    INFO("Arduino thread was starting.");
	System &system = *((System *)ptr);
	// Создаём экземпляр класса для "общения" с Arduino 
	ArduinoCtrl controller(system);
    // Структур, которая хранит информацию о модели (скорость, угол поворота передних колёс и т.п.)
	Engine engine;
    // Массив, который хранит отправляемое сообщение на Arduino
	char message[128];
	// Если не получилось подключиться к Arduino, то выводим ошибку и завершаем работы потока
	if (!controller.IsConnected()) {
        ERROR("Arduino isn't attached.");
		return NULL;
	}
    INFO("Arduino is attached.");

    // Код выполняется в цикле, пока не появится сигнал завершения программы.
    // То есть, пока функция is_end() возвращает false, то код выполняется, в противном случае,
    // выходим из цикла
	while (!system.program_end.get()) {
		// Получаем информацию о движении модели
		engine = system.engine.get();

		// Создаём команду для отправки на Arduino
        // Пример созданной команды: SPD 90,1,30,0
		snprintf(message, sizeof(message), "SPD %d,%d,%d ", 
				engine.angle, 			// Угол поворота качалки сервопривода (90 - колёса стоят прямо, если угол меньше 90, то колёса 
                                    	// поварачиваются направо, а если больше 90, то колёса поворачиваются налево)
				engine.direction, 		// Направление  движения (true - вперёд, false - назад)
				engine.speed); 			// Скорость модели в см/c (минимальная скорость 30см/c, а максимальная 60см/c)

		// Отправляем команду на Arduino
		controller.SendCommand(message, strlen(message));

		// Получаем информацию с Arduino
		ArduinoData arduino_data = controller.Feedback();

		if(arduino_data.distance > 0.0) {
			engine.distance = arduino_data.distance;
		}

		// Обновляем дистанцию в структуре
        system.engine.write(engine);
		// Задержка для отправки/приёма данных на/c Arduino в микросекундах
		usleep(10000);
	}
	// отключаемся от Arduino и завершаем поток
	controller.DeInit();
    INFO("Exit in arduino thread.");
	return NULL;
}
