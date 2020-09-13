#pragma once
// Библиотеки C++
#include <string>
#include <vector>
#include <cstring>
#include <stdint.h>
#include <algorithm>
// Файлы программы
#include "LogInfo.hpp"

// Структура для хранения координат прямоугольника
struct Rect {
    int32_t width,      // width - ширина прямоугольника в пикселях
            height,     // height - высота прямоугольника в пикселях 
            x,          // x - координата верхнего левого угла прямоугольника
            y;          // y - координата верхнего левого угла прямоугольника

    // Констуктор структуры устаналивает начальные значения для параметров
    Rect(uint32_t x = 0, uint32_t y = 0, 
        uint32_t width = 0, uint32_t height = 0) {
        this->x = x; 
        this->y = y;
        this->width = width; 
        this->height = height;
    }
    // Проверяет, что ширина и высота прямоугольника не нулевые
    bool empty() {
        if(width < 0 || height < 0)
            return true;
        return false;
    }
};

/*
 * Структура для хранения двух целых чисел.
 * Структуру используется для хранения размера (ширина, высота) и 
 * координат (x и y).
 */
struct Point {
    int32_t x,          // x - первый целочисленный параметр
            y;          // y - второй целочисленный параметр

    // Конструктор класса
    Point(int32_t x = 0, int32_t y = 0) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const Point& input) {
        if(x == input.x && y == input.y) 
            return true;
        return false;
    }
};

// Перечисление, которое содержит цвета.
enum Color {
    None = 0,       // Неизвестный цвет
    Red,            // Красный цвет
    Green,          // Зелёный цвет
    Yellow,         // Жёлтый цвет
    Blue,           // Синий цвет
    White,          // Белый цвет
    Black,          // Чёрный цвет

};

// Тип матрицы в классе Mat
enum TypeMat {
    UC8_3 = 0,          // В матрице каждый пиксель имеет 3 компонента. 
                        // Для 1 компоненты выделяется 1 БАЙТ (минимально значение компоненты 0, а максимальное 255).
    UC8_1,              // В матрице каждый пиксель имеет 1 компоненту. 
                        // Для 1 компоненты выделяется 1 БАЙТ (минимально значение компоненты 0, а максимальное 255).
};

// Перечесление, которое определяет порядок компонент в матрице.
enum ColorType {
    RGB = 0,        // Красная компонента, зелёная компонента, синия компонента
    BGR,            // Синия компонента, зелёная компонента, красная компонента
};

// Класс реализует матрицу для хранения изображений
class Mat {
private:
    uint8_t* matrix_uc8;                // matrix_uc8 - массив для хранения матрицы типа UC8_3 и UC8_1
    TypeMat matrix_type;                // type - тип матрицы, смотрите перечесление TypeMat
    uint8_t matrix_dims;                // dims - количество каналов матрицы
                                        // Например для RGB изображения, это значение 3,
                                        // а для чёрно-белого изображения, это значение 1.
    Point matrix_size;                  // matrix_size - размер матрицы, то есть ширина и высота
    bool matrix_set;                    // matrix_set - флаг определяющий есть ли данные в матрице

    /*
     * Функция для инициализации новой матрицы.
     * size - параметр типа size, размер изображения для хранения,
     * type - параметр типа TypeMat, тип матрицы. Смотрите перечесление TypeMat.
     */
    void init(Point size, TypeMat type);
    /*
     * Функция для копирования первого класса во второй.
     * input - константная ссылка на параметр типа Mat, присваиваемый класс.
     */
    void copy(const Mat &input);
public:
    // Конструкторы класса
	Mat();
    /*
     * size - параметр типа Point, размер изображения,
     * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat.
     */
    Mat(Point size, TypeMat type);
    /*
     * size - параметр типа Point, размер изображения,
     * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat,
     * data - динамический массив типа uint8_t, данные для заполнения матрицы.
     */
    Mat(Point size, TypeMat type, uint8_t* data);
    /*
     * Конструктор копирования.
     * input - константная ссылка на параметр типа Mat, присваиваемый класс.
     */
    Mat(const Mat &input);
    // Дескриптор класса
    ~Mat();
    /*
     * Функция создаёт новую матрицу.
     * size - параметр типа Point, размер изображения,
     * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat.
     */
    void create(Point size, TypeMat type);
    /*
     * Функция заполняет матрицу новыми данными.
     * new_data - динамический массив типа uint8_t, новые данные для заполнения матрицы.
     */
    bool set(uint8_t* new_data);
    bool setPixel(Point point, uint8_t value);
    // Возвращает размер матрицы
    Point size() const;
    // Возвращает количество каналов матрицы
    uint8_t dims() const;
    // Возвращает тип матрицы
    TypeMat type() const;
    // Возвращает матрицу в виде массива
    uint8_t* data() const;
    /*
     * Проверяет является ли матрица пустой.
     * Если возвращает true, то матрица пустая, в противном случае матрица не пустая.
     */
    bool empty();
    /*
     * Функция возвращает цвет пикселя в формате std::vector<Tp> по координатам point.x и point.y.
     * Tp - шаблон для функции, тип возвращаемых данных,
     * point - параметр типа Point, координаты пикселя.
     */
    template<typename Tp>
    std::vector<Tp> at(Point point);
    /*
     * Функция возвращает компонент пикселя в формате Tp по координатам point.x и point.y.
     * Tp - шаблон для функции, тип возвращаемых данных,
     * j - номер компоненты в матрице.
     */
    template<typename Tp>
    Tp at(int j);
    /*
     * Функция определяет цвет пикселя и возвращает его по координатам point.x и point.y.
     * point - параметр типа Point, координаты пикселя для определения цвета,
     * type_color - параметр типа ColorType, порядок компонент в матрице. Смотрите перечисление ColorType.
     */
    Color color(Point point, ColorType type_color);
    // Преобразовывает тип матрицы в её название
    std::string typeString();
    // Функция освобождает ресурсы, которые занимает данные матрицы
    void clear();
    // Перегрузка операторов
    /*
     * Перегрузка опреатора реализует возвращение области на изображение
     * по координатам области.
     * rect - параметр типа Rect, координаты области на изображение, которую
     * извлекаем.
     */
    Mat operator()(Rect rect);
    /*
     * Перегрузка оператора реализует присваевания класса.
     * input - константная ссылка на параметр типа Mat, присваиваемый объект.
     */
    Mat& operator=(const Mat& input);
    bool operator==(const Mat& input);
};