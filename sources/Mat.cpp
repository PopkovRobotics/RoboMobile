#include "Mat.hpp"
#include <iostream>

// Функции класса Mat
// Конструкторы класса
Mat::Mat() {
    matrix_uc8 = nullptr;
    matrix_dims = 0;
    matrix_set = false;
}
/*
 * size - параметр типа Point, размер изображения,
 * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat.
 */
Mat::Mat(Point size, TypeMat type) {
    matrix_uc8 = nullptr;
    matrix_set = false;
    init(size, type);
    matrix_uc8 = new uint8_t[this->matrix_size.x * this->matrix_size.y * this->matrix_dims];
}
/*
 * size - параметр типа Point, размер изображения,
 * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat,
 * data - динамический массив типа uint8_t, данные для заполнения матрицы.
 */
Mat::Mat(Point size, TypeMat type, uint8_t* data) {
    matrix_uc8 = nullptr;
    matrix_set = false;
    init(size, type);
    matrix_uc8 = new uint8_t[this->matrix_size.x * this->matrix_size.y * this->matrix_dims];
    set(data);
}
/*
 * Конструктор копирования.
 * input - константная ссылка на параметр типа Mat, присваиваемый класс.
 */
Mat::Mat(const Mat &input) {
    matrix_uc8 = nullptr;
    matrix_dims = 0;
    matrix_set = false;
    copy(input);
}
// Дескриптор класса
Mat::~Mat() {
    clear();
}
/*
 * Функция создаёт новую матрицу.
 * size - параметр типа Point, размер изображения,
 * type - параметр типа TypeMat, типа матрицы. Смотрите перечесление TypeMat.
 */
void Mat::create(Point size, TypeMat type) {
    clear();
    init(size, type);
    matrix_set = false;
    matrix_uc8 = new uint8_t[this->matrix_size.x * this->matrix_size.y * this->matrix_dims];
}
/*
 * Функция для инициализации новой матрицы.
 * size - параметр типа size, размер изображения для хранения,
 * type - параметр типа TypeMat, тип матрицы. Смотрите перечесление TypeMat.
 */
void Mat::init(Point size, TypeMat type) {
    this->matrix_size = size;
    this->matrix_type = type;
    switch(this->matrix_type) {
        // Для матрицы с 3 каналами
        case UC8_3:
        {
            matrix_dims = 3;
            break;                    
        };
        // Для матрицы с 1 каналом
        case UC8_1:
        {
            matrix_dims = 1;
            break;
        };
    }
}
/*
 * Функция для копирования первого класса во второй.
 * input - константная ссылка на параметр типа Mat, присваиваемый класс.
 */
void Mat::copy(const Mat &input) {
    // Выполняем копирование значений
    matrix_set = input.matrix_set;
    matrix_type = input.matrix_type;
    matrix_dims = input.matrix_dims;
    matrix_size = input.matrix_size;
    if(matrix_uc8 != nullptr) 
        delete[] matrix_uc8;
    matrix_uc8 = nullptr;

    matrix_uc8 = new uint8_t[matrix_size.x * matrix_size.y * matrix_dims];

    if(matrix_set) {
        std::memcpy(matrix_uc8, input.matrix_uc8, (matrix_size.x * matrix_size.y * matrix_dims));
    }
}
// Функция освобождает ресурсы, которые занимает данные матрицы
void Mat::clear() {
    if(matrix_uc8 != nullptr) {
        delete[] matrix_uc8;
        matrix_uc8 = nullptr;
    }
    matrix_set = false;
}
/*
 * Функция заполняет матрицу новыми данными.
 * new_data - динамический массив типа uint8_t, новые данные для заполнения матрицы.
 */
bool Mat::set(uint8_t* new_data) {
    if(empty() || matrix_uc8 == nullptr) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return false;
    }
    std::memcpy(matrix_uc8, new_data, (matrix_size.x * matrix_size.y * matrix_dims));
    matrix_set = true;
    return true;
}
bool Mat::setPixel(Point point, uint8_t value) {
    if(empty() || matrix_uc8 == nullptr || !matrix_set) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return false;
    }
    uint32_t start = (point.y * (matrix_size.x * 3) + (point.x * 3));
    matrix_uc8[start] = value;
    matrix_uc8[start + 1] = value;
    matrix_uc8[start + 2] = value;
    return true;
}
// Возвращает размер матрицы
Point Mat::size() const {
    return matrix_size;
}
// Возвращает количество каналов матрицы
uint8_t Mat::dims() const {
    return matrix_dims;
}
// Возвращает тип матрицы
TypeMat Mat::type() const {
    return matrix_type;
}
// Возвращает матрицу в виде массива
uint8_t* Mat::data() const {
    return matrix_uc8;
}
/*
 * Проверяет является ли матрица пустой.
 * Если возвращает true, то матрица пустая, в противном случае матрица не пустая.
 */
bool Mat::empty() {
    if((matrix_size.x <= 0 && matrix_size.y <= 0) || matrix_dims <= 0)
        return true;
    return false;
}
/*
 * Функция возвращает цвет пикселя в формате std::vector<Tp> по координатам point.x и point.y.
 * point - параметр типа Point, координаты пикселя.
 */
