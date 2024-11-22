


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_rectseries.h"
#include <vector>




void DearPyGui::set_configuration(PyObject* inDict, mvRectSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "fill"))  outConfig.fill = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "raw_w"))  outConfig.raw_w = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "raw_h"))  outConfig.raw_h = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "draw_w"))  outConfig.draw_w = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "draw_h"))  outConfig.draw_h = ToInt(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "data")) {outConfig.setValue(item); }
	// printf("=== fill=%d shape=%d\n", outConfig.fill, outConfig.shape);
}


void DearPyGui::set_required_configuration(PyObject* inDict, mvRectSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvRectSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvRectSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	PyDict_SetItemString(outDict, "fill",    mvPyObject(ToPyBool(inConfig.fill)));
}


void mvRectSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any,       "data" });
	args.push_back({ mvPyDataType::Float,     "weight", mvArgType::KEYWORD_ARG, "0.25" });
	args.push_back({ mvPyDataType::Bool,      "fill",    mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Integer,      "raw_w",    mvArgType::KEYWORD_ARG, "0" });
	args.push_back({ mvPyDataType::Integer,      "raw_h",    mvArgType::KEYWORD_ARG, "0" });
	args.push_back({ mvPyDataType::Integer,      "draw_w",    mvArgType::KEYWORD_ARG, "0" });
	args.push_back({ mvPyDataType::Integer,      "draw_h",    mvArgType::KEYWORD_ARG, "0" });

	setup.about = "Adds a shape series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


struct _Obj {
	int id;
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
};

static void PlotMyShapes(const char* label_id,  const mvRectSeriesConfig &config)
{
	auto &val = *config.value;
	auto item = (_Obj *)val.data();
	int count = val.size() / sizeof(_Obj);
	char id_str[32];

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	ImPlotPoint mp = ImPlot::GetPlotMousePos();
	// printf("------\n");

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		// auto &col = ImPlot::GetCurrentItem()->Color;

		float w2 = config.weight / 2;
		float font_size = 0.0f;
		for (int i = 0; i < count; i++, item++) {
			auto w = ::abs(item->w);
			auto h = ::abs(item->h);
			ImVec2 p1 = ImPlot::PlotToPixels(item->x, item->y);
			ImVec2 p2 = ImPlot::PlotToPixels(item->x + w, item->y - h);
			// auto pos = ImPlot::GetPlotPos();
			// auto sz  = ImPlot::GetPlotSize();
			// printf("p2-pixel: %f,%f  pos: %f,%f  sz: %f,%f\n", p2.x, p2.y, pos.x,pos.y,  sz.x,sz.y);
			auto c = item->color;
			auto col = IM_COL32((c>>16)&0xff, (c>>8)&0xff, c&0xff, 255);
			if(!config.fill)
				draw_list->AddRect(p1, p2, col);
			else
				draw_list->AddRectFilled(p1, p2, col, 1.f);

			ImVec2 p3 = ImPlot::PlotToPixels(item->x, item->y);
			sprintf(id_str, "%d", (int)item->id);
			ImU32 colTxt = ImPlot::GetStyleColorU32(ImPlotCol_InlayText);
			draw_list->AddText(p3, colTxt, font_size, id_str);


			if(ImPlot::IsPlotHovered() 
				&& mp.x >= item->x && mp.x <= item->x + item->w
				&& mp.y <= item->y && mp.y >= item->y - item->h)
			{
				// printf("id: %d m.x=%.1f m.y=%.1f   x=%.1f  y=%.1f\n", item->id, mp.x, mp.y, item->x, item->y);

				// 绘制竖直顶头的光标
				float half_width = 0.33; 
				float  tool_l = ImPlot::PlotToPixels(mp.x - half_width * 1.5, 0).x;
				float  tool_r = ImPlot::PlotToPixels(mp.x + half_width * 1.5, 0).x;
				float  tool_t = ImPlot::GetPlotPos().y;
				float  tool_b = tool_t + ImPlot::GetPlotSize().y;
				ImPlot::PushPlotClipRect();
				draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 255, 64));
				ImPlot::PopPlotClipRect();

				// 添加tooltip
				ImGui::BeginTooltip();
				ImGui::Text("ID: %d", item->id);
				ImGui::Text("X: %.3f", item->x);
				ImGui::Text("y: %.3f", item->y);
				ImGui::EndTooltip();

			}

		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvRectSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	const mvRectSeriesConfig& config = configData;

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

