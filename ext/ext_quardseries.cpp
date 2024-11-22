


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_quardseries.h"
#include <vector>




void DearPyGui::set_configuration(PyObject* inDict, mvQuardSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "fill"))  outConfig.fill = ToBool(item);
	
	if (PyObject* item = PyDict_GetItemString(inDict, "data")) {outConfig.setValue(item); }
	// printf("=== fill=%d shape=%d\n", outConfig.fill, outConfig.shape);
}


void DearPyGui::set_required_configuration(PyObject* inDict, mvQuardSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvQuardSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvQuardSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	PyDict_SetItemString(outDict, "fill",    mvPyObject(ToPyBool(inConfig.fill)));
}


void mvQuardSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any,       "data" });
	args.push_back({ mvPyDataType::Float,     "weight", mvArgType::KEYWORD_ARG, "0.25" });
	args.push_back({ mvPyDataType::Bool,      "fill",    mvArgType::KEYWORD_ARG, "True" });
	
	setup.about = "Adds a shape series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


struct _Obj {
	
	float x0;
	float y0;
	float x1;
	float y1;
	float x2;
	float y2;
	float x3;
	float y3;
	
	
};

namespace ext
{
    void AddQuadrilateral(ImDrawList *dl, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness);
}

static void PlotMyShapes(const char* label_id,  const mvQuardSeriesConfig &config)
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
		auto& col = ImPlot::GetCurrentItem()->Color;
		for (int i = 0; i < count; i++, item++) {
			
			ImVec2 p1 = ImPlot::PlotToPixels(item->x0, item->y0); // left-back 
			ImVec2 p2 = ImPlot::PlotToPixels(item->x1, item->y1); // right-back
			ImVec2 p3 = ImPlot::PlotToPixels(item->x2, item->y2); // right-front
			ImVec2 p4 = ImPlot::PlotToPixels(item->x3, item->y3); // left-front    

			//draw_list->AddQuadrilateral(p1, p2, p3, p4, col, 2);
			ext::AddQuadrilateral(draw_list, p1, p2, p3, p4, col, 2);
		
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvQuardSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	const mvQuardSeriesConfig& config = configData;

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

