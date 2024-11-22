#ifndef __OBSTACLEBOXSERIES__INCLUDE__
#define __OBSTACLEBOXSERIES__INCLUDE__

/*

用于在像素图片上绘制目标框
*/

#include <vector>
#include <memory>

#include <Python.h>
#include "mvPyUtils.h"
#include "../src/mvItemRegistry.h"

struct mvObstacleBoxSeriesConfig
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
    void set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<mvUUID>& outValue); // from mvPlotting.h

    void fill_configuration_dict(const mvObstacleBoxSeriesConfig& inConfig, PyObject* outDict);
    void set_configuration(PyObject* inDict, mvObstacleBoxSeriesConfig& outConfig);
    void set_required_configuration(PyObject* inDict, mvObstacleBoxSeriesConfig& outConfig);
    void draw_obstaclebox_series(ImDrawList* drawlist, mvAppItem& item, const mvObstacleBoxSeriesConfig& config);
}


class mvObstacleBoxSeries : public mvAppItem
{
public:
    mvObstacleBoxSeriesConfig configData{};
    explicit mvObstacleBoxSeries(mvUUID uuid) : mvAppItem(uuid) {}
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

    void draw(ImDrawList* drawlist, float x, float y) override { DearPyGui::draw_obstaclebox_series(drawlist, *this, configData); }

    // invoked by DearPyGui::GetEntityParser
    static void defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup);
};

#endif


