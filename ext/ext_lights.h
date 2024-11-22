#ifndef __LIGHTS__INCLUDE__
#define __LIGHTS__INCLUDE__

/*

在plot中的窗口指定位置绘制信号灯及文字， 不跟随坐标系
*/

#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

struct mvLightsConfig
{
    unsigned int   size = 20; // 信号灯的大小（像素)
    int   pos = 2; // 信号灯位置,  2-top-right  3-bottom-left  4-bottom-right
    std::vector<std::string> labels; // 信号灯标签列表

    std::string lab;
    float txt_width = 0, txt_height = 0;

    std::shared_ptr<std::vector<int>> value = std::make_shared<std::vector<int>>(
            std::vector<int>{});
    void setValue(PyObject *vals);
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<int>>& outValue);

    void fill_configuration_dict(const mvLightsConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvLightsConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvLightsConfig& outConfig);
}


class mvLights : public mvAppItem
{
public:
    mvLightsConfig configData{};
    explicit mvLights(mvUUID uuid) : mvAppItem(uuid) {}
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


