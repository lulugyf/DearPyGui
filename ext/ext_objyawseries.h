#ifndef __OBJYAWSERIES__INCLUDE__
#define __OBJYAWSERIES__INCLUDE__

/*

用于在像素图片上绘制目标框
*/

#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"
//#include "../mvItemRegistry.h"

struct mvObjYawSeriesConfig
{
    std::string fmt; // 目标数据的结构描述串

    int hovered_id = -1;
    char type[64];

    std::vector<formatdef> _ff;
    std::vector<std::string> _fn;
    int _stru_sz = 0;  // 目标结构的字节数
    std::shared_ptr<mvUUID> value = std::make_shared<mvUUID>(mvUUID(0));
    void setValue(PyObject* item) {
        mvUUID name = GetIDFromPyObject(item);
        value.reset(new mvUUID(name));
    };

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
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<mvUUID>& outValue); 

    void fill_configuration_dict(const mvObjYawSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvObjYawSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvObjYawSeriesConfig& outConfig);
    void draw_objyaw_series(ImDrawList* drawlist, mvAppItem& item, const mvObjYawSeriesConfig& config);
    
    
}


class mvObjYawSeries : public mvAppItem
{
public:
    mvObjYawSeriesConfig configData{};
    explicit mvObjYawSeries(mvUUID uuid) : mvAppItem(uuid) {}
    // explicit mvObjYawSeries(mvUUID uuid);
    void handleSpecificRequiredArgs(PyObject* dict) override { DearPyGui::set_required_configuration(dict, configData); }
    void handleSpecificKeywordArgs(PyObject* dict) override { DearPyGui::set_configuration(dict, configData); }
    void getSpecificConfiguration(PyObject* dict) override { DearPyGui::fill_configuration_dict(configData, dict); }
    void setDataSource(mvUUID dataSource) override { DearPyGui::set_data_source(*this, dataSource, configData.value); }
    void* getValue() override { return &configData.value; }
    PyObject* getPyValue() override {
        auto& ret = *configData.value;
        return ToPyInt(ret);
    }
    void setPyValue(PyObject* value) override { configData.setValue(value); }

    void draw(ImDrawList* drawlist, float x, float y) override { DearPyGui::draw_objyaw_series(drawlist, *this, configData); }

    // invoked by DearPyGui::GetEntityParser
    static void defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup);



    // void updatePoints();  
    // void PlotMyShapes(const char* label_id, mvAppItem& item, const mvObjYawSeriesConfig& config); 
    


    // mvVec4  _p1 = { 0.0f, 0.0f, 0.0f, 1.0f };
    // mvVec4  _p2 = { 0.0f, 0.0f, 0.0f, 1.0f };
    // mvColor _color = {255/255.0f, 0/255.0f, 0/255.0f, 255/255.0f};
    // float   _thickness = 1.0f;
    // float   _size = 4;
    // mvVec4  _points[3];

};

#endif


