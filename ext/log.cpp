//
// Created by laoguan on 2/13/2023.
//

#include "log.h"

#include <stdio.h>
#include <time.h>
#include <string.h>


static FILE* _log_fp = NULL;


void _log_init(const char* file) {
    // if (_log_fp != NULL) {
    //     fclose(_log_fp);
    //     _log_fp = NULL;
    // }
    // if(file == NULL)  // 关闭文件输出
    //     return;
    // int r = fopen_s(&_log_fp, file, "a");
    // if (r != 0) {
    //     printf("open log file failed, errno: %d\n", r);
    //     _log_fp = NULL;
    // }
}

void getTimestamp(char* timestamp, size_t size)
{
    //  time_t sec = time->tv_sec; /* a necessary variable to avoid a runtime error on Windows */
    struct tm calendar;
    time_t sec = time(NULL);
    localtime_s(&calendar, &sec);
    strftime(timestamp, size, "%m-%dT%H:%M:%S ", &calendar);
    //  sprintf(&timestamp[17], ".%06ld", (long) time->tv_usec);
    //  timestamp[16] = ' ';
    //  timestamp[17] = 0;
}
void _log(const char* fmt, ...) {
    va_list carg;
    char tm[24];
    FILE* fp = _log_fp == NULL ? stderr : _log_fp;
    getTimestamp(tm, sizeof(tm) - 1);
    fprintf(fp, tm);
            va_start(carg, fmt);
    vfprintf(fp, fmt, carg);
            va_end(carg);
    fputs("\n", fp);
    fflush(fp);
}

void _vlog(const char* filename, const int line, const char* fmt, ...) {
    va_list carg;
    char tm[24], * p;
    FILE* fp = _log_fp == NULL ? stderr : _log_fp;
    p = strrchr((char*)filename, '\\');
    if (p == NULL)
        p = strrchr((char*)filename, '/');
    p += 1;
    getTimestamp(tm, sizeof(tm) - 1);
    fprintf(fp, "%s%s:%d ", tm, p, line);
            va_start(carg, fmt);
    vfprintf(fp, fmt, carg);
            va_end(carg);
    fputs("\n", fp);
    fflush(fp);
}
