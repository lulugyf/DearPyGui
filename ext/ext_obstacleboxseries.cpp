


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"
#include "ext_obstacleseries.h"

#include "ext_obstacleboxseries.h"
#include <vector>
#include <cmath>

#include <stdint.h>


struct BaseOB {
	uint32_t id;   // 目标id
	float lat;     // 横向距离,  单位m， 右正
	float lgt;     // 纵向距离， 单位m， 上（前）正
	float width;   // 横向宽度， 单位m
	float length;  // 纵向长度
	int16_t angle;   // 目标运动方向， 单位°， 正前方0， 正右90， 正左-90， 正后180
	uint8_t flag;    // 按位的标志， b0:1-CIPV;  b1: 0-静止 1-运动; b2~b4: 目标类别,  类型对应 OBTYPE; b5~b7: Movement state运动状态
	uint8_t reserve; // 保留
};


struct FR_OBJECT {
	uint8_t FR_ObjID;
	float FR_ObjDirY;
	uint8_t FR_ObjSource;
	float FR_ObjDirX;
	uint8_t FR_ObjMovingStatus;
	float FR_ObjDirVY;
	float FR_ObjDirVX;
	float FR_ObjDirAY;
	float FR_ObjDirAX;
	float FR_ObjHeadgYawAgl;
	float FR_ObjAzimAgl;
	float FR_ObjElemAgl;
	uint8_t FR_ObjExistprob;
	float FR_ObjLength;
	uint8_t FR_ObjObstcl;
	float FR_ObjBoundingBox_Y_LF;
	uint8_t FR_ObjNewBuildFlag;
	float FR_ObjBoundingBox_Y_LB;
	uint8_t FR_ObjPredictionFlag;
	float FR_ObjBoundingBox_Y_RB;
	float FR_ObjBoundingBox_Y_RF;
	float FR_ObjBoundingBox_X_LF;
	float FR_ObjHeight;
	float FR_ObjBoundingBox_X_RB;
	float FR_ObjWidth;
	float FR_ObjBoundingBox_X_RF;
	float FR_ObjBoundingBox_X_LB;
	uint8_t FR_ObjNature;
	float FR_ObjRcs;
	uint16_t FR_ObjAliveCylce;

};

namespace ext
{
    void AddQuadrilateral(ImDrawList *dl, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness);
}

inline void _rotate(const float x, const float y, const float theta, float& x0, float& y0) {
	x0 = x * cosf(theta) - y * sinf(theta);
	y0 = y * cosf(theta) + x * sinf(theta);
}

void
DearPyGui::set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<mvUUID>& outValue)
{
	if (dataSource == item.config.source) return;
	item.config.source = dataSource;

	mvAppItem* srcItem = GetItem((*GContext->itemRegistry), dataSource);
	if (!srcItem)
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
			"Source item not found: " + std::to_string(dataSource), &item);
		return;
	}
	if (DearPyGui::GetEntityValueType(srcItem->type) != DearPyGui::GetEntityValueType(item.type))
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
			"Values types do not match: " + std::to_string(dataSource), &item);
		return;
	}
	outValue = *static_cast<std::shared_ptr<mvUUID>*>(srcItem->getValue());
}

void DearPyGui::set_configuration(PyObject* inDict, mvObstacleBoxSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;

	if (PyObject* item = PyDict_GetItemString(inDict, "fmt")) {
		if (PyBytes_Check(item)) {
			auto len = PyBytes_Size(item);
			outConfig.fmt = std::string(PyBytes_AsString(item), len);
			printf("----fmt: [%s]\n", outConfig.fmt.c_str());
			auto sz = unpackf(outConfig.fmt.c_str(), outConfig._ff, outConfig._fn);
			outConfig._stru_sz = (sz < 0) ? 0 : sz;
			if (sz < 0)
				printf("----Invalid fmt, parse return %d\n", sz);

			float theta = (-30.f / 180.f) * 3.14159f, x0, y0;
			_rotate(-5, 0, theta, x0, y0);
			printf("x0=%.1f  y0=%.1f\n", x0, y0);
			theta = (-90.f / 180.f) * 3.14159f;
			_rotate(-5, 0, theta, x0, y0);
			printf("x0=%.1f  y0=%.1f\n", x0, y0);
		}
		else {
			printf("fmt must be bytes!!!\n");
			outConfig._stru_sz = 0;
		}
	}
}

void DearPyGui::set_required_configuration(PyObject* inDict, mvObstacleBoxSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvObstacleBoxSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvObstacleBoxSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
}


