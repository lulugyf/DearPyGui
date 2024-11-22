


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_laneseries.h"
#include <vector>

namespace Str{
	std::vector<std::string> split(const std::string &ss, std::string delimiter);
}

enum {
	LINE_MODE_EQUATION=4,
	LINE_MODE_DOTS=7,
	LINE_MODE_MEMLINE = 9,
};

void mvLaneSeriesConfig::setValue(PyObject *bytes){
	auto &ret = *value;
	if(PyBytes_Check(bytes)){
		auto len = PyBytes_Size(bytes);
		ret.resize(len);
		::memcpy(ret.data(), PyBytes_AsString(bytes), len); 
	}else if(PyUnicode_Check(bytes)){
		if(line_mode != LINE_MODE_MEMLINE){
			printf("invalid set memline with mode %d", line_mode);
			return;
		}
		auto len = PyUnicode_GET_LENGTH(bytes);
		auto fmt = std::string((const char *)PyUnicode_1BYTE_DATA(bytes), len);
		// (MEMLINE <dot-count> <x_addr> <y_addr> <y_type(i|f)>)
		auto fl = Str::split(fmt, " ");
		if(fl.size() != 5){
			printf("Invalid field-size, want 5 but %d", fl.size());
			return;
		}
		char ftype = fl[4].at(0);
		if(fl[0] != "MEMLINE" || (ftype != 'i' && ftype != 'f')){
			printf("Invalid format [%s]", fmt.c_str());
			return;
		}
		auto dot_count = std::stoi(fl[1]);
		void *x_addr = (void *)std::stoull(fl[2]);
		void *y_addr = (void *)std::stoull(fl[3]);
		y_type = ftype;

		x_float.resize(dot_count);
		memcpy(x_float.data(), x_addr, dot_count * sizeof(float));
		if(ftype == 'i'){
			y_int.resize(dot_count);
			memcpy(y_int.data(), y_addr, dot_count * sizeof(int));
		}else{
			y_float.resize(dot_count);
			memcpy(y_float.data(), y_addr, dot_count * sizeof(float));
		}
	}
}

void DearPyGui::set_configuration(PyObject* inDict, mvLaneSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "typeflag")) outConfig.typeflag = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "line_mode")){
		auto mode = ToInt(item);
		if(mode != LINE_MODE_EQUATION && mode != LINE_MODE_DOTS && mode != LINE_MODE_MEMLINE){
			printf("LaneSeries ERROR: invalid line_mode: %d\n", mode);
			outConfig.line_mode = LINE_MODE_EQUATION;
		}else
			outConfig.line_mode = mode;
	}

	if (PyObject* item = PyDict_GetItemString(inDict, "lanes")) {outConfig.setValue(item); }
}


void DearPyGui::set_required_configuration(PyObject* inDict, mvLaneSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvLaneSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void
DearPyGui::fill_configuration_dict(const mvLaneSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	PyDict_SetItemString(outDict, "tooltip",    mvPyObject(ToPyBool(inConfig.tooltip)));
	PyDict_SetItemString(outDict, "line_mode",    mvPyObject(ToPyInt(inConfig.line_mode)));
	PyDict_SetItemString(outDict, "typeflag",    mvPyObject(ToPyBool(inConfig.typeflag)));
}


void mvLaneSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any, "lanes" });
	args.push_back({ mvPyDataType::Float, "weight", mvArgType::KEYWORD_ARG, "0.01" });
	args.push_back({ mvPyDataType::Bool, "tooltip", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool, "typeflag", mvArgType::KEYWORD_ARG, "False" });
	args.push_back({ mvPyDataType::Integer, "line_mode", mvArgType::KEYWORD_ARG, "4" });
	// printf("sizeof(_Lane) = %d\n", sizeof(_Lane));

	setup.about = "Adds a lane series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////

// 曲线方程系数结构体： iiffffff(id,type,C0,C1,C2,C3,SP,EP)
struct _Lane {
	int id;
	int type; // 如果设定了 typeflag ， 则通过此进行区分;  1-车道线 2-路沿 ... 更多类别待扩展
	float C0;
	float C1;
	float C2;
	float C3;
	float SP; // start point
	float EP; // end point
};

/**
 * 以曲线方程系数绘制曲线
 */
