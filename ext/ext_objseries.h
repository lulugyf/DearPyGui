#ifndef __OBJSERIES__INCLUDE__
#define __OBJSERIES__INCLUDE__


#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"

struct mvObjSeriesConfig
{
    float   weight = 0.25f;
    bool    tooltip = true;
    bool    fill = true;
    bool    txtShow = true;
    float   txtOffsetX = 0.2;  // id 标签横向偏移量
    float   txtOffsetY = -0.2; // id 标签纵向偏移量
    int     shape = 1; // 1- triangle  2- rectangle
    float   fontSize = 0.0f;
    std::shared_ptr<std::vector<std::vector<double>>> value = std::make_shared<std::vector<std::vector<double>>>(
        std::vector<std::vector<double>>{ 
            std::vector<double>{},
            std::vector<double>{},
            std::vector<double>{} 
        });
};



namespace DearPyGui {
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<std::vector<double>>>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvObjSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvObjSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvObjSeriesConfig& outConfig);
    void draw_obj_series     (ImDrawList* drawlist, mvAppItem& item, const mvObjSeriesConfig& config);

}


class mvObjSeries : public mvAppItem
{
public:
    mvObjSeriesConfig configData{};
    explicit mvObjSeries(mvUUID uuid) : mvAppItem(uuid) {}
    void handleSpecificRequiredArgs(PyObject* dict) override { DearPyGui::set_required_configuration(dict, configData); }
    void draw(ImDrawList* drawlist, float x, float y) override { DearPyGui::draw_obj_series(drawlist, *this, configData); }
    void handleSpecificKeywordArgs(PyObject* dict) override { DearPyGui::set_configuration(dict, configData); }
    void getSpecificConfiguration(PyObject* dict) override { DearPyGui::fill_configuration_dict(configData, dict); }
    void setDataSource(mvUUID dataSource) override { DearPyGui::set_data_source(*this, dataSource, configData.value); }
    void* getValue() override { return &configData.value; }
    PyObject* getPyValue() override { return ToPyList(*configData.value); }
    void setPyValue(PyObject* value) override { *configData.value = ToVectVectDouble(value); }

    // invoked by DearPyGui::GetEntityParser
    static void defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup);
};

#endif


