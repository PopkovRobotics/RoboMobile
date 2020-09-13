#include "Video.hpp"

/*
 * Функция для создания потока чтения кадров с вебкамеры.
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* CaptureFnc(void* ptr) {
    INFO("Capture thread is started.");
    System &system = *((System *)ptr);
    // Таймер для расчёта ФПС
    Timer timer_fps;
    // Переменная, которая хранит значение ФПС
    uint16_t fps_count;
    // Получаем размер кадра
    Point frame_size = system.frame_size.get();  
    // Создаём экземпляр класса для хранения кадра с вебкамеры
    Mat capture_mat(Point(frame_size.x, frame_size.y),          // Размер матрицы (640 - ширина, 480 - высота)
                    UC8_3);                                     // Тип матрицы (на каджый пиксель даётся 1 байт)
                                                                // Минимальное значение пикселя 0, а максимальное 255
    #ifdef DEBUG_VIDEO
    cv::VideoCapture cap("/home/artemy/Документы/stop_i/111.avi");
    #else
    // Начинаем работу с вебкамерой
    Webcam webcam("/dev/video0", Point(640, 480));
    #endif
    // Запускаем таймер
    timer_fps.start();
    while(!system.program_end.get()) {
        #ifdef DEBUG_VIDEO
        cv::Mat image_opencv;
        cap.read(image_opencv);
        if(image_opencv.empty()) 
            continue;
        cv::resize(image_opencv, image_opencv, cv::Size(frame_size.x, frame_size.y));
        std::vector<uint8_t> data;
        for(int32_t i = 0; i < (frame_size.x * frame_size.y * 3); i++) {
            data.push_back(image_opencv.at<uint8_t>(i));
        }
        capture_mat.set(data.data());
        #else
	capture_mat.set(webcam.frame());
        #endif
        
        // Записываем считанный кадр
        system.frame.write(capture_mat);

        #ifdef DEBUG_VIDEO
        //cv::Mat img_cv(cv::Size(frame_size.x, frame_size.y), CV_8UC3, capture_mat.data());
        //cv::imshow("frame", img_cv);
        //cv::waitKey(5);
        usleep(20000);
        #endif

        // Проверяем таймер, если прошла одна секунда, то выводим
        // количество ФПС и обнуляем таймер, в противном случае прибавляем единицу
        // к сётчику ФПС
        timer_fps.stop();
        if(timer_fps.millis() >= 1000) {
            INFO(("FPS count: " + std::to_string(fps_count)).c_str());
            fps_count = 0;
            timer_fps.start();
        }else fps_count++;
    }
    INFO("Capture thread is stoped.");
    return NULL;
}
