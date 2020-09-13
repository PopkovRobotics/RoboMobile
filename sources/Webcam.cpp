#include "Webcam.hpp"

// Очищает структуру
#define CLEAR(x) memset(&(x), 0, sizeof(x))

/*
 * Вспомогательная функция для установки параметров вебкамеры.
 * fh - параметр типа int, дескриптор объекта для которого устаналивается параметры,
 * request - параметр типа unsigned long int, тип устанавливаемых параметров,
 * arg - универсальный указатель, указатель на параметры.
 */
static int xioctl(int fh, unsigned long int request, void *arg) {
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

// Устаналивает значение пикселя в пределах 0 и 255
#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

/*
 * Конвертирует изображение из YUYV формата в BGR формат.
 * src - массив типа char, входное YUYV изображение,
 * dest - массив типа char, выходное BGR изображение,
 * width - параметр типа int, ширина кадра,
 * height - параметр типа int, высота кадра,
 * stride - параметр типа int, количество байтов в одной строке BGR изображения.
 */
static void yuyv_to_bgr(const unsigned char* src,
                                     unsigned char* dest,
                                     int width, int height, 
                                     int stride) {
    int j;

    while (--height >= 0) {
        for (j = 0; j + 1 < width; j += 2) {
            // Получаем YUYV компоненты
            int u = src[1];
            int v = src[3];
            int u1 = (((u - 128) << 7) +  (u - 128)) >> 6;
            int rg = (((u - 128) << 1) +  (u - 128) +
                    ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
            int v1 = (((v - 128) << 1) +  (v - 128)) >> 1;

            // Записываем пиксели в BGR мизображение
            *dest++ = CLIP(src[0] + u1);            // Синия компонента
            *dest++ = CLIP(src[0] - rg);            // Зелёная компонента
            *dest++ = CLIP(src[0] + v1);            // Красная компонента

            *dest++ = CLIP(src[2] + u1);            // Синия компонента
            *dest++ = CLIP(src[2] - rg);            // Зелёная компонента
            *dest++ = CLIP(src[2] + v1);            // Красная компонента
            src += 4;
        }
        src += stride - (width * 2);
    }
}

/*
 * Конструктор класса.
 * device - константный параметр  типа std::string, порт, к которому подключена вебкамера,
 * frame_size - параметр типа Point, размер кадров, считанных с вебкамеры.
 */
Webcam::Webcam(const std::string& device, Point frame_size) :
                        device(device),
                        xres(frame_size.x),
                        yres(frame_size.y) {
    // Открываем вебкамеру
    open_device();
    // Устаналивает параметры считывания кадров с вебкамеры
    init_device();

    // Массив для хранения кадра с вебкамеры
    data = (unsigned char*)malloc((xres * yres * 3) * sizeof(char));

    // Запускаем вебкамеру для считывания кадров с вебкамеры
    start_capturing();
}

// Дескриптор класса
Webcam::~Webcam() {
      // Функция завершает работу с вебкамерой
      stop_capturing();
      //Освобождаем память занятую буферами
      uninit_device();
      // Закрываем вебкамеру
      close_device();

      // Освобождаем память
      free(data);
}

/*
 * Функция считывает кадр с вебкамеры и возвращает его. В случае неудачи повторяет действие,
 * пока не пройдёт время timeout.
 * timeout - параметр типа int, время в течении, которого функция пытается считать кадр с вебкамеры.
 */
unsigned char* Webcam::frame(int timeout) {

    // Выполянем код в цикле, пока не считаем кадр
    for (;;) {
        fd_set fds;
        struct timeval tv;
        int r;

        // Обнуляем структуры
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // Выбераем вебкамеру, как устройство для считывания кадра
        r = select(fd + 1, &fds, NULL, NULL, &tv);
        // Если не удалось выбрать вебкамеру
        if (-1 == r) {
            if (EINTR == errno) return data;
            throw std::runtime_error("select");
        }

        // Если истекло время ожидания считывания кадра
        if (0 == r) {
	    throw std::runtime_error(device + ": select timeout");
	}

        // Считываем кадр с вебкамеры
        if (read_frame())
            return data;
    }

}

// Функция считывает кадр с вебкамеры
bool Webcam::read_frame() {
    struct v4l2_buffer buf;
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    // Передаёт буфер драйвер вебкамеры
    if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
        switch (errno) {
            case EAGAIN:
                return false;

            case EIO:
                // Игнорируем ошибку о попытке записи
                // на отключенное консольное устройство

            default:
                throw std::runtime_error("VIDIOC_DQBUF");
        }
    }

    // Проверяем, то что индекс буфера не превышает их количество
    assert(buf.index < n_buffers);

    // Конвертируем YUYV изображение в BGR изображение
    yuyv_to_bgr((unsigned char*)buffers[buf.index].data,    // Массив с YUYV изображением
                data,                                       // Массив, в который записывается BGR изображение
                xres,                                       // Ширина кадра
                yres,                                       // Высота кадра
                stride);                                    // Количество байтов на одну строку кадра

    // Завершаем чтение кадра с вебкамеры
    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
        throw std::runtime_error("VIDIOC_QBUF");

    return true;
}

// Функция открывает вебкамеру для дальнейшей работы
void Webcam::open_device() {
      struct stat st;
      // Проверяем на наличие устройства.
      // Если устройство нету, то возвращаем исключение
      if (-1 == stat(device.c_str(), &st)) {
            throw std::runtime_error(device + ": cannot identify! " + std::to_string(errno) +  ": " + strerror(errno));
      }
      if (!S_ISCHR(st.st_mode)) {
            throw std::runtime_error(device + " is no device");
      }

      // Открываем вебкамеру
      fd = open(device.c_str(), O_RDWR | O_NONBLOCK, 0);
      // Если возникла ошибка, то возвращаем исключение
      if (-1 == fd) {
            throw std::runtime_error(device + ": cannot open! " + std::to_string(errno) + ": " + strerror(errno));
      }
}

// Функция инициализирует массив для хранения кадра
void Webcam::init_mmap() {
      struct v4l2_requestbuffers req;
      CLEAR(req);

      // Число запрошенных буферов
      req.count = 3;
      // Тип буффера для хранения кадра
      req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      // Для считывания кадра из буфера
      req.memory = V4L2_MEMORY_MMAP;

      // Проверяем поддерживает ли устройство метод ввода-вывода кадра
      if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                  throw std::runtime_error(device + " does not support memory mapping");
            }else {
                  throw std::runtime_error("VIDIOC_REQBUFS");
            }
      }
      // Проверяем достаточно ли буферной памяти для хранения кадра
      if (req.count < 2) {
            throw std::runtime_error(std::string("Insufficient buffer memory on ") + device);
      }

      // Выделяем память для хранения кадра
      buffers = (buffer*)calloc(req.count, sizeof(*buffers));
      // Проверяем выделилась ли память для буфера
      if (!buffers) {
            throw std::runtime_error("Out of memory");
      }

      // Создаём буферы для хранения кадра
      for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
            struct v4l2_buffer buf;
            CLEAR(buf);

            // Тип буффера для хранения кадра
            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            // Для считывания кадра из буфера
            buf.memory      = V4L2_MEMORY_MMAP;
            // Индекс буфера
            buf.index       = n_buffers;

            // Устаналиваем информацию о буфере
            if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                  throw std::runtime_error("VIDIOC_QUERYBUF");

            // Записываем информацию о буфере в структуру
            buffers[n_buffers].size = buf.length;
            // Инициализируем буфер
            buffers[n_buffers].data =
                  mmap(NULL,
                        buf.length,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd, buf.m.offset);

            // Если не удалось создать буфер
            if (MAP_FAILED == buffers[n_buffers].data)
                  throw std::runtime_error("mmap");
      }
}

