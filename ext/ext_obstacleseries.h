#ifndef __OBSTACLESERIES__INCLUDE__
#define __OBSTACLESERIES__INCLUDE__

/*

用于在像素图片上绘制目标框

 todo： 
 1 名称需要可以映射为中文； 
 2 enum的名称展开； 
 3 顺序可以设定  -- 通过添加顺序设定
 4 设置数值单位

 设定字符串： orig_name|名称|unit_name|eval|ename|eval|ename|...
   当配置串为空时： 清空现有配置
*/

#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

typedef struct _formatdef {
    char format;
    int size;
    int alignment;
    char* (*unpack)(const char *, char);
    short offset;
    char dot_n;
} formatdef;

int unpackf(const char *fmt, std::vector<formatdef> &ff, std::vector<std::string> &fn);

struct FldDesc { // 字段描述
    int fld_idx; // 在 _ff 中的序号, -1无效
    std::string mapped_name; // 映射的名称， 通常中文名称, 为空则不显示
    std::string unit_name; // 单位名称
    std::map<int, std::string> enum_names; // enum name map
};
struct ObjDesc { // 附加的目标描述文字
    int obj_id;
    std::string desc;
};


struct mvObstacleSeriesConfig
{
    bool  fill = true;  // 是否填充
    bool  tooltip = true; // 是否显示 鼠标提示 信息 
    bool  show_id = true; // 是否绘制id数字
    bool  show_angle = false; // 是否按偏转角度绘制
    bool  show_size = false;  // 是否按目标的长宽绘制
    bool  show_CIPV = false;  // 是否标识 CIPV 目标
    bool  show_move_state = false; // 是否标识运动状态
    bool  show_static = false;   // 是否绘制静态目标（暂时不用）
    unsigned int selected_id = 0u; // 单击选中的目标
    uint8_t shape = 0;   // 形状，  1-三角形  2-矩形
    std::string obj_desc; // 指定目标旁边添加文字描述(可以有多个， str中可以有\n)， 格式： <obj-id> <str>[;<obj-id> <str>[...]]
    
    float len = 2.5;
    float width = 0.75;

    std::string fmt; // 目标数据的结构描述串

    int hovered_id = -1;
    char type[64];

    std::vector<formatdef> _ff;
    std::vector<std::string> _fn;
    int _stru_sz = 0;  // 目标结构的字节数
    std::vector<FldDesc> flds; //  设定显示的字段配置列表
    std::vector<ObjDesc> _objDesc; // 从 obj_desc 中解析的结果放在这里

    mvAppItem *relTbl = nullptr; // 相对应的表格对象

    std::shared_ptr<std::vector<char>> value = std::make_shared<std::vector<char>>(
            std::vector<char>{});
    void setValue(PyObject *bytes){
        auto &ret = *value;
        auto len = PyBytes_Size(bytes);
        ret.resize(len);
        ::memcpy(ret.data(), PyBytes_AsString(bytes), len);
        setTblCells();
    }
    void setTblCells(); // 更新数据到表格中

    void setType(const char* type)
    {
        memcpy(this->type, type, 64);
    }

    void setId(int id)
    {
        hovered_id = id;
    }
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<char>>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvObstacleSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvObstacleSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvObstacleSeriesConfig& outConfig);
}


class mvObstacleSeries : public mvAppItem
{
public:
    mvObstacleSeriesConfig configData{};
    explicit mvObstacleSeries(mvUUID uuid) : mvAppItem(uuid) {}
    void handleSpecificRequiredArgs(PyObject* dict) override { DearPyGui::set_required_configuration(dict, configData); }
    void handleSpecificKeywordArgs(PyObject* dict) override { DearPyGui::set_configuration(dict, configData); }
    void getSpecificConfiguration(PyObject* dict) override { DearPyGui::fill_configuration_dict(configData, dict); }
    void setDataSource(mvUUID dataSource) override { DearPyGui::set_data_source(*this, dataSource, configData.value); }
    void* getValue() override { return &configData.value; }
    PyObject* getPyValue() override;
    void setPyValue(PyObject* value) override { configData.setValue(value); }

    void draw(ImDrawList* drawlist, float x, float y) override;

    // invoked by DearPyGui::GetEntityParser
    static void defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup);
};

#endif


