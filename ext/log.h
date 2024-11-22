//
// Created by laoguan on 2/13/2023.
//

#ifndef ANN_MATRIX_LOG_H
#define ANN_MATRIX_LOG_H


#include <stdarg.h>

#define _LOG(fmt, ...)  _vlog(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
typedef unsigned long long size_t;

void getTimestamp(char* timestamp, size_t size);
void _log_init(const char* file);
void _log(const char* fmt, ...);
void _vlog(const char* filename, const int line, const char* fmt, ...);



#endif //ANN_MATRIX_LOG_H