template<typename Tp>
std::vector<Tp> Mat::at(Point point) {
	std::vector<Tp> pixel;
    if(empty() || matrix_uc8 == nullptr || !matrix_set) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return pixel;
    }
    pixel = std::vector<Tp>(matrix_dims);
    uint32_t start = (point.y * (matrix_size.x * 3) + (point.x * 3));
    switch(matrix_type) {
        case UC8_1:
        case UC8_3:
        {

            std::memcpy(&pixel[0], &matrix_uc8[start], matrix_dims);
            break;
        };
    }
    return pixel;
}
/*
 * Функция возвращает компонент пикселя в формате Tp по координатам point.x и point.y.
 * Tp - шаблон для функции, тип возвращаемых данных,
 * j - номер компоненты в матрице.
 */
template<typename Tp>
Tp Mat::at(int j) {
    Tp comp;
    if(empty() || matrix_uc8 == nullptr || !matrix_set) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return comp;
    }
    if(j > matrix_size.x * matrix_size.y * matrix_dims) {
        ERROR("Index j out of range matrix.");
        return comp;
    }
    switch(matrix_type) {
        case UC8_1:
        case UC8_3:
        {
            
            comp = matrix_uc8[j];
            break;
        };
    }
    return comp;
}
/*
 * Функция определяет цвет пикселя и возвращает его по координатам point.x и point.y.
 * point - параметр типа Point, координаты пикселя для определения цвета,
 * type_color - параметр типа ColorType, порядок компонент в матрице. Смотрите перечисление ColorType.
 */
Color Mat::color(Point point, ColorType type_color) {
    Color color = None;
    if(empty() || matrix_uc8 == nullptr || !matrix_set) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return color;
    }
    switch(matrix_type) {
        case UC8_3:
        {
            std::vector<uint8_t> pixel = at<uint8_t>(point);
            uint8_t red,                   // Красная компонента
                    green = pixel[1],      // Зелёная компонента
		            blue;                  // Синия компонента
            switch(type_color) {
                case RGB:
                {
                    red = pixel[0];
                    blue = pixel[2];
                    break;
                }
                case BGR:
                {   red = pixel[2];
                    blue = pixel[0];
                    break;
                }
            }

            float i = (float)red / (float)green,
		        j = (float)green / (float)blue;
	        float value = i / j;

            if(green > 80 && (float)green > ((float)(red + blue) * (float)1.1)) color = Green;
            else if (red > 60 && red > ((float)(green + blue) * (float)0.7) && (red - green) > 30) color = Red;
            else if(red > 90 && abs(red - green) < 30 && blue <= 70) color = Yellow;
	        else if ((blue - (red + green)) > 10) color = Blue;
	        else if (blue <= 60 && green <= 60 && red <= 60) color = Black;
            else if (red > 60 && (value > (float)0.7 && value < (float)1.3)) color = White;
            
            break;
        };
        default:
        {
            std::string warning = "At the moment there is no implementation of color recognition on the matrix type " + typeString();
            WARNING(warning.c_str());
            break;
        };
    }
    return color;
}
// Преобразовывает тип матрицы в её название
std::string Mat::typeString() {
    if(empty() || matrix_uc8 == nullptr) {
        return "NONE";
    }
    std::string name;
    switch(matrix_type) {
        case UC8_3:
        {
            name = "UC8_3";
            break;
        }
        case UC8_1:
        {
            name = "UC8_1";
            break;
        }
    }
    return name;
}
// Перегрузка операторов
/*
 * Перегрузка опреатора реализует возвращение области на изображение
 * по координатам области.
 * rect - параметр типа Rect, координаты области на изображение, которую
 * извлекаем.
 */
Mat Mat::operator()(Rect rect) {
    Mat roi(Point(rect.width, rect.height), matrix_type);
    if(empty() || matrix_uc8 == nullptr || !matrix_set) {
        ERROR("This matrix are empty. Call function create() to create matrix.");
        return roi;
    }
    if(rect.empty() || rect.x > (matrix_size.x - 1) || 
        rect.y > (matrix_size.y - 1)) {
        ERROR("Area coordinates are empty or out of matrix.");
        return roi;
    }

    if(matrix_type == UC8_3 || matrix_type == UC8_1) {
        std::vector<uint8_t> data;
        for(int32_t y = rect.y; y < (rect.height + rect.y); y++) {
            for(int32_t x = rect.x; x < (rect.width + rect.x); x++) {
                std::vector<uint8_t> dims = at<uint8_t>(Point(x, y));
                for(uint8_t i = 0; i < dims.size(); i++)
                    data.push_back(dims[i]);
            }
        }
        roi.set(data.data());
    }
    return roi;
}
/*
 * Перегрузка оператора реализует присваевания класса.
 * input - константная ссылка на параметр типа Mat, присваиваемый объект.
 */
Mat& Mat::operator=(const Mat& input) {
    // Проверка на самоприсваивание
    if (this == &input)
        return *this;
 
    // Выполняем копирование значений
    copy(input);

    // Возвращаем текущий объект
    return *this;
}

bool Mat::operator==(const Mat& input) {
    if(input.size() == matrix_size && input.dims() == matrix_dims) {
        uint8_t* data = input.data();
        for(int32_t i = 0; i < (matrix_dims * matrix_size.x * matrix_size.y); i++) {
            if(matrix_uc8[i] != data[i]) 
                return false;
        }
        return true;
    }
    return false;
}
