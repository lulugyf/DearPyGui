


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_objstruseries.h"
#include <vector>




void DearPyGui::set_configuration(PyObject* inDict, mvObjstruSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "fill"))  outConfig.fill = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "shape"))  outConfig.shape = ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "showid"))  outConfig.showid = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "data")) {outConfig.setValue(item); }
	// printf("=== fill=%d shape=%d\n", outConfig.fill, outConfig.shape);
}


void DearPyGui::set_required_configuration(PyObject* inDict, mvObjstruSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvObjstruSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvObjstruSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	// PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	// PyDict_SetItemString(outDict, "tooltip",    mvPyObject(ToPyBool(inConfig.tooltip)));
}


void mvObjstruSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any,       "data" });
	args.push_back({ mvPyDataType::Float,     "weight", mvArgType::KEYWORD_ARG, "0.25" });
	args.push_back({ mvPyDataType::Bool,      "fill",    mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Integer,   "shape",    mvArgType::KEYWORD_ARG, "1-triangle 2-rectangle" }); // 
	args.push_back({ mvPyDataType::Bool,      "tooltip", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool,      "showid",    mvArgType::KEYWORD_ARG, "True" });

	setup.about = "Adds a shape series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


struct _Obj {
	int id;
	float x;
	float y;
};

static void PlotMyShapes(const char* label_id,  const mvObjstruSeriesConfig &config)
{
	auto &val = *config.value;
	auto item = (_Obj *)val.data();
	int count = val.size() / sizeof(_Obj);
	char id_str[32];

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();
	// ImPlotPoint point = ImPlot::GetPlotMousePos();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col = ImPlot::GetCurrentItem()->Color;

		float w2 = config.weight / 2;
		float w1 = config.weight;
		int shape = config.shape;
		float font_size = 0.0f;
		for (int i = 0; i < count; i++, item++) {
			
			if(shape == 1) {// triangle
				ImVec2 p1 = ImPlot::PlotToPixels(item->x-w2, item->y);  // left-bottom
				ImVec2 p2 = ImPlot::PlotToPixels(item->x+w2, item->y); // right-bottom
				ImVec2 p3 = ImPlot::PlotToPixels(item->x, item->y+config.weight); // top-center
				if(config.fill)
					draw_list->AddTriangleFilled(p1, p2, p3, col);
				else
					draw_list->AddTriangle(p1, p2, p3, col, 2.f);
			}else if(shape == 2) { // rectangle
				ImVec2 p1 = ImPlot::PlotToPixels(item->x-w2, item->y+config.weight);
				ImVec2 p2 = ImPlot::PlotToPixels(item->x+w2, item->y);
				if(!config.fill)
					draw_list->AddRect(p1, p2, col);
				else
					draw_list->AddRectFilled(p1, p2, col, 1.f);
			}else if(shape == 3){ // circle
				ImVec2 p1 = ImPlot::PlotToPixels(item->x, item->y);
				if(config.fill)
					draw_list->AddCircleFilled(p1, w1, col, 6);
				else
					draw_list->AddCircle(p1, w1, col, 6);
			}
			if(config.showid){  //draw ID
				ImVec2 p3 = ImPlot::PlotToPixels(item->x, item->y);
				p3.x += 7; // 标签向右移动一点， 像素值
				sprintf(id_str, "%d", (int)item->id);
				ImU32 colTxt = ImPlot::GetStyleColorU32(ImPlotCol_InlayText);
				draw_list->AddText(p3, colTxt, font_size, id_str);
			}
					
			// if(config.tooltip) {

			// }
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvObjstruSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	const mvObjstruSeriesConfig& config = configData;

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

