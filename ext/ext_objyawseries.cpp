


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"
#include "ext_obstacleseries.h"

#include "ext_objyawseries.h"
#include <vector>
#include <cmath>

#include <stdint.h>
// #include "mvAppItemCommons.h"



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

inline void _rotate(const float x, const float y, const float theta, float& x0, float& y0) {
	x0 = x * cosf(theta) - y * sinf(theta);
	y0 = y * cosf(theta) + x * sinf(theta);
}



void DearPyGui::set_configuration(PyObject* inDict, mvObjYawSeriesConfig& outConfig)
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

void DearPyGui::set_required_configuration(PyObject* inDict, mvObjYawSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvObjYawSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	
}

void DearPyGui::fill_configuration_dict(const mvObjYawSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
}

// mvObjYawSeries::mvObjYawSeries(mvUUID uuid)
// 	:
// 	mvAppItem(uuid)
// {
// 	// updatePoints();
// }


void mvObjYawSeries::defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup)
{
	args.push_back({ mvPyDataType::Any,       "item" });
	args.push_back({ mvPyDataType::Any,       "fmt",    mvArgType::KEYWORD_ARG, "" });

	setup.about = "Adds a obstacle series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////

// void mvObjYawSeries::updatePoints()
// {
// 	float xsi = _p1.x;
// 	float xfi = _p2.x;
// 	float ysi = _p1.y;
// 	float yfi = _p2.y;

// 	// length of arrow head
// 	double xoffset = _size;
// 	double yoffset = _size;

// 	// get pointer angle w.r.t +X (in radians)
// 	double angle = 0.0;
// 	if (xsi >= xfi && ysi >= yfi) {
// 		angle = atan((ysi - yfi) / (xsi - xfi));
// 	}
// 	else if (xsi < xfi && ysi >= yfi) {
// 		angle = M_PI + atan((ysi - yfi) / (xsi - xfi));
// 	}
// 	else if (xsi < xfi && ysi < yfi) {
// 		angle = -M_PI + atan((ysi - yfi) / (xsi - xfi));
// 	}
// 	else if (xsi >= xfi && ysi < yfi) {
// 		angle = atan((ysi - yfi) / (xsi - xfi));
// 	}

// 	// arrow head points
// 	auto x1 = (float)(xsi - xoffset * cos(angle));
// 	auto y1 = (float)(ysi - yoffset * sin(angle));

// 	_points[0] = { xsi, ysi, 0.0f, 1.0f };
// 	_points[1] = { (float)(x1 - 0.5 * _size * sin(angle)), (float)(y1 + 0.5 * _size * cos(angle)), 0.0f, 1.0f };
// 	_points[2] = { (float)(x1 + 0.5 * _size * cos((M_PI / 2.0) - angle)), (float)(y1 - 0.5 * _size * sin((M_PI / 2.0) - angle)), 0.0f, 1.0f };

// }


// void  mvObjYawSeries::PlotMyShapes(const char* label_id, mvAppItem& item, const mvObjYawSeriesConfig& config)
// {
	
// 	mvAppItem* pItem = GetItem(*GContext->itemRegistry, *config.value);
// 	auto objSeries = static_cast<mvObstacleSeries*>(pItem);
// 	mvObstacleSeriesConfig objConfig = objSeries->configData;
// 	auto& val = *objConfig.value;
// 	char* ptr = (char*)val.data();
// 	if (objConfig._stru_sz == 0) {
// 		printf("No valid fmt\n");
// 		return; // 没有成功解析fmt, 不绘制
// 	}
// 	if (objConfig._stru_sz != sizeof(FR_OBJECT))
// 	{
// 		printf("Size is erroe, need: %d, get: %d\n", objConfig._stru_sz, sizeof(FR_OBJECT));
// 		return;
// 	}
// 	auto data_sz = val.size();
// 	if (data_sz % (objConfig._stru_sz + sizeof(BaseOB)) > 0) {
// 		printf("invalid data size,  %d %% (24 + %d) > 0", data_sz, objConfig._stru_sz);
// 		return; // 数据可能不匹配, 
// 	}
// 	int count = data_sz / (sizeof(BaseOB) + objConfig._stru_sz);
// 	char id_str[32];

// 	ImDrawList* drawlist = ImPlot::GetPlotDrawList();

// 	ImPlotPoint mp = ImPlot::GetPlotMousePos(); // mouse pos

// 	// begin plot item
// 	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
// 		auto& col = ImPlot::GetCurrentItem()->Color;

// 		objConfig.hovered_id = -1;
// 		for (int i = 0; i < count; i++, ptr += sizeof(BaseOB) + objConfig._stru_sz) {
// 			BaseOB* objBase = (BaseOB*)ptr;
// 			FR_OBJECT* obj = (FR_OBJECT*)(ptr + sizeof(BaseOB));
// 			float vx = obj->FR_ObjDirVX;
// 			float vy = obj->FR_ObjDirVY;
			
// 			float disx = objBase->lat;
// 			float disy = objBase->lgt;

// 			_p1 = { disx, disy, 0.0f, 1.0f };
// 			_p2 = { disx + vx*0.23f, disy + vy*0.23f, 0.0f, 1.0f };

// 			// printf("dis=== %f   %f", disx, disy);
		
// 			updatePoints();

		
// 			if ((int)obj->FR_ObjMovingStatus==0 || (int)obj->FR_ObjHeadgYawAgl==0 || (int)obj->FR_ObjHeadgYawAgl==200)
// 				continue;
			
// 			if (objConfig.shape == 1 ) {// triangle (三角形不处理旋转角度)
				
// 				printf("mmmmmmmm\n");
// 				// mvVec4  tp1 = drawInfo->transform * _p1;
// 				// mvVec4  tp2 = drawInfo->transform * _p2;

// 				// mvVec4  tpp1 = drawInfo->transform * _points[0];
// 				// mvVec4  tpp2 = drawInfo->transform * _points[1];
// 				// mvVec4  tpp3 = drawInfo->transform * _points[2];
// 			// 	if (drawInfo->perspectiveDivide)
// 			// 	{
// 			// 		tp1.x = tp1.x / tp1.w;
// 			// 		tp2.x = tp2.x / tp2.w;
// 			// 		tpp1.x = tpp1.x / tpp1.w;
// 			// 		tpp2.x = tpp2.x / tpp2.w;
// 			// 		tpp3.x = tpp3.x / tpp3.w;

// 			// 		tp1.y = tp1.y / tp1.w;
// 			// 		tp2.y = tp2.y / tp2.w;
// 			// 		tpp1.y = tpp1.y / tpp1.w;
// 			// 		tpp2.y = tpp2.y / tpp2.w;
// 			// 		tpp3.y = tpp3.y / tpp3.w;

// 			// 		tp1.z = tp1.z / tp1.w;
// 			// 		tp2.z = tp2.z / tp2.w;
// 			// 		tpp1.z = tpp1.z / tpp1.w;
// 			// 		tpp2.z = tpp2.z / tpp2.w;
// 			// 		tpp3.z = tpp3.z / tpp3.w;
// 			// 	}

// 				// if (drawInfo->depthClipping)
// 				// {
// 				// 	if (mvClipPoint(drawInfo->clipViewport, tp1)) return;
// 				// 	if (mvClipPoint(drawInfo->clipViewport, tp2)) return;
// 				// 	if (mvClipPoint(drawInfo->clipViewport, tpp1)) return;
// 				// 	if (mvClipPoint(drawInfo->clipViewport, tpp2)) return;
// 				// 	if (mvClipPoint(drawInfo->clipViewport, tpp3)) return;
// 				// }
// 				printf("nnnnnnnn\n");
// 				//drawlist->AddTriangleFilled(ImPlot::PlotToPixels(tpp1), ImPlot::PlotToPixels(tpp2), ImPlot::PlotToPixels(tpp3), _color);
// 				drawlist->AddLine(ImPlot::PlotToPixels(_p1), ImPlot::PlotToPixels(_p2), col, (float)ImPlot::GetCurrentContext()->Mx * _thickness);
// 				//drawlist->AddTriangle(ImPlot::PlotToPixels(tpp1), ImPlot::PlotToPixels(tpp2), ImPlot::PlotToPixels(tpp3), _color, (float)ImPlot::GetCurrentContext()->Mx * _thickness);
// 			printf("zzzzzzzzzzzzzzzzzz\n");
				
// 			}
// 		}

// 		// end plot item
// 		ImPlot::EndItem();
// 	}
// }

static void PlotMyShapes(const char* label_id, mvAppItem& item, const mvObjYawSeriesConfig& config)
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

	ImDrawList* drawlist = ImPlot::GetPlotDrawList();

	ImPlotPoint mp = ImPlot::GetPlotMousePos(); // mouse pos

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto& col = ImPlot::GetCurrentItem()->Color;

		objConfig.hovered_id = -1;
		for (int i = 0; i < count; i++, ptr += sizeof(BaseOB) + objConfig._stru_sz) {
			BaseOB* objBase = (BaseOB*)ptr;
			FR_OBJECT* obj = (FR_OBJECT*)(ptr + sizeof(BaseOB));
			float vx = obj->FR_ObjDirVX;
			float vy = obj->FR_ObjDirVY;
			
			float disx = objBase->lat;
			float disy = objBase->lgt;

			mvVec4  _p1 = { disx, disy, 0.0f, 1.0f };
			mvVec4  _p2 = { disx + vx*0.23f, disy + vy*0.23f, 0.0f, 1.0f };
			mvVec2  _p3 = { disx + vx*0.23f, disy + vy*0.23f};

			// printf("dis=== %f   %f", disx, disy);
		
			// updatePoints();

		
			if ((int)obj->FR_ObjMovingStatus==0 || (int)obj->FR_ObjHeadgYawAgl==0 || (int)obj->FR_ObjHeadgYawAgl==200)
				continue;
			
			if (objConfig.shape == 1 ) {// triangle (三角形不处理旋转角度)
				
				
				//drawlist->AddTriangleFilled(ImPlot::PlotToPixels(tpp1), ImPlot::PlotToPixels(tpp2), ImPlot::PlotToPixels(tpp3), _color);
				drawlist->AddLine(ImPlot::PlotToPixels(_p1), ImPlot::PlotToPixels(_p2), col,  0.3f);
				//drawlist->AddTriangle(ImPlot::PlotToPixels(tpp1), ImPlot::PlotToPixels(tpp2), ImPlot::PlotToPixels(tpp3), _color, (float)ImPlot::GetCurrentContext()->Mx * _thickness);
				drawlist->AddCircle(ImPlot::PlotToPixels(_p3), 3, col, 25);
			
				
			}
		}

		// end plot item
		ImPlot::EndItem();
	}
}

void  DearPyGui::draw_objyaw_series(ImDrawList* drawlist, mvAppItem& item, const mvObjYawSeriesConfig& config) {

	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);
	{
		// mvObjYawSeries oo(0);
		// printf("before plot\n");
		// oo.PlotMyShapes(item.info.internalLabel.c_str(), item, config);
		// printf("after plot\n");

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

