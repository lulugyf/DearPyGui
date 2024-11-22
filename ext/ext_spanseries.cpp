


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_spanseries.h"
#include <vector>




void DearPyGui::set_configuration(PyObject* inDict, mvSpanSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "pos_y"))  outConfig.pos_y = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "span_y"))  outConfig.span_y = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "data")) {outConfig.setValue(item); }
}


void
DearPyGui::set_required_configuration(PyObject* inDict, mvSpanSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvSpanSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void
DearPyGui::fill_configuration_dict(const mvSpanSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	// PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	// PyDict_SetItemString(outDict, "tooltip",    mvPyObject(ToPyBool(inConfig.tooltip)));
}


void mvSpanSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any, "data" });
	args.push_back({ mvPyDataType::Float, "weight", mvArgType::KEYWORD_ARG, "0.25" });
	args.push_back({ mvPyDataType::Float, "pos_y", mvArgType::KEYWORD_ARG, "1" });
	args.push_back({ mvPyDataType::Float, "span_y", mvArgType::KEYWORD_ARG, "0.5" });
	args.push_back({ mvPyDataType::Bool, "tooltip", mvArgType::KEYWORD_ARG, "True" });
	// printf("sizeof(_Lane) = %d\n", sizeof(_Lane));

	setup.about = "Adds a lane series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


struct _Span {
	int id;
	float begin;
	float end;
};

static void PlotMyShapes(const char* label_id,  const mvSpanSeriesConfig &config)
{
	auto &val = *config.value;
	auto item = (_Span *)val.data();
	int count = val.size() / sizeof(_Span);
	char id_str[32];

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col = ImPlot::GetCurrentItem()->Color;

		float w2 = config.weight;
		float font_size = 0.0f;
		for (int i = 0; i < count; i++, item++) {
			float y = config.pos_y + (i % 2)*0.5f;
			ImVec2 p1 = ImPlot::PlotToPixels(item->begin, y - config.weight);
			ImVec2 p2 = ImPlot::PlotToPixels(item->end, y);
			draw_list->AddRectFilled(p1, p2, col);
			::sprintf(id_str, "%d", item->id);
			draw_list->AddText(p1, col, font_size, id_str);
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvSpanSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	const mvSpanSeriesConfig& config = configData;

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