void mvObstacleBoxSeries::defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup)
{
	args.push_back({ mvPyDataType::Any,       "item" });
	args.push_back({ mvPyDataType::Any,       "fmt",    mvArgType::KEYWORD_ARG, "" });

	setup.about = "Adds a obstacle series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


static void PlotMyShapes(const char* label_id, mvAppItem& item, const mvObstacleBoxSeriesConfig& config)
{
	mvAppItem* pItem = GetItem(*GContext->itemRegistry, *config.value);
	auto objSeries = static_cast<mvObstacleSeries*>(pItem);
	mvObstacleSeriesConfig objConfig = objSeries->configData;
	auto& val = *objConfig.value;
	char* ptr = (char*)val.data();
	if (objConfig._stru_sz == 0) {
		printf("No valid fmt\n");
		return; // 没有成功解析fmt, 不绘制
	}
	if (objConfig._stru_sz != sizeof(FR_OBJECT))
	{
		printf("Size is erroe, need: %d, get: %d\n", objConfig._stru_sz, sizeof(FR_OBJECT));
		return;
	}
	auto data_sz = val.size();
	if (data_sz % (objConfig._stru_sz + sizeof(BaseOB)) > 0) {
		printf("invalid data size,  %d %% (24 + %d) > 0", data_sz, objConfig._stru_sz);
		return; // 数据可能不匹配, 
	}
	int count = data_sz / (sizeof(BaseOB) + objConfig._stru_sz);
	char id_str[32];

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	ImPlotPoint mp = ImPlot::GetPlotMousePos(); // mouse pos

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto& col = ImPlot::GetCurrentItem()->Color;

		float w2 = 0.75; // half width
		float l2 = 2.5; // length
		float font_size = 0.0f;
		objConfig.hovered_id = -1;
		for (int i = 0; i < count; i++, ptr += sizeof(BaseOB) + objConfig._stru_sz) {
			BaseOB* objBase = (BaseOB*)ptr;
			FR_OBJECT* obj = (FR_OBJECT*)(ptr + sizeof(BaseOB));
			float len = obj->FR_ObjWidth/2.0;
			float width = obj->FR_ObjLength/2.0;
			//X_LF, Y_LF           X_RF, Y_RF
			//X_LB, Y_LB           X_RB, Y_RB
			if (objConfig.shape == 1 && (objBase->flag & 2)) {// triangle (三角形不处理旋转角度)
				// ImVec2 p1 = ImPlot::PlotToPixels(objBase->lat + obj->FR_ObjBoundingBox_X_LB, objBase->lgt + obj->FR_ObjBoundingBox_Y_LB); // left-back 
				// ImVec2 p2 = ImPlot::PlotToPixels(objBase->lat + obj->FR_ObjBoundingBox_X_RB, objBase->lgt + obj->FR_ObjBoundingBox_Y_RB); // right-back
				// ImVec2 p3 = ImPlot::PlotToPixels(objBase->lat + obj->FR_ObjBoundingBox_X_RF, objBase->lgt + obj->FR_ObjBoundingBox_Y_RF); // right-front
				// ImVec2 p4 = ImPlot::PlotToPixels(objBase->lat + obj->FR_ObjBoundingBox_X_LF, objBase->lgt + obj->FR_ObjBoundingBox_Y_LF); // left-front    

				
				ImVec2 p1 = ImPlot::PlotToPixels(objBase->lat - len, objBase->lgt - width); // left-back 
				ImVec2 p2 = ImPlot::PlotToPixels(objBase->lat + len, objBase->lgt - width); // right-back
				ImVec2 p3 = ImPlot::PlotToPixels(objBase->lat + len, objBase->lgt + width); // right-front
				ImVec2 p4 = ImPlot::PlotToPixels(objBase->lat - len, objBase->lgt + width); // left-front    

				// draw_list->AddQuadrilateral(p1, p2, p3, p4, col, 2);
				ext::AddQuadrilateral(draw_list, p1, p2, p3, p4, col, 2);
			}
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  DearPyGui::draw_obstaclebox_series(ImDrawList* drawlist, mvAppItem& item, const mvObstacleBoxSeriesConfig& config) {

	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);
	{

		PlotMyShapes(item.info.internalLabel.c_str(), item, config);

		// Begin a popup for a legend entry.
		if (ImPlot::BeginLegendPopup(item.info.internalLabel.c_str(), 1))
		{
			for (auto& childset : item.childslots)
			{
				for (auto& item : childset)
				{
					// skip item if it's not shown
					if (!item->config.show)
						continue;
					item->draw(drawlist, ImPlot::GetPlotPos().x, ImPlot::GetPlotPos().y);
					UpdateAppItemState(item->state);
				}
			}
			ImPlot::EndLegendPopup();
		}
	}

	// pop font off stack
	if (item.font)
		ImGui::PopFont();

	// handle popping themes
	cleanup_local_theming(&item);
}