static void PlotEquationLines(const char* label_id,  const mvLaneSeriesConfig &config)
{
	auto &val = *config.value;
	auto ln = (_Lane *)val.data();
	int count = val.size() / sizeof(_Lane);

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col = ImPlot::GetCurrentItem()->Color;

		//float w2 = config.weight / 2.f;
		// printf("----\n");
		for (int i = 0; i < count; i++, ln++) {
			// printf("  %d %.1f %.1f\n", ln->id, ln->SP, ln->EP);
			auto inerval = abs(ln->EP - ln->SP)/10;
			ImVec2 p1, p2;
			float weight = config.weight;
			if(config.typeflag && ln->type == 2) {
					weight *= 2;
			}
			for(int j = 0; j< 11; j++){ // 分10段 绘制
				auto y = ln->SP + j* inerval;
				auto x = -(ln->C3*y*y*y + ln->C2*y*y + ln->C1*y + ln->C0);
				p2 = ImPlot::PlotToPixels(x, y);
				if(j> 0)
					draw_list->AddLine(p1, p2, col, weight);
				p1 = p2;


				// if(j == 1){ // 绘制id
				// 	auto id_str = std::to_string(ln.id);
				// 	text->drawText(id_str.c_str(), img_x, img_y, 1.f, glm::vec3(r, g, b));
				// }
			}
		}

		// end plot item
		ImPlot::EndItem();
	}
}

// 结构体： IfiI(ff)*size (col-rgba, weight, size, reserve, size*(x,y))
struct _DotLineHead {
	unsigned int color; // r-g-b-a
	float weight;
	int size;
	unsigned int reserve;
	unsigned int col() {
		return IM_COL32((color>>16)&0xff, (color>>8)&0xff, color&0xff, (color>>24)&0xff);
	}
};
struct _Dot {
	float x;
	float y;
};
/**
 * 按点画线
 */
static void PlotDotLines(const char* label_id,  const mvLaneSeriesConfig &config)
{
	auto &val = *config.value;
	char *ptr_begin = val.data();
	char *ptr_end = ptr_begin + val.size(); // 用于判断边界
	// auto ln = (_Lane *)val.data();
	// int count = val.size() / sizeof(_Lane);

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col_default = ImPlot::GetCurrentItem()->Color;

		while(ptr_begin < ptr_end){
			// 一次循环一条线
			auto lh = (_DotLineHead *)ptr_begin;
			auto ld = (_Dot *)(ptr_begin + sizeof(_DotLineHead));
			if((char *)(ld + lh->size) > ptr_end){
				printf("LaneSeries ERROR: dots invalid dot count\n");
				ptr_begin = ptr_end;
				break;
			}
			ptr_begin += sizeof(_DotLineHead) + sizeof(_Dot) * lh->size; // 偏移指针

			auto col = lh->color == 0u ? col_default : lh->col();
			auto weight = lh->weight < 0.001f ? config.weight : lh->weight;
			//printf(" --- dot-lines: %d  weight: %.2f\n", lh->reserve, weight);
			ImVec2 p1 = ImPlot::PlotToPixels(ld->x, ld->y), p2;
			ld ++;
			for (int i = 1; i < lh->size; i++, ld++) {
				p2 = ImPlot::PlotToPixels(ld->x, ld->y);
				draw_list->AddLine(p1, p2, col, weight);
				p1 = p2;
			}
		}

		ImPlot::EndItem(); // end plot item
	}
}

/**
 * 按内存点画线
 */
static void PlotMemline(const char* label_id,  const mvLaneSeriesConfig &config)
{
	auto x = config.x_float.data();
	int dot_count = config.x_float.size();
	if(dot_count == 0)
		return;

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col_default = ImPlot::GetCurrentItem()->Color;
		auto weight = config.weight;
		if(config.y_type == 'i'){
			auto y = config.y_int.data();
			ImVec2 p1 = ImPlot::PlotToPixels(*x, *y), p2;
			x ++, y++;
			for(int i=1; i<dot_count; i++, x++, y++){
				p2 = ImPlot::PlotToPixels(*x, *y);
				draw_list->AddLine(p1, p2, col_default, weight);
				p1 = p2;
			}
		}else if(config.y_type == 'f'){
			auto y = config.y_float.data();
			ImVec2 p1 = ImPlot::PlotToPixels(*x, *y), p2;
			x ++, y++;
			for(int i=1; i<dot_count; i++, x++, y++){
				p2 = ImPlot::PlotToPixels(*x, *y);
				draw_list->AddLine(p1, p2, col_default, weight);
				p1 = p2;
			}
		}
		ImPlot::EndItem(); // end plot item
	}
}

void mvLaneSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	const mvLaneSeriesConfig& config = configData;
	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);

	{
		if(config.line_mode == LINE_MODE_EQUATION)
			PlotEquationLines(item.info.internalLabel.c_str(), config);
		else if(config.line_mode == LINE_MODE_DOTS)
			PlotDotLines(item.info.internalLabel.c_str(), config);
		else if(config.line_mode == LINE_MODE_MEMLINE)
			PlotMemline(item.info.internalLabel.c_str(), config);

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

