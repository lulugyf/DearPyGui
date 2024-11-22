#include <stdio.h>
#include <vector>
#include <string>

static char *nu_byte(const char *ptr, char n){
    static char _buf[6];
    char c = *ptr;
    sprintf(_buf, "%d", c);
    return _buf;
}
static char *nu_ubyte(const char *ptr, char n){
    static char _buf[6];
    unsigned char c = *ptr;
    sprintf(_buf, "%u", c);
    return _buf;
}
static char *nu_short(const char *ptr, char n){
    static char _buf[7];
    sprintf(_buf, "%d", *((short *)ptr));
    return _buf;
}
static char *nu_ushort(const char *ptr, char n){
    static char _buf[7];
    sprintf(_buf, "%u", *((unsigned short *)ptr) );
    return _buf;
}
static char *nu_int(const char *ptr, char n){
    static char _buf[12];
    sprintf(_buf, "%d", *((int *)ptr));
    return _buf;
}
static char *nu_uint(const char *ptr, char n){
    static char _buf[12];
    sprintf(_buf, "%u",  *((unsigned int *)ptr) );
    return _buf;
}
static char *nu_float(const char *ptr, char n){
    static char _buf[18];
    if(n > 0){
        if(n > 9) n = 9;
        char fmt[5] = {'%', '.', '0'+n, 'f', 0};
        sprintf(_buf, fmt, *((float *)ptr) );
    }else
        sprintf(_buf, "%f",  *((float *)ptr) );
    return _buf;
}
static char *nu_double(const char *ptr, char n){
    static char _buf[26];
    if(n > 0){
        if(n > 9) n = 9;
        char fmt[5] = {'%', '.', '0'+n, 'f', 0};
        sprintf(_buf, fmt, *((double *)ptr) );
    }else
        sprintf(_buf, "%f",  *((double *)ptr) );
    return _buf;
}

typedef struct { char c; short x; } st_short;
typedef struct { char c; int x; } st_int;
typedef struct { char c; long x; } st_long;
typedef struct { char c; float x; } st_float;
typedef struct { char c; double x; } st_double;
typedef struct { char c; bool x; } st_bool;
typedef struct { char c; long long x; } s_long_long;

#define SHORT_ALIGN (sizeof(st_short) - sizeof(short))
#define INT_ALIGN (sizeof(st_int) - sizeof(int))
#define LONG_ALIGN (sizeof(st_long) - sizeof(long))
#define FLOAT_ALIGN (sizeof(st_float) - sizeof(float))
#define DOUBLE_ALIGN (sizeof(st_double) - sizeof(double))
#define BOOL_ALIGN (sizeof(st_bool) - sizeof(_Bool))
#define LONG_LONG_ALIGN (sizeof(s_long_long) - sizeof(long long))

typedef struct _formatdef {
    char format;
    int size;
    int alignment;
    char* (*unpack)(const char *, char);
    short offset;
    char dot_n;
} formatdef;


static const formatdef native_table[] = {
    {'b',       sizeof(char),   0,              nu_byte},
    {'B',       sizeof(char),   0,              nu_ubyte},
    {'h',       sizeof(short),  SHORT_ALIGN,    nu_short},
    {'H',       sizeof(short),  SHORT_ALIGN,    nu_ushort},
    {'i',       sizeof(int),    INT_ALIGN,      nu_int},
    {'I',       sizeof(int),    INT_ALIGN,      nu_uint},
    // {'q',       sizeof(long long), LONG_LONG_ALIGN, nu_longlong},
    // {'Q',       sizeof(long long), LONG_LONG_ALIGN, nu_ulonglong},
    {'f',       sizeof(float),  FLOAT_ALIGN,    nu_float},
    {'d',       sizeof(double), DOUBLE_ALIGN,   nu_double},
    {0}
};

