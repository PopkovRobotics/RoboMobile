#include "Server.hpp"

// Функции класса Server
/*
 * Конструктор класса.
 * system - ссылка на параметр типа System, для получения порта сервера.
 */
Server::Server(System& system) {
    connected = false;
    server_port = system.server_port.get();
	// Запускаем сервер
    start();

    // Отправляем размер кадра
    Point frame_size = system.frame_size.get();
    sendCmd(FrameSize_t, (uint32_t)sizeof(Point), (void*)(&frame_size));
}
// Функция запсукает сервер
void Server::start() {
	// Запускаем сервер
	// Если клиент подключён, то выходим из функции
	if(connected) return;
	socklen_t cli_len;
	struct sockaddr_in serv_addr, 
                    cli_addr;    

	// Создаём сокет для сервера
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// Если возникла ошибка создания сокета, то выходим из функции
	if (sockfd < 0) {
        ERROR("Can`t create socket for server. Restart program!");
        return;
    }

    int32_t opt_val = 1;
	size_t opt_len = sizeof(opt_val);
	if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt_val, opt_len) < 0) {
        ERROR("Can`t set socket options KEEPALIVE. Restart program!");
        return;
    }
    opt_val = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, opt_len) < 0) {
        ERROR("Can`t set socket options REUSEADDR. Restart program!");
        return;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(server_port);

	// Запускаем сервер
	while(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        sleep(1); 
	// Прослушиваем сервер на входящие соеденения 
	listen(sockfd, 5);
	cli_len = sizeof(cli_addr);
	new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
	// Если подключился клиент
	INFO("Client was connecting.");
	connected = true;
}
// Функция останаливает работу сервера
void Server::stop() {
    close(sockfd);
    close(new_sockfd);
}
/*
 * Функция отправляет значение, которое хранит указатель source на клиент.
 * source - указатель типа void, отправляемая информация. 
 * size - параметр типа size, размер отправляемых данных в байтах.
 */
bool Server::sendMsg(void* source, size_t size) {
    // bytes - переменная, которая хранит размер данных в байтах, которые
    // отправили на клиент
    int bytes = 0;      
	for (size_t i = 0; i < size; i += bytes) {
		if ((bytes = send(new_sockfd, ((uint8_t *)source) + i, size - i, MSG_NOSIGNAL)) <= 0) {
			ERROR("Sending data error.");
			return false;
		}
	}
	return true;
}
/*
 * Функция отправляет команду на клиент в формате:
 * тип команды, размер команды, команды.
 * type - параметр типа DataType, тип команды (см. Перечисление DataType).
 * data_size - параметр типа uint32_t, размер отправляемой команды.
 * ptr - указатель типа void, отправляемая информация.
 */
void Server::sendCmd(DataType type, uint32_t data_size, void *ptr) {
    bool status = true;
    uint32_t type_int = type;
    // Тип команды
	status = (sendMsg((void *)&type_int, sizeof(uint32_t)) ? status : false);
    // Размер команды
    status = (sendMsg((void *)&data_size, sizeof(uint32_t)) ? status : false);
    // Команда
    status = (sendMsg(ptr, (size_t)data_size) ? status : false);	

	// Если данные не отправились, то выводим ошибку
	if (!status){
		ERROR("Client disconnected.");
		connected = false;
        // Перезапускаем сервер
		stop();
        start();
	}
}
// Функция возвращает статус подключения клиента к серверу
bool Server::clientState() {
    return connected;
}

/*
 * Функция для создания потока сервера.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* ServerFnc(void *ptr) {
    INFO("Server thread was starting.");
    System &system = *((System *)ptr);
    // Создаём экземпляр класса для работы с сервером
    Server server(system);
    // Создаём экземпляр класса для хранения кадра с вебкамеры
    Mat frame;
	while(!system.program_end.get()) {
		
		// Получаем изображение с камеры
		frame = system.frame.waitNew(frame);
		// Если изображение пустое, то возвращаемся в начало цикла
        if(frame.empty()) 
            continue;

		// Отправляем изображение с камеры
		server.sendCmd(Image_t,                                                         // Тип команды (Отправляем изображение с вебкамеры)
                    (uint32_t)(frame.size().x * frame.size().y * frame.dims()),         // Размер изображения в байтах
                    (void *)(&frame.data()[0]));                                        // Изображение с вебкамеры

        // Отправляем распознанный знак 
        SignData sign = system.sign.get();
        if(sign.sign != none_s) {
            server.sendCmd(Sign_t,                                                      // Тип команды (Отправляем изображение с вебкамеры)
                        (uint32_t)sizeof(SignData),                                     // Размер изображения в байтах
                        (void *)(&sign));                                               // Распознанный дорожный знак
        }   


        // Отправляем информацию о направляющей линии
        LineInfo line = system.line.get();
        if(line.center > -1) {
            server.sendCmd(Line_t,                                                      // Тип команды (Отправляем изображение с вебкамеры)
                        (uint32_t)sizeof(LineInfo),                                     // Размер изображения в байтах
                        (void *)(&line));                                               // Информация о направляющей линии
        }

        // Отправляем информацию о движение модели
        Engine engine = system.engine.get();
        		server.sendCmd(Engine_t,                                                 // Тип команды (Отправляем изображение с вебкамеры)
                    (uint32_t)sizeof(Engine),                                            // Размер изображения в байтах
                    (void *)(&engine));                                                  // Информация о движение модели
	
	}

	printf("Server thread is stoped.");
	return NULL;
}
