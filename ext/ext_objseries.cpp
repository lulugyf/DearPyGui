


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_objseries.h"
#include <vector>



template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) { // 二分搜索， 数据必须是排序的
	if (r >= l) {
		int mid = l + (r - l) / 2;
		if (arr[mid] == x)
			return mid;
		if (arr[mid] > x)
			return BinarySearch(arr, l, mid - 1, x);
		return BinarySearch(arr, mid + 1, r, x);
	}
	return -2;
}



void DearPyGui::set_configuration(PyObject* inDict, mvObjSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "show_id")) outConfig.txtShow = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "id_offsetx")) outConfig.txtOffsetX = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "id_offsety")) outConfig.txtOffsetY = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "fill"))  outConfig.fill = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "weight")) outConfig.weight = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "font_size")) outConfig.fontSize = ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "shape")) outConfig.shape = ToInt(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "xx")) { (*outConfig.value)[0] = ToDoubleVect(item); }
	if (PyObject* item = PyDict_GetItemString(inDict, "yy")) { (*outConfig.value)[1] = ToDoubleVect(item); }
	if (PyObject* item = PyDict_GetItemString(inDict, "oid")) { (*outConfig.value)[2] = ToDoubleVect(item); }
}


void
DearPyGui::set_required_configuration(PyObject* inDict, mvObjSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvObjSeries)], inDict))
		return;

	(*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
	(*outConfig.value)[1] = ToDoubleVect(PyTuple_GetItem(inDict, 1));
	(*outConfig.value)[2] = ToDoubleVect(PyTuple_GetItem(inDict, 2));
}

void
DearPyGui::fill_configuration_dict(const mvObjSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "show_id", mvPyObject(ToPyBool(inConfig.txtShow)));
	PyDict_SetItemString(outDict, "id_offsetx", mvPyObject(ToPyFloat(inConfig.txtOffsetX)));
	PyDict_SetItemString(outDict, "id_offsety", mvPyObject(ToPyFloat(inConfig.txtOffsetY)));
	PyDict_SetItemString(outDict, "fill",  mvPyObject(ToPyBool(inConfig.fill)));
	PyDict_SetItemString(outDict, "weight",     mvPyObject(ToPyFloat(inConfig.weight)));
	PyDict_SetItemString(outDict, "tooltip",    mvPyObject(ToPyBool(inConfig.tooltip)));
	PyDict_SetItemString(outDict, "font_size",     mvPyObject(ToPyFloat(inConfig.fontSize)));
	PyDict_SetItemString(outDict, "shape",     mvPyObject(ToPyInt(inConfig.shape)));
}



void mvObjSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::DoubleList, "xx" });
	args.push_back({ mvPyDataType::DoubleList, "yy" });
	args.push_back({ mvPyDataType::DoubleList, "oid" });
	args.push_back({ mvPyDataType::Bool, "fill", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool, "show_id", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Float, "weight", mvArgType::KEYWORD_ARG, "0.25" });
	args.push_back({ mvPyDataType::Bool, "tooltip", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Float, "id_offsetx", mvArgType::KEYWORD_ARG, "0.2"});
	args.push_back({ mvPyDataType::Float, "id_offsety", mvArgType::KEYWORD_ARG, "-0.2"});
	args.push_back({ mvPyDataType::Float, "font_size", mvArgType::KEYWORD_ARG, "0.0"});
	args.push_back({ mvPyDataType::Integer, "shape", mvArgType::KEYWORD_ARG, "1"});

	setup.about = "Adds a Object shape series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}




static void PlotMybars(const char* label_id,  const mvObjSeriesConfig &config)
{

	const double* xx = (*config.value.get())[0].data();
	const double* yy  = (*config.value.get())[1].data();
	const double* oid = (*config.value.get())[2].data();
	int count = (*config.value.get())[0].size();

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();
	char id_str[64];
	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &cl = ImPlot::GetCurrentItem()->Color;
		if (ImPlot::FitThisFrame()) {
			for (int i = 0; i < count; ++i) {
				ImPlot::FitPoint(ImPlotPoint(xx[i], yy[i]));
				// ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
			}
		}
		// render data
		auto a1 = ImPlot::PlotToPixels(1.0, 1.0);
		auto a2 = ImPlot::PlotToPixels(.0, .0);
		ImVec2 offset{config.txtOffsetX * (a1[0]-a2[0]), config.txtOffsetY * abs(a1[1]-a2[1]) };
		float w = config.weight / 2.f;
		if(config.shape == 1){
			for (int i = 0; i < count; ++i) {
				ImVec2 p1 = ImPlot::PlotToPixels(xx[i]-w, yy[i]);
				ImVec2 p2 = ImPlot::PlotToPixels(xx[i]+w, yy[i]);
				ImVec2 p3 = ImPlot::PlotToPixels(xx[i], yy[i]+config.weight);
				if(!config.fill)
					draw_list->AddTriangleFilled(p1, p2, p3, cl);
				else
					draw_list->AddTriangle(p1, p2, p3, cl, 2.f);
				if(config.txtShow){
					sprintf(id_str, "%d", (int)oid[i]);
					ImPlot::PlotText(id_str, xx[i], yy[i], config.fontSize, false, offset);
				}
			}
		}else if(config.shape == 2){
			for (int i = 0; i < count; ++i) {
				ImVec2 p1 = ImPlot::PlotToPixels(xx[i]-w, yy[i]+config.weight);
				ImVec2 p2 = ImPlot::PlotToPixels(xx[i]+w, yy[i]);
				if(!config.fill)
					draw_list->AddRect(p1, p2, cl);
				else
					draw_list->AddRectFilled(p1, p2, cl, 2.f);
				if(config.txtShow){
					sprintf(id_str, "%d", (int)oid[i]);
					ImPlot::PlotText(id_str, xx[i], yy[i], config.fontSize, false, offset);
				}
			}
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void DearPyGui::draw_obj_series(ImDrawList* drawlist, mvAppItem& item, const mvObjSeriesConfig& config)
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

