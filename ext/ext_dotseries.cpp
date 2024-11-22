


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_dotseries.h"
#include <vector>




void DearPyGui::set_configuration(PyObject* inDict, mvDotSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "xoff")) outConfig.xoff = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "yoff")) outConfig.yoff = ToFloat(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "dots")) {outConfig.setValue(item); }
}


void
DearPyGui::set_required_configuration(PyObject* inDict, mvDotSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvDotSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
	// (*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
}

void
DearPyGui::fill_configuration_dict(const mvDotSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	PyDict_SetItemString(outDict, "tooltip",    mvPyObject(ToPyBool(inConfig.tooltip)));
}


void mvDotSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any, "dots" });
	args.push_back({ mvPyDataType::Float, "weight", mvArgType::KEYWORD_ARG, "0.01" });
	args.push_back({ mvPyDataType::Bool, "tooltip", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Float, "xoff", mvArgType::KEYWORD_ARG, "0.0" });
	args.push_back({ mvPyDataType::Float, "yoff", mvArgType::KEYWORD_ARG, "0.0" });

	setup.about = "Adds a dot cloud series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////



static void PlotMybars(const char* label_id,  const mvDotSeriesConfig &config)
{
	auto &val = *config.value;
	const float* xx = (const float *)val.data();
	int count = val.size() / 4;

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();
	// char id_str[64];
	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		//auto &cl = ImPlot::GetCurrentItem()->Color;
		float alpha = 0.8;
		ImColor 
			color_higher{0.8f, 0.4f, 0.4f, alpha}, // h > 4
		 	color_lower{0.4f, 0.0f, 1.0f, alpha},  // h < 0
		 	color_4{1.0, 0.0, 0.0, alpha}, // h: 3 ~ 4
		 	color_3{1.0, 0.0, 0.0, alpha}, // h: 2 ~ 3
		 	color_2{1.0, 0.0, 0.0, alpha}, // h: 1 ~ 2
		 	color_1{1.0, 0.0, 0.0, alpha} // h: 0 ~ 1
		 ;  //
		ImColor *cl = NULL;

		float w2 = config.weight / 2.f;
		for (int i = 0; i < count; i+=4) {
			float y = xx[i]+config.yoff, height=xx[i+1], x = xx[i+2]+config.xoff;
			ImVec2 p1 = ImPlot::PlotToPixels(x-w2, y-w2);
			ImVec2 p2 = ImPlot::PlotToPixels(x+w2, y+w2);
			if(height > 4.0f)
				cl = &color_higher;
			else if(height > 3.0f){
				cl = &color_4;
				cl->Value.y = 1.0f - (height-3.0f);
			}else if(height > 2.0f){
				cl = &color_3;
				cl->Value.x = height - 2.f;
			}else if(height > 1.0f){
				cl = &color_2;
				cl->Value.z = 1.0f - (height-1.0f);
			}else if(height > 0.0f){
				cl = &color_1;
				cl->Value.y = height;
			}else{
				cl = &color_lower;
			}

			draw_list->AddRectFilled(p1, p2, *cl);
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void DearPyGui::draw_dot_series(ImDrawList* drawlist, mvAppItem& item, const mvDotSeriesConfig& config)
{
	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);

	{

		PlotMybars(item.info.internalLabel.c_str(), config);

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

