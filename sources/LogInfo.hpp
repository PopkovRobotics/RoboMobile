#pragma once
// Библиотеки C++
#include <stdio.h>
// Функции для отправки сообщений в терминал
// Функция вывода информационного сообщения
#define INFO(msg) printf("[Info]: %s\n", msg)

// Функция вывода сообщения с предупреждением
#define WARNING(msg) printf("[Warning]: %s\n", msg)

// Функция вывода сообщения с ошибкой
#define ERROR(msg) printf("[Error]: %s\n", msg)