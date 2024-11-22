


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_lights.h"
#include <vector>

namespace Str{
	std::vector<std::string> split(const std::string &ss, std::string delimiter);
}

void mvLightsConfig::setValue(PyObject *vals){
	// auto &ret = *value;
	*value = ToIntVect(vals);
	// auto len = PyBytes_Size(bytes);
	// ret.resize(len);
	// ::memcpy(ret.data(), PyBytes_AsString(bytes), len);  
}

void DearPyGui::set_configuration(PyObject* inDict, mvLightsConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "size")) outConfig.size = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "position"))  outConfig.pos = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "labels")){
		outConfig.lab = ToString(item);
		outConfig.labels.clear();
		//int txtw = 120;
		for(auto &s: Str::split(outConfig.lab, " ")){
			outConfig.labels.push_back(s);
			// auto sz = ImGui::CalcTextSize(s.c_str());
			// if(sz.y > txtw) txtw = sz.y;
		}
		outConfig.txt_width = 0;
	} 

	if (PyObject* item = PyDict_GetItemString(inDict, "data")) {outConfig.setValue(item); }
}


void DearPyGui::set_required_configuration(PyObject* inDict, mvLightsConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvRectSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvLightsConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "size",     mvPyObject(ToPyInt(inConfig.size)));
	PyDict_SetItemString(outDict, "position",    mvPyObject(ToPyInt(inConfig.pos)));
	PyDict_SetItemString(outDict, "labels",    mvPyObject(ToPyString(inConfig.lab)));
}


void mvLights::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any,       "data" });
	args.push_back({ mvPyDataType::Integer,     "size", mvArgType::KEYWORD_ARG, "20" });
	args.push_back({ mvPyDataType::Integer,     "position",  mvArgType::KEYWORD_ARG, "2" });
	args.push_back({ mvPyDataType::String,     "labels",  mvArgType::KEYWORD_ARG, "" });

	setup.about = "Adds a light series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////



static void PlotMyShapes(const char* label_id,  mvLightsConfig &config)
{
	auto &labs = config.labels;
	auto count = labs.size();
	if(config.value->size() < count)
		count = config.value->size();

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		// auto &col = ImPlot::GetCurrentItem()->Color;
		int cols[3] = {IM_COL32(128,128,128,255), IM_COL32(255,255,0,255), IM_COL32(255,0,0,255)};
		int col_txt = IM_COL32(128,255,128,255);
		auto pos = ImPlot::GetPlotPos();
		auto sz  = ImPlot::GetPlotSize();

		if(config.txt_width == 0){ // 计算文本的最大高度和宽度
			float txtw = 0, txth;
			for(auto &s: labs){
				auto sz = ImGui::CalcTextSize(s.c_str());
				if(sz.x > txtw) txtw = sz.x;
				txth = sz.y;
			}
			config.txt_width = txtw;
			config.txt_height = txth;
			printf(" pos.x=%f sz.x=%f txtw=%f txth=%f\n", pos.x, sz.x, txtw, txth);
		}
		auto tw = config.txt_width;

		// top-right mode
		float rd = config.size / 2.0f;
		float th = config.size;
		if(th < config.txt_height)  // item的高度从圆直径和文本高度中选大的
			th = config.txt_height;
		int tx = pos.x + sz.x - tw - config.size - 10;
		int ty = pos.y + 10;
		if(config.pos == 4){ // bottom-right
			ty = pos.y + sz.y - th * count - (count-1)*3 - 10;
		}else if(config.pos == 3){ // bottom-left
			tx = pos.x+10;
			ty = pos.y + sz.y - th * count - (count-1)*3 - 10;
		}
		for (int i = 0; i < count; i++) {
			ImVec2 pt{tx+rd, ty+rd};
			uint32_t col_idx = config.value->operator[](i);
			auto &lb = labs[i];
			col_idx %= 3;
			draw_list->AddCircleFilled(pt, rd, cols[col_idx], 12);
			draw_list->AddText(ImVec2{(float)(tx+config.size+3), (float)ty}, col_txt, lb.c_str());
			
			ty += th + 3;
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvLights::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	auto& config = configData;

	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);
	{

		PlotMyShapes(item.info.internalLabel.c_str(), config);

		// // Begin a popup for a legend entry.
		// if (ImPlot::BeginLegendPopup(item.info.internalLabel.c_str(), 1))
		// {
		// 	for (auto& childset : item.childslots)
		// 	{
		// 		for (auto& item : childset)
		// 		{
		// 			// skip item if it's not shown
		// 			if (!item->config.show)
		// 				continue;
		// 			item->draw(drawlist, ImPlot::GetPlotPos().x, ImPlot::GetPlotPos().y);
		// 			UpdateAppItemState(item->state);
		// 		}
		// 	}
		// 	ImPlot::EndLegendPopup();
		// }
	}

	// pop font off stack
	if (item.font)
		ImGui::PopFont();

	// handle popping themes
	cleanup_local_theming(&item);
}

