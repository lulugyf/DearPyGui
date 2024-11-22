#ifndef __SPANSERIES__INCLUDE__
#define __SPANSERIES__INCLUDE__


#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

struct mvSpanSeriesConfig
{
    float   weight = 0.25f;
    float   pos_y = 1.0f;
    float   span_y = 0.5f;
    bool    tooltip = true;
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

    void fill_configuration_dict(const mvSpanSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvSpanSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvSpanSeriesConfig& outConfig);
    void draw_dot_series(ImDrawList* drawlist, mvAppItem& item, const mvSpanSeriesConfig& config);
}


class mvSpanSeries : public mvAppItem
{
public:
    mvSpanSeriesConfig configData{};
    explicit mvSpanSeries(mvUUID uuid) : mvAppItem(uuid) {}
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