int stru_to_int(formatdef *f, const char *data_ptr) {
    int ret = -1;
    switch(f->format){
        case 'b':
        case 'B':
            ret = data_ptr[0]; break;
        case 'h':
            ret = *((short *)data_ptr); break;
        case 'H':
            ret = *((unsigned short *)data_ptr); break;
        case 'i':
            ret = *((int *)data_ptr); break;
        case 'I':
            ret = *((unsigned int *)data_ptr); break;
    }
    return ret;
}

inline const formatdef * getentry(int c, const formatdef *f)
{
    for (; f->format != '\0'; f++) {
        if (f->format == c) {
            return f;
        }
    }
    return NULL;
}

inline int align(int size, char c, const formatdef *e)
{
    int extra;

    if (e->format == c) {
        if (e->alignment && size > 0) {
            extra = (e->alignment - 1) - (size - 1) % (e->alignment);
            if (extra > 65536 - size)
                return -1;
            size += extra;
        }
    }
    return size;
}

int unpackf(const char *fmt, std::vector<formatdef> &ff, std::vector<std::string> &fn)
{
    const formatdef *f;
    const formatdef *e;
    // formatdef ee, LL={'Q',       sizeof(long long), LONG_LONG_ALIGN, nu_uint};
    formatdef ee, LL={'I',       sizeof(long), LONG_ALIGN, nu_uint};

    char *s, *s1;
    char c, num, fdot;
    short offset, itemsize;

    ff.clear();
    fn.clear();

    f = native_table;
    offset = 0;
    s1 = (char *)fmt;
    while(1){
        s = s1;
        num = 0;
        while( *s != '\0' && *s != ' ' ){
            s++;
            num ++;
        }
        bool end = *s == '\0';
        c = *s1; s1 ++; num--; // 第 0 or 1 字符作为格式， 第一为数字时是浮点数显示几位小数
        fdot = 3; // 默认小数点1
        if(c >= '0' && c <= '9'){
            fdot = c - '0';
            c = *s1; s1 ++; num --;
        }
        e = getentry(c, f);
        if (e == NULL)
            return -7;
        ee = *e;
        ee.dot_n = fdot; // 设置为0则不显示

        offset = align(offset, ee.format, &ee);
        if(offset == -1){
            printf("overflow\n");
            return -2;
        }
        ee.offset = offset;
        offset += ee.size;
        ff.push_back(ee);  // 保存格式描述结构
            
        fn.push_back(std::string(s1, 0, num)); // 保存名称
        if(end) break;
        s1 = s + 1;
    }
    if(ff.size() != fn.size())
        return -4;

    return align(offset, LL.format, &LL); // 返回结构体的大小, python的 struct.calcsize 的计算不准确， 这个做了部分情况的对照， 与c++的结果一致
}

#ifdef __TEST__

/// @brief //////////////
struct _test {
    int a;
    uint8_t b;
    int8_t c;
    uint16_t d;
    float e;
    double f;
    uint32_t g;
    float h;
    int8_t i;
};

#define OFF(t, a) printf("  % 3d %s %d\n", (char *)&t.a - (char *)&t, #a, sizeof(t.a));

int main()
{
    printf("hello world\n");

    _test t = {-1, 2, -3, 4, -5.1, 6.0004, 7, 8.002, -9};
    printf("struct size: %d   f: %d\n", sizeof(_test), sizeof(st_float));
    // printf("  % 3d %s\n", (char *)&t.a - (char *)&t, "a");
    // OFF(t, a);
    // OFF(t, b);
    // OFF(t, c);
    // OFF(t, d);
    // OFF(t, e);
    // OFF(t, f);
    // OFF(t, g);
    // OFF(t, h);
    // OFF(t, i);

    // unpackf("iBbhfdIfb", "a b c d e f g h i", (char *)&t);
    auto ret = unpackf("iBbhf4dI3fb", "a b c d e f g h i", (char *)&t);
     printf("=====ret: %d\n", ret);

    ret = unpackf("ia Bb bc hd fe 4df Ig 3fh bi", (char *)&t);
    printf("=====ret: %d\n", ret);


    return 0;
}

#endif
