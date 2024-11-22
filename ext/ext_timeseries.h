#ifndef __TIMESERIES__INCLUDE__
#define __TIMESERIES__INCLUDE__


#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"


struct mvTimeSeriesConfig
{
    bool    tooltip = true;
    std::shared_ptr<std::vector<std::vector<double>>> value = std::make_shared<std::vector<std::vector<double>>>(
        std::vector<std::vector<double>>{
            std::vector<double>{},
            std::vector<double>{},
            std::vector<double>{}});
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<std::vector<double>>>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvTimeSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvTimeSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvTimeSeriesConfig& outConfig);
    void draw_time_series(ImDrawList* drawlist, mvAppItem& item, const mvTimeSeriesConfig& config);
}


class mvTimeSeries : public mvAppItem
{
public:
    mvTimeSeriesConfig configData{};
    explicit mvTimeSeries(mvUUID uuid) : mvAppItem(uuid) {}
    void handleSpecificRequiredArgs(PyObject* dict) override { DearPyGui::set_required_configuration(dict, configData); }
    void draw(ImDrawList* drawlist, float x, float y) override { DearPyGui::draw_time_series(drawlist, *this, configData); }
    void handleSpecificKeywordArgs(PyObject* dict) override { DearPyGui::set_configuration(dict, configData); }
    void getSpecificConfiguration(PyObject* dict) override { DearPyGui::fill_configuration_dict(configData, dict); }
    void setDataSource(mvUUID dataSource) override { DearPyGui::set_data_source(*this, dataSource, configData.value); }
    void* getValue() override { return &configData.value; }
    PyObject* getPyValue() override { return ToPyList(*configData.value); }
    void setPyValue(PyObject* value) override { *configData.value = ToVectVectDouble(value); }
    static void defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup);
};
#endif


