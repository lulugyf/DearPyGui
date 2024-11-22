


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_timeseries.h"
#include <vector>



template <typename T>
int BinarySearch(const T* arr, int l, int r, T x) {
	if (r >= l) {
		int mid = l + (r - l) / 2;
		if (arr[mid] == x)
			return mid;
		if (arr[mid] > x)
			return BinarySearch(arr, l, mid - 1, x);
		return BinarySearch(arr, mid + 1, r, x);
	}
	return -1;
}



void
DearPyGui::set_configuration(PyObject* inDict, mvTimeSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip")) outConfig.tooltip = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "dates")) { (*outConfig.value)[0] = ToDoubleVect(item); }
	if (PyObject* item = PyDict_GetItemString(inDict, "lows")) { (*outConfig.value)[1] = ToDoubleVect(item); }
	if (PyObject* item = PyDict_GetItemString(inDict, "highs")) { (*outConfig.value)[2] = ToDoubleVect(item); }
}


void
DearPyGui::set_required_configuration(PyObject* inDict, mvTimeSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvTimeSeries)], inDict))
		return;

	(*outConfig.value)[0] = ToDoubleVect(PyTuple_GetItem(inDict, 0));
	(*outConfig.value)[1] = ToDoubleVect(PyTuple_GetItem(inDict, 1));
	(*outConfig.value)[2] = ToDoubleVect(PyTuple_GetItem(inDict, 2));
}


void
DearPyGui::fill_configuration_dict(const mvTimeSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "tooltip", mvPyObject(ToPyBool(inConfig.tooltip)));
}


void mvTimeSeries::defineArgs(std::vector<mvPythonDataElement>& args, mvPythonParserSetup& setup)
{
	args.push_back({ mvPyDataType::DoubleList, "dates" });
	args.push_back({ mvPyDataType::DoubleList, "lows" });
	args.push_back({ mvPyDataType::DoubleList, "highs" });
	args.push_back({ mvPyDataType::Bool, "tooltip", mvArgType::KEYWORD_ARG, "True" });

	setup.about = "Adds a time-span series to a plot.";
	setup.category = { "Plotting" };
}




static void
PlotTime(const char* label_id, const double* xs, const double* lows, const double* highs, int count,
	bool tooltip)
{

	ImDrawList* draw_list = ImPlot::GetPlotDrawList();
	// calc real value width
	double half_width = 0.33;


	// custom tooltip
	if (ImPlot::IsPlotHovered() && tooltip) {
		// 绘制白色光标
		ImPlotPoint point = ImPlot::GetPlotMousePos();
		point.x = ImPlot::RoundTo(point.x, 0);  // 四舍五入取整，获取当前鼠标对应的plot_x值
		float  tool_l = ImPlot::PlotToPixels(point.x - half_width * 1.5, 0).x;
		float  tool_r = ImPlot::PlotToPixels(point.x + half_width * 1.5, 0).x;
		float  tool_t = ImPlot::GetPlotPos().y;
		float  tool_b = tool_t + ImPlot::GetPlotSize().y;
		ImPlot::PushPlotClipRect();
		draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 255, 64));
		ImPlot::PopPlotClipRect();

		// 二分查找 获取当前mouse最近的x
		int idx = BinarySearch(xs, 0, count - 1, point.x);

		ImPlotItem* item = ImPlot::GetItem(label_id);
		if (idx != -1 && item->Show)
		{
			int rightIndex = idx;
			int leftIndex = idx;
			ImGui::BeginTooltip();  // 使用分钟计数
			ImGui::Text("%s", label_id);
			// 递增 rightIndex 直到找到不等于 point.x 的值
			while (rightIndex + 1 < count && xs[rightIndex + 1] == point.x) {
				rightIndex++;
			}

			// 递减 leftIndex 直到找到不等于 point.x 的值
			while (leftIndex - 1 >= 0 && xs[leftIndex - 1] == point.x) {
				leftIndex--;
			}

			for (int i = leftIndex; i <= rightIndex; ++i)
			{
				int low_tm_s = int(lows[i]);
				int low_tm_ms = (lows[i] - low_tm_s) * 1000;
				int high_tm_s = int(highs[i]);
				int high_tm_ms = (highs[i] - high_tm_s) * 1000;
				ImGui::Text("time: %d:%d:%03d~%d:%d:%03d", low_tm_s / 60, low_tm_s % 60, low_tm_ms,
					high_tm_s / 60, high_tm_s % 60, high_tm_ms);
			}
			ImGui::EndTooltip();
		}
	}

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Fill)) {
		// override legend icon color
		ImPlotItem* item = ImPlot::GetItem(label_id);
		ImPlot::GetCurrentItem()->Color = item->Color;
		// fit data if requested
		if (ImPlot::FitThisFrame()) {
			for (int i = 0; i < count; ++i) {
				ImPlot::FitPoint(ImPlotPoint(xs[i], lows[i]));
				ImPlot::FitPoint(ImPlotPoint(xs[i], highs[i]));
			}
		}
		// render data
		for (int i = 0; i < count; ++i) {
			ImVec2 low_pos = ImPlot::PlotToPixels(xs[i] - half_width, lows[i]);
			ImVec2 high_pos = ImPlot::PlotToPixels(xs[i] + half_width, highs[i]);
			draw_list->AddRectFilled(low_pos, high_pos, item->Color);
		}

		// end plot item
		ImPlot::EndItem();
	}
}

void DearPyGui::draw_time_series(ImDrawList* drawlist, mvAppItem& item, const mvTimeSeriesConfig& config)
{
	//-----------------------------------------------------------------------------
	// pre draw
	//-----------------------------------------------------------------------------
	if (!item.config.show)
		return;

	// push font if a font object is attached
	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}

	// themes
	apply_local_theming(&item);

	//-----------------------------------------------------------------------------
	// draw
	//-----------------------------------------------------------------------------
	{

		static const std::vector<double>* datesptr;
		static const std::vector<double>* lowptr;
		static const std::vector<double>* highptr;

		datesptr = &(*config.value.get())[0];
		lowptr = &(*config.value.get())[1];
		highptr = &(*config.value.get())[2];

		PlotTime(item.info.internalLabel.c_str(), datesptr->data(),
			lowptr->data(), highptr->data(), (int)datesptr->size(), config.tooltip);

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


	//-----------------------------------------------------------------------------
	// update state
	//   * only update if applicable
	//-----------------------------------------------------------------------------


	//-----------------------------------------------------------------------------
	// post draw
	//-----------------------------------------------------------------------------

	// pop font off stack
	if (item.font)
		ImGui::PopFont();

	// handle popping themes
	cleanup_local_theming(&item);
}
