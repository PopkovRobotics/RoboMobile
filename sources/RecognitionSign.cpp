#include "RecognitionSign.hpp"

#define PRINT_SIGN(x, y, z)             \
    if(x != y) {                        \
        INFO(z);                        \
    }

// Функции
/*
 * Функция возвращает расстояние от чёрной линии до вебкамеры модели.
 * line_width - параметр типа int32_t, ширина чёрной линии в пикселях.
 */
float lineDistance(int32_t line_width){
    return (float)VALUE / (float)line_width;
}
/*
 * Функция возвращает расстояние от знака до вебкамеры модели.
 * width - параметр типа int32_t, ширина знака в пикселях.
 */
float signDistance(int32_t width) {
    return (float)SIGN_VALUE / (float)width;
}
/*
 * Функция реализует "алгоритм жука" для поиска и описания контуров изображения.
 * Функция возвращает массив с минимальной обрамляющей контуров.
 * image - ссылка на параметр типа Mat, изображения для поиска контуров,
 * roi - параметр типа Rect, координаты области, в которой ведётся поиск контуров,
 * min_rect - параметр типа Point, минимальнй размер контура. 
 * Если найденный контур маньше размера min_rect, то минимальная обрамляющая контура 
 * не добавляется в вектор.
 * uint8_t - параметр типа uint8_t, значение для определения чёрного и белого цвета. 
 * Для поиска контуров на изображении необходимо разделить пикселя изображения на 2 группы
 * (1 группа - чёрные пиксели, а вторая белые пикселя).
 * Если одна из компонент пикселя меньше treshold, то пиксель считается чёрным, в противном случае белым.
 */
std::vector<Rect> findRects(Mat image, Rect roi, Point min_rect, uint8_t treshold) {
    Point start_point,                          // Координаты первого чёрного пикселя
            now_point;                          // Координаты текущего пикселя
    std::vector<uint8_t> pixel;                 // Хранит значение трёх компонент пикселя
    std::vector<Rect> rects;                    // Вектор с минимальными обрамляющими
    int8_t direction = 0,                       // Ориентация направления перехода на соседний пиксель
                                                // 0 - направо
                                                // 1 - вниз
                                                // 2 - налево
                                                // 3 - вверх
        bias = 0;

    bool is_right,                              // Переменная определяет, на какой пиксель переходим
                                                // false - на левый
                                                // true - на правый
        is_continue = false;

    // Если область для поиска объектов выходит за рамки изображения,
    // то выходим из функции
    if((roi.x >= image.size().x || roi.y >= image.size().y) || 
       ((roi.x + roi.width) > image.size().x || (roi.y + roi.height) > image.size().y))
        return rects;

    // Крайние точки изображения меняются на белые
    for(int32_t x = 0; x < roi.width; x++) {
        // Рисуем горизонтальные белые линии вверху и внизу изображения
        image.setPixel(Point(roi.x + x, roi.y), 255);
        image.setPixel(Point(roi.x + x, (roi.y + roi.height) - 1), 255);
    }
    for(int32_t y = 0; y < roi.height; y++) {
        // Рисуем вертикальные белые линии слева и справа
        image.setPixel(Point(roi.x, roi.y + y), 255);
        image.setPixel(Point((roi.x + roi.width) - 1, roi.y + y), 255);
    }

    // Идем до тех пор, пока не встретим черный пиксель
    for (start_point.y = 0; start_point.y < roi.height; start_point.y++) {
        for (start_point.x = 0; start_point.x < roi.width; start_point.x++) {

            for(uint32_t n = 0; n < rects.size(); n++) {
                if(rects[n].x <= start_point.x && rects[n].y <= start_point.y && 
                   (rects[n].width + rects[n].x) >= start_point.x && (rects[n].height + rects[n].y) >= start_point.y) {
                    is_continue = true;
                    break;
                }
            }

            if(is_continue) {
                is_continue = false;
                continue;
            }

            // Получаем значение цвета пикселя
            pixel = image.at<uint8_t>(Point(roi.x + start_point.x, roi.y + start_point.y));
            // Если пиксель чёрного цвета
            if (!(pixel[0] > treshold && pixel[1] > treshold && pixel[2] > treshold))
                break;
        }

        // Если встретили белый пиксель, то переходим на правый пиксель, в противном случае 
        // на левый. Продолжаем выполнять переходы, пока не дойдём до первого чёрного пикселя.
        // Если встречен пиксель чёрного цвета
        if (start_point.x != roi.width) {
            // Переходим на левый пиксель
            Rect rect;
            rect.x = start_point.x;
            rect.y = start_point.y;
            now_point.x = start_point.x;
            now_point.y = start_point.y - 1;
            direction = 3;

            // Пока не придем в исходную точку, выделяем контур объекта
            while (!(now_point == start_point)) {

                // Получаем значение цвета пикселя
                pixel = image.at<uint8_t>(Point(roi.x + now_point.x, roi.y + now_point.y));
                // Если пиксель чёрного цвета, то поворачиваем налево
                if(!(pixel[0] > treshold && pixel[1] > treshold && pixel[2] > treshold)) {
                    is_right = false;          // Налево
                }
                // В противном случае поворачиваем направо
                else is_right = true;          // Направо

                
                // От строки 118 до 152 реализуется переход на левый либо на правый пиксель,
                // в зависимости от ориентации.
                if(is_right) 
                    bias = 1;
                else 
                    bias = -1;
        
                switch(direction) {
                    // Вправо
                    case 0:
                    {
                        now_point.y += bias;
                        break;
                    }
                    // Вниз
                    case 1:
                    {
                        now_point.x -= bias;
                        break;
                    }
                    // Влево
                    case 2:
                    {
                        now_point.y -= bias;
                        break;
                    }
                    // Вверх
                    case 3:
                    {
                        now_point.x += bias;
                        break;
                    }
                }

                if(is_right)
                    direction = ((direction + 1) > 3 ? 0 : (direction + 1));
                else 
                    direction = ((direction - 1) < 0 ? 3 : (direction - 1));
        
                // Ищем крайние границы минимальной обрамляющей
                if(rect.width < now_point.x) rect.width = now_point.x;
                if(rect.height < now_point.y) rect.height = now_point.y;
                if(rect.x > now_point.x) rect.x = now_point.x;
                if(rect.y > now_point.y) rect.y = now_point.y;
            }

            // Вычисляем ширину и высоту минимальной обрамляющей
            rect.width -= rect.x; 
            rect.height -= rect.y;

            // Если размер минимальной обрамляющей равен или больше размера min_rect,
            // то добавляем минимальную обрамляющую в вектор
            if(rect.width >= min_rect.x && rect.height >= min_rect.y) {
                rects.push_back(rect);
            } 
        }
    }   

    return rects;
}
/*
 * Функция вычесляет процентное соотношение красной, жёлтой, синий, черной
 * компоненты в области.
 * image - ссылка на параметр типа Mat, изображение, на котором вычисляем соотношение.
 */
