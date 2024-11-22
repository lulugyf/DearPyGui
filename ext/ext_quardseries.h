#ifndef __QUARDSERIES__INCLUDE__
#define __QUARDSERIES__INCLUDE__

/*

用于在像素图片上绘制目标框
*/

#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

struct mvQuardSeriesConfig
{
    float   weight = 0.25f;
    bool    fill = true;
    
    std::shared_ptr<std::vector<char>> value = std::make_shared<std::vector<char>>(
            std::vector<char>{});
    void setValue(PyObject *bytes){
        auto &ret = *value;
        auto len = PyBytes_Size(bytes);
        ret.resize(len);
        ::memcpy(ret.data(), PyBytes_AsString(bytes), len);  
    };
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<char>>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvQuardSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvQuardSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvQuardSeriesConfig& outConfig);
}


class mvQuardSeries : public mvAppItem
{
public:
    mvQuardSeriesConfig configData{};
    explicit mvQuardSeries(mvUUID uuid) : mvAppItem(uuid) {}
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


