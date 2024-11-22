#ifndef __OBJSTRUSERIES__INCLUDE__
#define __OBJSTRUSERIES__INCLUDE__


#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

struct mvObjstruSeriesConfig
{
    float   weight = 0.25f;
    bool    fill = true;
    int     shape = 0;
    bool    tooltip = true;
    bool    showid = true; // 是否绘制id数字
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

    void fill_configuration_dict(const mvObjstruSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvObjstruSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvObjstruSeriesConfig& outConfig);
    void draw_dot_series(ImDrawList* drawlist, mvAppItem& item, const mvObjstruSeriesConfig& config);
}


class mvObjstruSeries : public mvAppItem
{
public:
    mvObjstruSeriesConfig configData{};
    explicit mvObjstruSeries(mvUUID uuid) : mvAppItem(uuid) {}
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