std::vector<float> getPercent(Mat& image){
    // Возвращает количество красного, жёлтого, синего и чёрного цвета в процентах, на изображении image
	std::vector<float> percents = {0, 0, 0, 0};
    // Считаем количество пикселей красного, жёлтого, синего, чёрного цвета
	for(int32_t y = 0; y < image.size().y; y++){
		for(int32_t x = 0; x < image.size().x; x++){
            switch(image.color(Point(x, y), BGR)) {
                case Red:
                {
                    percents[0]++;
                    break;
                };
                case Yellow:
                case Green:
                {
                    percents[1]++;
                    break;
                };
                case Blue:
                {
                    percents[2]++;
                    break;
                };
                case Black:
                {
                    percents[3]++;
                    break;
                };
                default:
                {
                    break;
                }
            }
		}
	}
    // Узнаём процентное соотношение цветов
	float count = image.size().x * image.size().y;
    percents[0] /= count;
	percents[1] /= count;
	percents[2] /= count;
	percents[3] /= count;
	return percents;
}
/*
 * Функция для создания потока распознавания дорожных знаков на изображении.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* RecognitionSignFnc(void* ptr) {
    INFO("Recongition sign thread was starting.");
	System &system = *((System*)ptr);
    // Экземпляр класса, в который будем записывать полученное изображение с вебкамеры
    Mat frame;
    // Распознанный дорожный знак
    SignData sign_detect;
    // Класс для работы с таймером
    Timer robo_timer;
    // Получаем размер картинки
    Point frame_size = system.frame_size.get();
    // Вычисляем область для распознавания зелёной части стойки знака
    std::vector<Point> line_scan;
	for(int x = 350; x < 600; x++){
		line_scan.push_back(Point(x, frame_size.y - ((float)KOEF * x + (float)BIAS)));
	}

    // Область интереса, в которой распознаём светофоры
    Rect trlight_rect(440,                          // Координата x верхнего левого угла
                    205,                            // Координата y верхнего левого угла
                    200,                            // Ширина области интереса
                    140);                           // Высота области интереса

    while(!system.program_end.get()) {
        robo_timer.start();
        // Получаем изображение с вебкамеры
        frame = system.frame.waitNew(frame);
        if(frame.empty()) continue;
        // Проверяем, полностью ли зелёный прямоугольник находится внутри области интереса
		int count_left = 0, count_right = 0;
		for(uint32_t i = 0; i < 7; i++)
            if(frame.color(line_scan[i], BGR) != Green) count_left++;

		for(uint32_t i = line_scan.size() - 7; i <= line_scan.size() - 1; i++)
            if(frame.color(line_scan[i], BGR) != Green) count_right++;

		if(count_left > 4 && count_right > 4) {
            // Ищем края зелёной стойки знака
            int count_green = 0;
            Point border_right, border_left(1000, 1000);
            for(unsigned int i = 0; i < line_scan.size(); i++){
			    if(frame.color(line_scan[i], BGR) == Green){
				    count_green++;
				    if(border_left.x > line_scan[i].x)
					    border_left = line_scan[i];

				    if(border_right.x < line_scan[i].x)
					    border_right = line_scan[i];
			    }
		    }
        // Если ширина зелёной стойки больше 55 пикселей, то продолжаем распознавание знака
		if(count_green >= 55){

                // Находим координаты верхнего края зелёной стойки
                Point center_rect((border_right.x + border_left.x) / 2, (border_right.y + border_left.y) / 2), 
                    top_rect;
			    for(int y = center_rect.y; y >= 0; y--){
				    if(frame.color(Point(center_rect.x, y), BGR) != Green){
					    top_rect.x = center_rect.x; 
                        top_rect.y = y;
					    break;
				    }
			    }   

                uint32_t width = (border_right.x - border_left.x) + 12;
                Rect rect_sign(border_left.x - 6, top_rect.y - (width + 7), width, width);

                // Находим границы знака
                std::vector<Rect> rects = findRects(frame, rect_sign, Point(10, 10), 80);

                // Ищем самую большую обрамляющую
                uint32_t max = 0, 
                        id = 0;
                for(uint32_t i = 0; i < rects.size(); i++) {
                    uint32_t area = (rects[i].width * rects[i].height);
                    if(area > max) {
                        max = area;
                        id = i;
                    }
                }
                SignData old_recognition = system.sign.get();
                rects[id].x += rect_sign.x;
                rects[id].y += rect_sign.y;
                Rect sign_clarify = rects[id];
                Mat sign_roi = frame(rects[id]);
                std::vector<float> colors = getPercent(sign_roi);
		        /*
                 * colors[0] - доля пикселей красного цвета,
                 * colors[1] - доля пикселей жёлтого цвета,
                 * colors[2] - доля пикселей синего цвета,
                 * colors[3] - доля пикселей чёрного цвета.
                 */
                // Знаки жёлтого и красного цвета
                if(colors[3] < (float)0.15 && colors[2] < (float)0.15) {
                    // Знак стоп
                    if(colors[0] > (float)0.45 && colors[1] < (float)0.20) {
                        sign_detect.sign = stop_s;
                        PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Sign Stop recognized.");
                        sign_detect.distance = signDistance(sign_clarify.width);
                        sign_detect.area = sign_clarify;
                    }

                    // Знак главная дорога
                    else if(colors[0] < (float)0.1 && colors[1] > (float)0.1){
                        sign_detect.sign = mainroad_s;
                        PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Sign Main Road recognized.");
                        sign_detect.distance = signDistance(sign_clarify.width);
                        sign_detect.area = sign_clarify;
                    }
                }
                // Знаки синего цвета
                else if(colors[3] < (float)0.01) {
                    // Считаем количество белых пикселей по краям знака
                    int all_count = 0;
                    // Верхний левый угол
                    for(int32_t y = 2; y < (5 + 2); y++) 
                        for(int32_t x = 2; x < (5 + 2); x++)
                            if(sign_roi.color(Point(x, y), BGR) == White)
                                all_count++;
                    // Верхний правый угол
                    for(int32_t y = 2; y < (5 + 2); y++)
                        for(int32_t x = (sign_clarify.width - 7); x < (sign_clarify.width - 2); x++)
                            if(sign_roi.color(Point(x, y), BGR) == White)
                                all_count++;
                    // Нижний левый угол
                    for(int32_t y = (sign_clarify.height - 7); y < (sign_clarify.height - 2); y++)
                        for(int32_t x = 2; x < (5 + 2); x++)
                            if(sign_roi.color(Point(x, y), BGR) == White)
                                all_count++;
                    // Нижний правый угол
                    for(int32_t y = (sign_clarify.height - 7); y < (sign_clarify.height - 2); y++)
                        for(int32_t x = (sign_clarify.width - 7); x < (sign_clarify.width - 2); x++)
                            if(sign_roi.color(Point(x, y), BGR) == White)
                                all_count++;
                    // Знак парковки
                    if(all_count < 60) {
                        sign_detect.sign = parking_s;
                        PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Sign Parking recognized.");
                        sign_detect.distance = signDistance(sign_clarify.width);
                        sign_detect.area = sign_clarify;
                    }else{
                        // Знаки со стрелками
                        double width_sign = ((double)sign_clarify.width / (double)59.0) * (double)12.0,
                                height_sign = ((double)sign_clarify.height / (double)59.0) * (double)14.0,
                                x_sign = ((double)sign_clarify.width / (double)59.0) * (double)12.0,
                                y_sign = ((double)sign_clarify.height / (double)59.0) * (double)36.0;
                        Rect rect_left(x_sign, y_sign, width_sign, height_sign), 
                                rect_right(sign_clarify.width - (x_sign + width_sign), y_sign, width_sign, height_sign),
                                rect_center((sign_clarify.width / 2) - (width_sign / 2), y_sign, width_sign, height_sign);

                        int count_left = 0, count_right = 0, count_center = 0;
                        // Правый прямоугольник
                        for(int32_t y = rect_left.y; y < (rect_left.y + rect_left.height); y++) {
                            for(int32_t x = rect_left.x; x < (rect_left.x + rect_left.width); x++) {
                                if(sign_roi.color(Point(x, y), BGR) == White){
                                    count_left++;
                                }
                            }
                        }
                        // Левый прямоугольник
                        for(int32_t y = rect_right.y; y < (rect_right.y + rect_right.height); y++) {
                            for(int32_t x = rect_right.x; x < (rect_right.x + rect_right.width); x++) {
                                if(sign_roi.color(Point(x, y), BGR) == White){
                                    count_right++;
                                }
                            }
                        }
                        // Центральный прямоугольник
                        for(int32_t y = rect_center.y; y < (rect_center.y + rect_center.height); y++) {
                            for(int32_t x = rect_center.x; x < (rect_center.x + rect_center.width); x++) {
                                if(sign_roi.color(Point(x, y), BGR) == White){
                                    count_center++;
                                }
                            }
                        }

                        // Стрелка налево
                        if((float)(count_center + count_right) * (float)0.7 < count_left) {
                            sign_detect.sign = right_s;
                            PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Left turn sign recognized.");
                            sign_detect.distance = signDistance(sign_clarify.width);
                            sign_detect.area = sign_clarify;
                        }
                        // Стрелка направо
                        else if((float)(count_center + count_left) * (float)0.7 < count_right) {
                            sign_detect.sign = left_s;
                            PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Right turn sign recognized.");
                            sign_detect.distance = signDistance(sign_clarify.width);
                            sign_detect.area = sign_clarify;
                        }
                        // Стрелка прямо
                        else if((float)(count_left + count_right) * (float)0.7 < count_center) {
                            sign_detect.sign = top_s;
                            PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Traffic sign straight recognized.");
                            sign_detect.distance = signDistance(sign_clarify.width);
                            sign_detect.area = sign_clarify;
                        }
                    }
                }
            }
        }

        // Преведённый алгоритм умеет распознавать знаки стоп, 
        // главная дорога, парковка, и стрелки прямо, налево, направо

        // РАСПОЗНАВАНИЕ СВЕТОФОРОВ

        LineInfo line_info = system.line.get();
        // Если перед моделью находится стоп-линия
        if(line_info.stop_line) {
		    // Область, в которой находится светофор
            Rect tr_roi;
            // tr_roi.y - координата y, верхнего левого края светофора
            // tr_roi.x - координата x, верхнего левого края светофора
            // tr_roi.height - высота светофора в пикселях
            // tr_roi.width - ширин светофора в пикселях

            // Находим нижнюю y координату светофора

            // Предпологаем, на какой высоте примерно находиться светофор
			tr_roi.height =  line_info.y_2 - 30;

            // Проходимся по изображению вверх, пока не найдём чёрный пиксель
			for(; tr_roi.height > 0; tr_roi.height--) {
                // Если нашли чёрный пиксель
				if(frame.color(Point(line_info.x_2 + 60, tr_roi.height), BGR) == Black) break; }

            // Проверяем на какой высоте находится нижний края светофора
            // Если координата слишком маленькая, то это значит, что светофор не найден
			if(tr_roi.height > 240) {

                // Ищем края чёрной бленды светофора

                // Находим координату x левого края бленды светофора
			    tr_roi.x = line_info.x_2 + 60;
                tr_roi.width = tr_roi.x;
			    for(; tr_roi.x > 0; tr_roi.x--) {
                    // Если нашли белый пиксель
                    if(frame.color(Point(tr_roi.x, tr_roi.height - 5), BGR) == White) break; }

                // Находим координату x правого края бленды светофора
			    for(; tr_roi.width < 640; tr_roi.width++) {
                    // Если нашли белый пиксель
                    if(frame.color(Point(tr_roi.width, tr_roi.height - 5), BGR) == White) break; }
		
                // Находим координату y верхнего края бленды
			    tr_roi.y = tr_roi.height - 5;
			    for(; tr_roi.y > 0; tr_roi.y--) {
                    // Если нашли белый пиксель
                    if(frame.color(Point(tr_roi.width - 5, tr_roi.y), BGR) == White) break; }

                // Рассчитывем ширину и высоту бленды светофора в пикселях
                // Для нахождения ширины из координаты х правого края вычитаем координату x левого края
                // А для нахождения высоты из координаты y нижнего края вычитаем координату y верхнего края
                tr_roi.width -= tr_roi.x;
                tr_roi.height -= tr_roi.y;

                // ********* ОПРЕДЕЛЯЕМ СИГНАЛ СВЕТОФОРА *********
                // Сигнал светофора определяется по положению, то есть цвет сигнала не учитывается.

                const int column = tr_roi.width * 0.5;
                // Количество белых пикселей
                int32_t white_pixels = 0,
                // Сумма порядковых номеров белых пикселей
                        white_pixels_offset = 0; 
                for (int32_t offset = 0; offset < tr_roi.height; offset++) {
                    std::vector<uint8_t> pixel = frame.at<uint8_t>(Point(tr_roi.x + column, tr_roi.y + offset));
                    if ((pixel[2] + pixel[1] + pixel[0]) >= 130) {
                        white_pixels++;
                        white_pixels_offset += offset;
                    }
                }

                // Если количество белых пикселей в строке меньше 10, то
                // останаливаем определения сигнала светофора
                if(white_pixels <= 10) continue;
                // Координата y, зажжённого сигнала
                float signal_center = white_pixels_offset / white_pixels,
                    light_position = signal_center / tr_roi.height;
                // Переменная lightPosition, определяет положение зажжённого сигнала 
                // светофора (0.7 - зеленый, 0.46 желтый, 0.34 - красножелтый, 0.23 - красный)

                SignData old_recognition = system.sign.get();

                // Зелёный сигнал светофора
                if(light_position >= 0.6) {
                    sign_detect.sign = tr_green_s; 
                    PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Green traffic light recognized.");
                    sign_detect.area = tr_roi;
                }
                // Жёлтый сигнал светофора
                else if(light_position >= 0.4) {
                    sign_detect.sign = tr_yellow_s; 
                    PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Yellow traffic light recognized.");
                    sign_detect.area = tr_roi;
                }
                // Красный сигнал светофора
                else if(light_position >= 0.1) {
                    sign_detect.sign = tr_red_s; 
                    PRINT_SIGN(sign_detect.sign, old_recognition.sign, "Red traffic light recognized.");
                    sign_detect.area = tr_roi;
                }
		    }
        }

        robo_timer.stop();
        // Получаем время, затраченное на обнуружение и распознавания дорожного знака
        long spend_time = robo_timer.millis();

        // Записываем распознанный знак для дальнейшей обработки
        if(sign_detect.sign != none_s) {
            system.sign.write(sign_detect);
        }
        // Если знак не был распознан, то проверяем, какой сейчас знак действует.
        // Если действующий знак не является none_s, то смотрим, как давно он был распознан.
        else{
            SignData cur_sign = system.sign.get();
            if(cur_sign.sign != none_s) {
                cur_sign.detect_time += spend_time;
                if(cur_sign.detect_time >= 30) {
                    system.sign.write(SignData());
                }else{
                    system.sign.write(cur_sign);
                }
            }
        }

        sign_detect = SignData();
    }
    INFO("RecognitionSign is stoped.");
    return NULL;
}
