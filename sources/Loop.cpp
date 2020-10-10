#include "Loop.hpp"

/*
 * Функция для создания расчёта параметров движения модели
 * ptr - указатель на параметр типа void, для получения основной информации о модели. 
 */
void* LoopFnc(void* ptr) {
    INFO("LoopFnc thread was starting.");
    System &system = *((System *)ptr);
    Timer tr_timer;                                         // Таймер красного сигнала светофора
    bool start = true;
    bool tr_timer_started = false,                          // Флаг, определяющий запущен ли таймер для красного сигнала светофора.
        hold = false;                                       // Флаг, определяющий начинать ли удерживать скорость
    // Индекс действия в векторе holding_step, которое сейчас выполняется
    uint32_t step = 0;
    // Вектор с действиями на распознанный знак
    std::vector<Hold> holding_step;
    // Создаём экземпляр класса для PID регулятора
    PID pid(KP, KD, KI, 320,
        std::make_pair<float, float>(-RANGE, RANGE),
        std::make_pair<float, float>(-50, 50));
    while(!system.program_end.get()) {
        LineInfo line = system.line.get();
        Engine engine = system.engine.get();

        if(line.center != -1) {
            // Рассчитываем угол поврота передних колёс
            pid.set_point = line.set_point;
            float result = pid.calc(line.center);
            // Ставим угол поворота передних колёс
            engine.angle = (uint8_t)((float)((float)90.0 + result));
            // Ограничиваем угол поворота передних колёс
            pid.constrain(engine.angle, std::make_pair<float, float>(55, 125));
        }

        // Устаналиваем скорость движения модели
        if(start) {
		    engine.speed = system.normal_speed.get();
		    start = false;
	    }

        // Получаем распознанный знак
        SignData sign_handle = system.sign.get();

        // Удержание скорости в течение времени или расстояния
		// Включено ли удерживание определенной скорости
        if (hold) {
            if(holding_step.size() <= 0) {
                step = 0;
                hold = false;
            }else{
                if(holding_step[step].hold_start) {
                    if(holding_step[step].hold_by_time) holding_step[step].hold_timer.start();
                    else holding_step[step].start_distance = engine.distance;
                    holding_step[step].hold_start = false;
                }

                // Останаливаем таймер
			    if(holding_step[step].hold_by_time) holding_step[step].hold_timer.stop();
                // Если прошло время удерживания скорости
                // или проехали расстояние в течение которого удерживаем скорость
			    if ((holding_step[step].hold_by_time && holding_step[step].hold_timer.millis() >= holding_step[step].hold_for) ||
                   (!holding_step[step].hold_by_time && (engine.distance - holding_step[step].start_distance) >= holding_step[step].hold_for)) {
			        if((holding_step.size() - 1) > step) step++;
                    else{
                        step = 0;
                        hold = false;
                        holding_step.clear();
			            engine.speed = system.normal_speed.get();
                    }
			    }
                // Если ещё нужно удерживать скорость
                else engine.speed = holding_step[step].hold_speed; 
            }
		}
        // Если удержание скорости не включенно 
        else{
            switch(sign_handle.sign) {
                // Если увидели запрещающий сигнал светофора
                case tr_red_s:
                case tr_yellow_s:
                case tr_yellowred_s:
                {
                    // Если запрещающий сигнал светофора и таймер красного светофора не запущен
                    if(!tr_timer_started) {
                        // Запускаем таймер
                        tr_timer.start();                                        
			            tr_timer_started = true;  
                    }
		            engine.speed = 0;
                    break;
                }
                // Если увидели зелёный сигнал светофора
                case tr_green_s:
                {
                    // Запускаем движение робота
				    engine.speed = system.normal_speed.get();   
                    hold = true;
                    holding_step.push_back(
                        Hold(true, 100, system.normal_speed.get()));
                    tr_timer_started = false;
                    break;
                }
                // Если увидели знак стоп
                case stop_s:
                {
                    hold = true;

                    // Для того, чтобы доехать до знака
                    holding_step.push_back(
                        Hold(false, sign_handle.distance, system.normal_speed.get()));

		            holding_step.push_back(
			            Hold(true, 2000, 0));

                    // Воспроизводим файл с оповещением о знаке стоп
                    system.play_audio("../audio/stop.wav");
                    break;
                }
                // Если увидели знак главная дорога
                case mainroad_s:
                {
                    hold = true;

                    if(sign_handle.distance >= 20) {
                        holding_step.push_back(
                            Hold(false, sign_handle.distance - 20, system.normal_speed.get()));
                    }

                    holding_step.push_back(
                        Hold(false, 40, system.normal_speed.get() + 15));

                    // Воспроизводим файл с оповещением о знаке главная дорога
                    system.play_audio("../audio/main_road.wav");
                    break;
                }
                // Если увидели знак парковка или знаки со стрелками
                case top_s:
                case left_s:
                case right_s:
                case parking_s:
                {
                    hold = true;

                    // Для того, чтобы доехать до знака
                    holding_step.push_back(
                        Hold(false, sign_handle.distance, system.normal_speed.get()));

                    // Воспроизводим файл с оповещением...
                    // 0 знаке парковка
                    if(sign_handle.sign == parking_s) system.play_audio("../audio/parking.wav");
                    // О знаке движение вперёд
                    if(sign_handle.sign == top_s) system.play_audio("../audio/top.wav");
                    // О знаке движение влево
                    if(sign_handle.sign == left_s) system.play_audio("../audio/left.wav");
                    // О знаке движение вправо
                    if(sign_handle.sign == right_s) system.play_audio("../audio/right.wav");
                    break;
                }
            }

		    // Если запрещающий сигнал или нет вообще сигнала и при этом таймер запущен
            if (tr_timer_started) {
                // Останавливаем таймер
			    tr_timer.stop(); 
                // Если прошло 18 секунд (это время с момента включения красного до зажигания зеленого + небольшой запас)
			    if (tr_timer.millis() >= 18000) { 
				    // ВАЖНО! бывает так, что робот теряет светофор, проехав по инерции. Тогда по истечении 18 сек он снова поедет в любом случае
                    // Запускаем движение робота
				    engine.speed = system.normal_speed.get();   
                    hold = true;
                    holding_step.push_back(
                        Hold(false, 50, system.normal_speed.get()));
				    // Выставлеяем состояние таймера как не запущен
                    tr_timer_started = false;   
			    }
		    }
        }
        // Обновляем параметры движения модели
        system.engine.write(engine);

        // Задержка для работы регулятора
        usleep(10000);
    }
    INFO("LoopFnc thread was stoping.");
    return NULL;
}
