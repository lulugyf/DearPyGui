#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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


int unpackf(const char *fmt, char *data){
    const formatdef *f;
    const formatdef *e;
    formatdef ee, LL={'Q',       sizeof(long long), LONG_LONG_ALIGN, nu_uint};

    char *s, *s1;
    char c, num, fdot;
    short offset, total_size;
    std::vector<formatdef> ff;
    std::vector<std::string> fn;


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
        fdot = 0;
        if(c >= '0' && c <= '9'){
            fdot = c - '0';
            c = *s1; s1 ++; num --;
        }
        e = getentry(c, f);
        if (e == NULL)
            return -7;
        ee = *e;
        ee.dot_n = fdot;

        offset = align(offset, ee.format, &ee);
        if(offset == -1){
            printf("overflow\n");
            return -2;
        }
        ee.offset = offset;
        // printf("offset of %c is %d\n", ee.format, ee.offset);
        offset += ee.size;
        ff.push_back(ee);  // 保存格式描述结构
            
        fn.push_back(std::string(s1, 0, num)); // 保存名称
        if(end){
            total_size = align(offset, LL.format, &LL);
            // if(total_size - offset == )
            // printf("total_size: %d\n", total_size);
            break;
        }
        s1 = s + 1;
    }
    if(ff.size() != fn.size())
        return -4;

    auto sz = ff.size();
    s = data;
    for(int i=0; i<sz; i++){
        f = &ff[i];
        printf(" ---% 3d  %s  val[%s](%c)\n", f->offset, fn[i].c_str(), f->unpack(data+f->offset, f->dot_n), f->format);
    }

    return total_size;
    // auto &fl =  ff[ff.size()-1];
    // return fl.size + fl.offset; // 返回结构体的大小
}




int parse_fmt(const char *fmt, std::vector<formatdef> &ff, std::vector<std::string> &fn)
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
        fdot = 0;
        if(c >= '0' && c <= '9'){
            fdot = c - '0';
            c = *s1; s1 ++; num --;
        }
        e = getentry(c, f);
        if (e == NULL)
            return -7;
        ee = *e;
        ee.dot_n = fdot;

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

    double l1;
    int l2;
    double l3;
    // uint8_t l3;

};

static void test_file(){
    std::ofstream fo;
    fo.open("d:/tmp/aa.bin", std::ios::binary|std::ios::out);
    for(int i=0; i<1024; i++)
        fo.write("12345678", 8);
    fo.close();
}

#define OFF(t, a) printf("  % 3d %s %d\n", (char *)&t.a - (char *)&t, #a, sizeof(t.a));

int test1()
{
    int ret;

    _test t = {-1, 2, -3, 4, -5.1, 6.0004, 7, 8.002, -9};
    printf("struct size: %d   f: %d\n", sizeof(_test), LONG_LONG_ALIGN);
    // printf("  % 3d %s\n", (char *)&t.a - (char *)&t, "a");
    OFF(t, a);
    OFF(t, b);
    OFF(t, c);
    OFF(t, d);
    OFF(t, e);
    OFF(t, f);
    OFF(t, g);
    OFF(t, h);
    OFF(t, i);

    _test arr[3];
    char *p0, *p1;
    p0 = (char *)&arr[0];
    p1 = (char *)&arr[2];
    printf("====== arr offset: %d\n", p1-p0);


    // ret = unpackf("ia Bb bc hd fe 4df Ig 3fh bi", (char *)&t);
    ret = unpackf("ia Bb bc hd fe df Ig fh bi d0 i0 d0", (char *)&t);
    printf("=====ret: %d  %d \n", ret, sizeof(_test));

    test_file();

    return 0;
}

using str = std::string;
struct BaseOB {
    uint32_t id;   // 目标id
    float lat;     // 横向距离,  单位m， 右正
    float lgt;     // 纵向距离， 单位m， 上（前）正
    float width;   // 横向宽度， 单位m
    float length;  // 纵向长度
    int16_t angle;   // 目标运动方向， 单位°， 正前方0， 正右90， 正左-90， 正后180
    uint8_t flag;    // 按位的标志， b0:1-CIPV;  b1: 0-静止 1-运动; b2~b4: 目标类别,  类型对应 OBTYPE; b5~b7: Movement state运动状态
    uint8_t reserve; // 保留
};
struct FIDX{
    unsigned int ms;
    unsigned int sz;
    long long fpos;
};

static void test2(){
    str dfile = "D:/record/h97d/20240304175437/misc/obj_f.stru";
    str line;

    int stru_sz = 0, bsz = sizeof(BaseOB), ret=0;
    str stru_ds;
    FIDX idx;
    long long gpos;
    BaseOB co;
    std::vector<char> oo;
    std::vector<formatdef> ff;
    std::vector<std::string> fn;


    std::ifstream fi(dfile + ".txt");
    if(fi.is_open()){
        std::getline(fi, line);
        // printf(" --line1: [%s]\n", line.c_str());
        stru_ds = line;
        std::getline(fi, line);
        stru_sz = std::atoi(line.c_str());
        // printf(" --line2: [%d]\n", stru_sz);
        fi.close();
    }
    oo.resize(stru_sz); // ready to read the custom structure
    ret = parse_fmt(stru_ds.c_str(), ff, fn);
    printf("parse return %d,  stru_sz: %d\n", ret, stru_sz);

    // for(int j=0; j<ff.size(); j++){
    //     auto f = &ff[j];
    //     printf(" ---% 3d  %s  val[%s](%c)\n", f->offset, fn[j].c_str(), f->unpack(data+f->offset, f->dot_n), f->format);
    // }

    //  read index file
    fi.open(dfile+".idx", std::ios::binary|std::ios::in);
    if(!fi.is_open()){
        printf("open stru index file failed\n");
        return;
    }
    
    for(int i=0; i<4; i++){
        gpos = fi.tellg();
        fi.read((char *)&idx, sizeof(idx));
        printf("--%d %lld ms=%u sz=%u\n", i, idx.fpos, idx.ms, idx.sz);
    }
    // fi.read((char *)&idx, sizeof(idx));
    fi.close();

    // read data file
    fi.open(dfile,  std::ios::binary|std::ios::in);
    if(!fi.is_open()){
        printf("open stru file failed\n");
        return;
    }

    // 读取一批
    fi.seekg(idx.fpos, std::ios::beg);
    for(int i=0; i<idx.sz; i++){
        fi.read((char *)&co, sizeof(co));
        fi.read((char *)oo.data(), stru_sz);

        printf("id=%d lat=%.1f lgt=%.1f\n", co.id, co.lat, co.lgt);
        auto data = (char *)oo.data();

        for(int j=0; j<ff.size(); j++){
            auto f = &ff[j];
            printf(" ---% 3d  %s  val[%s](%c)\n", f->offset, fn[j].c_str(), f->unpack(data+f->offset, f->dot_n), f->format);
        }
    }


}


struct item {
 char a[1024];
 char b[1024];
 char c[1024[;
 char d[1024];
};

item line[512];

int main(){
    // test1();
    test2();
    return 0;

}
