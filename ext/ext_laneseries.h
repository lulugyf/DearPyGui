#ifndef __LANESERIES__INCLUDE__
#define __LANESERIES__INCLUDE__


#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

/**
 * 
 * 绘制多条曲线的组件
 * 
 * 
 */

struct mvLaneSeriesConfig
{
    float   weight = 0.25f;
    bool    tooltip = true;
    int line_mode = 4;   /* 4- fomula 提供曲线方程系数的曲线绘制, 结构体： iiffffff(id,type,C0,C1,C2,C3,SP,EP)
                            7- 提供点列表的曲线， 结构体： IfiI(ff)*size (col-rgba, weight, size, reserve, size*(x,y))
                            9- 提供内存地址画线， 数据格式： MEMLINE 2000 2113401765888 2113402216256 102  (MEMLINE <dot-count> <x_addr> <y_addr> <y_type(i|f)>)
    */
    bool    typeflag = false;  // 是否使用 struct _Lane  中的 type 属性

    std::shared_ptr<std::vector<char>> value = std::make_shared<std::vector<char>>(
            std::vector<char>{});
    std::vector<float> x_float;
    std::vector<int>  y_int;
    std::vector<float> y_float;
    char y_type = 0; // 'i' - y_int;  'f' - y_float
    void setValue(PyObject *bytes);
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<char>>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvLaneSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvLaneSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvLaneSeriesConfig& outConfig);
    void draw_dot_series(ImDrawList* drawlist, mvAppItem& item, const mvLaneSeriesConfig& config);
}


class mvLaneSeries : public mvAppItem
{
public:
    mvLaneSeriesConfig configData{};
    explicit mvLaneSeries(mvUUID uuid) : mvAppItem(uuid) {}
    void handleSpecificRequiredArgs(PyObject* dict) override { DearPyGui::set_required_configuration(dict, configData); }
    void handleSpecificKeywordArgs(PyObject* dict) override { DearPyGui::set_configuration(dict, configData); }
    void getSpecificConfiguration(PyObject* dict) override { DearPyGui::fill_configuration_dict(configData, dict); }
    void setDataSource(mvUUID dataSource) override { DearPyGui::set_data_source(*this, dataSource, configData.value); }
    void* getValue() override { return &configData.value; }
    PyObject* getPyValue() override {
        auto &ret = *configData.value;
        return Py_BuildValue("y#", ret.data(), ret.size()); }
    void setPyValue(PyObject* value) override { configData.setValue(value); }

    void draw(ImDrawList* drawlist, float x, float y) override;
    // invoked by DearPyGui::GetEntityParser
    static void defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup);
};

#endif