// Функция закрывает вебкамеру
void Webcam::close_device() {
    // Закрываем вебкамеру
    if (-1 == close(fd))
        throw std::runtime_error("close");

    fd = -1;
}

// Функция устаналивает параметры считывания кадров с вебкамеры
void Webcam::init_device() {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;

    // Проверяем поддерживает ли V4L2 устройство
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            throw std::runtime_error(device + " is no V4L2 device");
        } else {
            throw std::runtime_error("VIDIOC_QUERYCAP");
        }
    }
    // Проверяем увляется ли устройство вебкамерой
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        throw std::runtime_error(device + " is no video capture device");
    }
    // Проверяем поддерживает ли устройство считывание видео
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        throw std::runtime_error(device + " does not support streaming i/o");
    }

    // Устаналивает параметры чтения кадров с вебкамеры

    // Устаналиваем параметры для обрезки кадра
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        xioctl(fd, VIDIOC_S_CROP, &crop);
    }

    // Устаналивает параметры кадра (разрешение, тип изображения и т.п.)
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // Ширина кадра
    fmt.fmt.pix.width       = xres;
    // Высота кадра
    fmt.fmt.pix.height      = yres;
    // Формат кадра
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; 
    fmt.fmt.pix.field       = V4L2_FIELD_NONE;
    // Устаналиваем параметры кадра
    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        throw std::runtime_error("VIDIOC_S_FMT");

    // Проверяем поддерживает ли камера заданный формат изображения
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
        throw std::runtime_error("Webcam does not support YUYV format. Support for more format need to be added!");

    // VIDIOC_S_FMT может изменить разрешение кадра, если установленное
    // не поддерживается
    xres = fmt.fmt.pix.width;
    yres = fmt.fmt.pix.height;

    // Получаем количество байтов на одну строку кадра
    stride = fmt.fmt.pix.bytesperline;

    struct v4l2_streamparm fps;
    CLEAR(fps);
    fps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(-1 == xioctl(fd, VIDIOC_G_PARM, &fps)) throw std::runtime_error("VIDIOC_G_PARM");
    
    fps.parm.capture.capturemode |= V4L2_CAP_TIMEPERFRAME;
    fps.parm.capture.timeperframe.numerator = 1;
    fps.parm.capture.timeperframe.denominator = 30;
    if(-1 == xioctl(fd, VIDIOC_S_PARM, &fps)) throw std::runtime_error("VIDIOC_S_PARM");

    // Инициализирует массив для хранения кадра
    init_mmap();
}

// Функция освобождает память занятую буферами
void Webcam::uninit_device() {
    unsigned int i;

    // Освобождаем память
    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap(buffers[i].data, buffers[i].size))
            throw std::runtime_error("munmap");

    free(buffers);
}

// Функция запускает вебкамеру для считывания кадров с вебкамеры
void Webcam::start_capturing() {
    unsigned int i;
    enum v4l2_buf_type type;

    // Устаналиваем, в какие буферы сохраняется считанный кадр
    for (i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            throw std::runtime_error("VIDIOC_QBUF");
    }

    // Запускаем вебкамеру
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        throw std::runtime_error("VIDIOC_STREAMON");
}

// Функция завершает работу с вебкамерой
void Webcam::stop_capturing() {
    enum v4l2_buf_type type;

    // Завершаем работу с вебкамерой
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
        throw std::runtime_error("VIDIOC_STREAMOFF");
}

int Webcam::is_readable(timeval* tv) {
	fd_set fdset;
	FD_ZERO(&fdset);	
	FD_SET(fd, &fdset);
	return select(fd, &fdset, NULL, NULL, tv);
}


