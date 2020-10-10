#include "RecognitionLine.hpp"

// Функции
/*
 * Функция, которая проверяет является ли пиксель черным.
 * Для определения чёрного цвета, используется только красная компонента.
 * frame - ссылка на параметр типа Mat, изображение, на котором находиться 
 * проверяемый пиксель,
 * point - параметр типа Point, координаты проверяемого пикселя 
 * (point.x - x координата, point.y - y координата),
 * range - параметр типа uint8_t, значение красной компоненты, при которой 
 * функция считает пиксель чёрным.
 */
bool isBlack(Mat& frame, Point point, uint8_t range) {
    std::vector<uint8_t> pixel = frame.at<uint8_t>(point);
    if(pixel[2] <= range)
        return true;
    return false;
}
/*
 * Функция для нахождения чёрной линии на изображение.
 * frame - ссылка на параметр типа Mat, изображение, на котором ищем линию,
 * line - ссылка на параметр типа LineInfo, в который будем записывать 
 * информацию о распознанной линии.
 * scan_row - параметр типа int, номер строки, на котором ищем линию.
 */
void RecognitionLine(Mat& frame, LineInfo& line, int scan_row) {
    int border_right = 0,
        border_left = 0,
        closest_center = -1,
		closest_width = -1;

	line.old_width = line.width;

	// Распознавание линии по всему кадру
	if (line.old_center == -1) {
		line.center = -1;
		for (int32_t i = 0; i < frame.size().x; i++) {
			if (isBlack(frame, Point(i, scan_row), 40)) {
				int32_t startPoint = i,
				 		nBlackPoints = 3;
				while (nBlackPoints > 1 && i + 3 < frame.size().x) {
					i += 3;
					nBlackPoints = 0;
					for (int j = 0; j < 3; ++j)
						nBlackPoints += int(isBlack(frame, Point(i + j, scan_row), 40));
				}
				if (i - startPoint < 10)
                    continue;
				else {
					line.center = (i + startPoint) / 2;
					line.width = i - startPoint;
                    if (abs(320 - line.center) < abs(320 - closest_center)) {
						closest_center = line.center;
						closest_width = line.width;
					}
				}
			}
		}
		line.center = closest_center;
		line.width = closest_width;
	}
	// Распознавание линии по старому центру
	else{
		if (isBlack(frame, Point(line.old_center, scan_row), 40)) {
			int32_t i,
				nBlackPoints;
			for (i = line.old_center + 1; i + 3 < frame.size().x; i += 3) {
				nBlackPoints = 0;
				for (int j = 0; j < 3; ++j)
					nBlackPoints += int(isBlack(frame, Point(i + j, scan_row), 40));
				if (nBlackPoints <= 1)
					break;
			}
			border_right = i;

			for (i = line.old_center - 1; i >= 3; i -= 3) {
				nBlackPoints = 0;
				for (int j = 0; j < 3; ++j)
					nBlackPoints += int(isBlack(frame, Point(i - j, scan_row), 40));
				if (nBlackPoints <= 1)
					break;
			}
			border_left = i;

			line.center = (border_left + border_right) / 2;
			line.width = border_right - border_left;
			if (abs(border_right - border_left) < 25) {
				line.center = -1;
				line.old_center = -1;
				return;
			}
		}else{
			line.center = -1;
			line.old_center = -1;
			return;
		}
	}

	if(line.center != -1) {
		if(line.max_difference < line.width || line.max_difference == 0) {
			line.max_difference = line.width;
			line.x_2 = border_right;
			line.y_2 = scan_row;
		}
	}

	line.old_center = line.center;
}
/*
 * Функция для создания потока нахождение линии на изображение.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* RecognitionLineFnc(void* ptr) {
    INFO("Recongition line thread was starting.");
    System &system = *((System*)ptr);
    // Структура, в которой находится информация о главной линии (по которой двигаемся)
    LineInfo main_line, 
			lines;          
    // Класс, в будет храниться изображение
    Mat frame; 
	// Структура, определяющая тип дороги перед моделью
	RoadType road_type = Unknown;

	Sign sign_handle;

	int scan_row = SCAN_ROW,					// Номер строки на изображение, на которой введётся распознавание чёрной линии
		road_distance = 0,						// Размер перекрёстка в см.
		fork_start = 0,
		crossing_start = 0;						

	bool direction = false,						// Флаг, определяющий в каком направление
												// должна двигаться модель на развилке
												// (true - направо, false - налево).
		rescan_line = true,
		crossing_line = false,					// Флаг, определяющий есть ли перед моделью пересечение двух чёрных линий
		green_tr_found = false;

	while (!system.program_end.get()) {
        // Получаем изображение с камеры
	    frame = system.frame.waitNew(frame);
        if(frame.empty()) continue;

        // Обнуляем переменные
		// Устаналиваем центр, к которому стремиться преблизиться модель
		main_line.set_point = 320;
		lines.max_difference = 0;

		if(road_type != Unknown) 
			main_line.old_center = -1;

		// Ищем центр чёрной линии на изображение

		if(rescan_line) RecognitionLine(frame, main_line, scan_row);

		for(int k = 0; k <= 100; k+=5) {
        	RecognitionLine(frame, 					// Изображение, на котором введётся распознавание чёрной линии
							lines, 					// Структура, в которую будет записываться информация о чёрной линии
							SCAN_ROW - k);			// Номер строки на изображение, на которой введётся распознавание чёрной линии
		}

		// Номер строки на изображение, на которой введётся распознавание чёрной линии
		scan_row = SCAN_ROW;
		rescan_line = true;

		Engine engine = system.engine.get();

		// Если двигаемся по перекрёстку
		// Останаливаем таймер
		if(road_type != Unknown && (engine.distance - fork_start) < (road_type == CrossRoad ? 10 : 40)) { 
			if(road_type == CrossRoad) {
				scan_row = 420;
			}else if(!direction) rescan_line = false;
		}else if(road_type != Unknown && (engine.distance - fork_start) > road_distance) {
			if(road_type == CrossRoad) green_tr_found = false;

			// Размер перекрёстка в см.
			road_distance = 0;
			// Тип дороги
			road_type = Unknown;

			fork_start = 0;

			sign_handle = none_s;
		}

		// Если перед машинкой находиться пересечение двух линией
		// Останаливаем таймер
		if(crossing_line && (engine.distance - crossing_start) < 5)
			scan_row = 440;
		else if(crossing_line && (engine.distance - crossing_start) > 5) { 
			crossing_line = false;
			crossing_start = 0;
		}

		// Для определения стоп линии
		if(lines.max_difference > 150 && !green_tr_found) {
			// Флаг, определяющий наличие стоп линии перед машинкой
			main_line.stop_line = true;
			main_line.x_2 = lines.x_2;
			main_line.y_2 = lines.y_2;
		}else{
			// Флаг, определяющий наличие стоп линии перед машинкой
			main_line.stop_line = false;
		}

		SignData sign = system.sign.get();
		if(sign.sign == left_s || sign.sign == top_s || sign.sign == right_s) {
			sign_handle = (Sign)sign.sign;
		}

		if(sign.sign == tr_green_s) green_tr_found = true; 

		if(main_line.old_width != 0 && main_line.width != 0) {
			// Пересечение двух линий на центральном переркёстке
			if(main_line.width > 220) {
				// Номер строки на изображение, на котором введётся распознавание чёрной линии
				scan_row = 440;
				// Устаналиваем значение для флага, определяющего наличие пересечений двух линий 
				crossing_line = true;

				crossing_start = engine.distance;
				continue;
			}
			// Центральный перекрёсток
			else if(main_line.width > 140 &&  (main_line.width - main_line.old_width) > 25 && road_type == Unknown) {
                INFO("Crossroad detected.");
				// Номер строки на изображение, на котором введётся распознавание чёрной линии
				scan_row = 420;
				// Тип дороги
				road_type = CrossRoad;
				// Размер перекрёстка в см.
				road_distance = CROSSROAD_SIZE;
				// Направление движения на перекрёстке
				direction = false;

				fork_start = engine.distance;
				continue;
			}
			// Развилка
			else if(main_line.width > 70 && (main_line.width - main_line.old_width) > 15 && road_type == Unknown) {
                INFO("Fork detected.");
				// Тип дороги
				road_type = Fork;
				// Размер перекрёстка в см.
				road_distance = FORK_SIZE;
				// Направление движения на перекрёстке
				direction = ((sign_handle == left_s || sign_handle == top_s) ? false : true);
				rescan_line = false;

				fork_start = engine.distance;
			}
		}


		// Если двигаемся по перекрёстку, то находим координаты правого или левого края чёрной линии
		if(road_type != Unknown) {
			int white_points = 0;
			// Если двигаемся по перекрёстку направо
			if(direction) {
				main_line.set_point = 320;
				for(int i = main_line.center; i <= 637; i+=3) {
					white_points = 0;
					for (int j = 0; j < 3; ++j) {
						std::vector<uint8_t> pixel = frame.at<uint8_t>(Point(i + j, scan_row));
						white_points += int(pixel[2] > 70);
					}
                    if(white_points > 2) {
						main_line.center = i + 1;
						main_line.old_center = main_line.center;
						break;
					}
                }
			}
			// Если двигаемся по перекрёстку налево
			else{
				main_line.set_point = 280;
				for(int i = main_line.center; i >= 3; i-=3) {
					white_points = 0;
					for (int j = 0; j < 3; ++j) {
						std::vector<uint8_t> pixel = frame.at<uint8_t>(Point(i - j, scan_row));
						white_points += int(pixel[2] > 40);
					}
                    if(white_points > 2) {
						main_line.center = i + 1;
						main_line.old_center = main_line.center;
						break;
					}
                }
			}
		}
		
        // Записываем информацию о положении главной линии (по которой двигаемся)
        system.line.write(main_line);
	}
	INFO("Recognition line thread was stoping.");
	return NULL;
}
