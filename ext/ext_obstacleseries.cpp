


#include <utility>
#include "../src/mvCore.h"
#include "../src/mvContext.h"
#include "../src/mvFontItems.h"
#include "../src/mvThemes.h"

#include "ext_obstacleseries.h"
#include <vector>
#include <cmath>

#include <stdint.h>
#include <sstream>
#include <iostream>
#include "log.h"

using std::istringstream;
using std::string;
using std::getline;

int stru_to_int(formatdef *f, const char *data_ptr);
namespace Str{
	std::vector<std::string> split(const std::string &ss, std::string delimiter);
}

// 用于绘制目标的公共结构体，  只跟绘制俯视图有关
enum OBTYPE { // Obstacle Type
    OT_VEHICLE = 2,     // 车辆
    OT_PEDESTRIAN = 3,  // 行人
    OT_CYCLIST = 4,     // 骑行者
    OT_UNKNOWN = 0,    // 未知（或其它）类别
};

enum MVSTATE { // Moving State
    MS_UNKNOWN = 0,
    MS_STATIONARY = 1, //静止
    MS_SAMEDIR= 2,     // 同向
    MS_OPPOSITE = 3,   //对象
    MS_CROSS = 4,      // 横穿
};

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

inline void _rotate(const float x, const float y, const float theta, float &x0, float &y0){
	x0 = x * cosf(theta) - y * sinf(theta);
	y0 = y * cosf(theta) + x * sinf(theta);
}


void DearPyGui::set_configuration(PyObject* inDict, mvObstacleSeriesConfig& outConfig)
{
	if (inDict == nullptr)
		return;

	if (PyObject* item = PyDict_GetItemString(inDict, "fill"))  outConfig.fill = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "tooltip"))  outConfig.tooltip = ToBool(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "show_id"))  outConfig.show_id = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "show_angle"))  outConfig.show_angle = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "show_size"))  outConfig.show_size = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "show_CIPV"))  outConfig.show_CIPV = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "show_move_state"))  outConfig.show_move_state = ToBool(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "shape"))  outConfig.shape = (uint8_t)ToInt(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "len"))  outConfig.len = (float)ToFloat(item);
	if (PyObject* item = PyDict_GetItemString(inDict, "width"))  outConfig.width = (float)ToFloat(item);

	if (PyObject* item = PyDict_GetItemString(inDict, "table")){ // 设置关联表格
		std::lock_guard<std::recursive_mutex> lk(GContext->mutex);
		mvUUID uid = GetIDFromPyObject(item);
		if(uid != 0){
			auto mvItem = GetItem((*GContext->itemRegistry), uid);
			if(mvItem != nullptr){
				if(mvItem->type == mvAppItemType::mvTable) {
					outConfig.relTbl = mvItem;
					_LOG("set relTbl %lld success", uid);
				}
			}
		}
	}

	if (PyObject* item = PyDict_GetItemString(inDict, "fmt")){
		if(PyBytes_Check(item)){
			auto len = PyBytes_Size(item);
			outConfig.fmt = std::string(PyBytes_AsString(item), len);
		}else if(PyUnicode_Check(item)){
			auto len = PyUnicode_GET_LENGTH(item);
			outConfig.fmt = std::string((const char *)PyUnicode_1BYTE_DATA(item), len);
		}else{
			_LOG("invalid fmt type");
			outConfig._stru_sz = 0;
			outConfig.fmt = "";
		}
		if(outConfig.fmt.length() > 0){
			auto sz = unpackf(outConfig.fmt.c_str(), outConfig._ff, outConfig._fn);
			outConfig._stru_sz = (sz < 0) ? 0: sz;
			printf("fmt data size,  %d ", outConfig._stru_sz);
			_LOG("fmt data size,  %d ", outConfig._stru_sz);
			if(sz < 0){
				_LOG("----Invalid fmt [%s], parse return %d\n", outConfig.fmt.c_str(), sz);
				outConfig._stru_sz = 0;
				outConfig.fmt = "";
			}
		}
	}
	if (PyObject* item = PyDict_GetItemString(inDict, "fld_desc")){ // 设定单个字段描述
		// 要求在设定 fmt 之后再操作
		if(outConfig._fn.size() == 0){
			_LOG("set fld_desc after fmt.....");
		}else{
			if(PyUnicode_Check(item)){
				// RdrObjID|目标ID|unit_name|eval|ename|eval|ename|...
				// auto ss = string((const char *)PyUnicode_1BYTE_DATA(item), PyUnicode_GET_LENGTH(item));
				auto ss = ToString(item);
				if(ss.length() == 0){
					outConfig.flds.clear();
					_LOG(" clear flds");
				}else{
					istringstream is(ss);
					FldDesc fld;
					string orig_name;
					int i = 0;
					
					getline(is, orig_name, '|');
					fld.fld_idx = -1;
					i = 0;
					for(auto &n: outConfig._fn){
						if(orig_name == n){
							fld.fld_idx = i;
							break;
						}
						i ++;
					}
					//_LOG("set fld: [%s] idx: %d  sz: %d", ss.c_str(), fld.fld_idx, outConfig.flds.size());
					if(fld.fld_idx != -1){
						string eval, ename;
						bool end = is.eof();
						getline(is, fld.mapped_name, '|');
						getline(is, fld.unit_name, '|');
						if(!end && fld.mapped_name.length() > 0){ // 新名称有效才添加， 空白的直接略过
							i = 0;
							while(true){
								if(is.eof()) break;
								getline(is, eval, '|'); getline(is, ename, '|');
								if(eval.length() == 0 || ename.length() == 0)
									break;
								// _LOG(" %d  %s = %s  %d", i++, eval.c_str(), ename.c_str(), eval.length());
								fld.enum_names[std::stoi(eval)] = ename;
							}
							outConfig.flds.push_back(fld);
						}
					}else{
						_LOG("invalid orig_name [%s]", orig_name.c_str());
					}
				}
			}
		}
	}
	if (PyObject* item = PyDict_GetItemString(inDict, "obj_desc")){ // 为目标添加文字描述
		// 格式： <obj-id> <str>[;<obj-id> <str>[...]]
		outConfig.obj_desc = ToString(item);
		auto ds = Str::split(outConfig.obj_desc, ";");
		auto &dd = outConfig._objDesc;
		dd.clear();
		for(auto &s: ds){
			auto p = s.find(' ');
			if(p < 0) continue;
			ObjDesc od;
			od.obj_id = std::stod(s.substr(0, p));
			od.desc = s.substr(p+1);
			dd.push_back(od);
		}
	}
}

void DearPyGui::set_required_configuration(PyObject* inDict, mvObstacleSeriesConfig& outConfig)
{
	if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(mvAppItemType::mvObstacleSeries)], inDict))
		return;
	outConfig.setValue(PyTuple_GetItem(inDict, 0));
}

void DearPyGui::fill_configuration_dict(const mvObstacleSeriesConfig& inConfig, PyObject* outDict)
{
	if (outDict == nullptr)
		return;
	PyDict_SetItemString(outDict, "tooltip",       mvPyObject(ToPyBool(inConfig.tooltip)));
	PyDict_SetItemString(outDict, "fill",          mvPyObject(ToPyBool(inConfig.fill)));
	PyDict_SetItemString(outDict, "show_id",       mvPyObject(ToPyBool(inConfig.show_id)));
	PyDict_SetItemString(outDict, "show_angle",    mvPyObject(ToPyBool(inConfig.show_angle)));
	PyDict_SetItemString(outDict, "show_size",     mvPyObject(ToPyBool(inConfig.show_size)));
	PyDict_SetItemString(outDict, "show_CIPV",     mvPyObject(ToPyBool(inConfig.show_CIPV)));
	PyDict_SetItemString(outDict, "show_move_state",    mvPyObject(ToPyBool(inConfig.show_move_state)));
	
	PyDict_SetItemString(outDict, "len",    mvPyObject(ToPyFloat(inConfig.len)));
	PyDict_SetItemString(outDict, "width",    mvPyObject(ToPyFloat(inConfig.width)));
	
	PyDict_SetItemString(outDict, "shape",         mvPyObject(ToPyInt(inConfig.shape)));
	PyDict_SetItemString(outDict, "selected_id",   mvPyObject(ToPyInt(inConfig.selected_id)));
	PyDict_SetItemString(outDict, "fmt",           mvPyObject(ToPyString(inConfig.fmt)));
	PyDict_SetItemString(outDict, "obj_desc",           mvPyObject(ToPyString(inConfig.obj_desc)));
	PyDict_SetItemString(outDict, "table",    inConfig.relTbl == nullptr ? mvPyObject(ToPyInt(0)) : mvPyObject(ToPyInt(inConfig.relTbl->uuid)));
	if( inConfig._stru_sz > 0){
		// 生成 table header
		std::ostringstream os{};
		auto &config = inConfig;
		if(inConfig.flds.size() == 0){ // 没有设置显示字段， 就不出表头
			// for(int j=0; j<config._ff.size(); j++){
			// 	auto f = &config._ff[j];
			// 	if(f->dot_n > 0)
			// 		os << config._fn[j] << '\t';
			// }
		}else{ // 设置了表头
			for(auto &fd : config.flds){
				if(fd.mapped_name.length() == 0)
					continue; // 不显示此字段
				if(fd.unit_name.length() > 0)
					os << fd.mapped_name << '(' << fd.unit_name << ')' << '\t';
				else
					os << fd.mapped_name << '\t';
			}
		}
		PyDict_SetItemString(outDict, "table_header", mvPyObject(ToPyString(os.str())));
	}else
		PyDict_SetItemString(outDict, "table_header",  mvPyObject(ToPyString("")));

}


void mvObstacleSeries::defineArgs(std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup)
{
	args.push_back({ mvPyDataType::Any,       "data" });
	args.push_back({ mvPyDataType::Bool,      "fill",    mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool,      "tooltip", mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool,      "show_id",    mvArgType::KEYWORD_ARG, "True" });
	args.push_back({ mvPyDataType::Bool,      "show_angle",    mvArgType::KEYWORD_ARG, "False" });
	args.push_back({ mvPyDataType::Bool,      "show_size",    mvArgType::KEYWORD_ARG, "False" });
	args.push_back({ mvPyDataType::Bool,      "show_CIPV",    mvArgType::KEYWORD_ARG, "False" });
	args.push_back({ mvPyDataType::Bool,      "show_move_state",    mvArgType::KEYWORD_ARG, "False" });
	args.push_back({ mvPyDataType::Integer,   "shape",    mvArgType::KEYWORD_ARG, "2" });
	args.push_back({ mvPyDataType::Float,     "len",    mvArgType::KEYWORD_ARG, "2.5" });
	args.push_back({ mvPyDataType::Float,     "width",    mvArgType::KEYWORD_ARG, "0.75" });

	args.push_back({ mvPyDataType::Any,       "fmt",    mvArgType::KEYWORD_ARG, "" });
	args.push_back({ mvPyDataType::String,    "fld_desc",    mvArgType::KEYWORD_ARG, "" });
	args.push_back({ mvPyDataType::String,    "obj_desc",    mvArgType::KEYWORD_ARG, "" });

	args.push_back({ mvPyDataType::Integer,   "table",    mvArgType::KEYWORD_ARG, "" });

	setup.about = "Adds a obstacle series to a plot.";
	setup.category = { "Plotting", "Containers", "Widgets" };
}


/////////////////////////////////


static void PlotMyShapes(const char* label_id, mvAppItem& item, mvObstacleSeriesConfig &config)
{
	auto &val = *config.value;
	char *ptr = (char *)val.data();
	BaseOB *obj;
	if(config._stru_sz == 0){
		//printf("No valid fmt\n");
		return; // 没有成功解析fmt, 不绘制
	}
	auto data_sz = val.size();
	if(data_sz % (config._stru_sz + sizeof(BaseOB)) > 0){
		printf("add_obstacle_series: invalid data size,  %d %% (24 + %d) > 0\n", data_sz, config._stru_sz);
		return; // 数据可能不匹配, 
	}
	int count = data_sz / (sizeof(BaseOB) + config._stru_sz);
	char id_str[32];

	config.setType(item.info.internalLabel.c_str());
	ImDrawList* draw_list = ImPlot::GetPlotDrawList();

	ImPlotPoint mp = ImPlot::GetPlotMousePos(); // mouse pos
	bool found_id = false; // 列表中是否有找到选中的id， 没有的话是之前选中的， 需要清除掉
	bool show_detail = false;

	// begin plot item
	if (ImPlot::BeginItem(label_id, ImPlotCol_Line)) {
		auto &col = ImPlot::GetCurrentItem()->Color;

		float w2 = config.width; // half width0.75
		float l2 = config.len; // length  2.5
		float font_size = 0.0f;
		ImU32 colTxt = ImPlot::GetStyleColorU32(ImPlotCol_InlayText);
		config.hovered_id = -1;
		for (int i = 0; i < count; i++, ptr+=sizeof(BaseOB) + config._stru_sz) {
			obj = (BaseOB *)ptr;
			//obj->angle = 30.f;
			if(config.show_size){ // 使用目标提供的长宽
				w2 = obj->width / 2.0f;
				l2 = obj->length;
			}

			if(config.shape == 1) {// triangle (三角形不处理旋转角度)
				ImVec2 p1 = ImPlot::PlotToPixels(obj->lat-w2, obj->lgt);  // left-bottom
				ImVec2 p2 = ImPlot::PlotToPixels(obj->lat+w2, obj->lgt); // right-bottom
				ImVec2 p3 = ImPlot::PlotToPixels(obj->lat, obj->lgt+l2); // top-center
				if(config.fill | obj->flag & 2)
					draw_list->AddTriangleFilled(p1, p2, p3, col);
				else
					draw_list->AddTriangle(p1, p2, p3, col, 2.f);
				if(config.selected_id == obj->id){ // 选中的目标， 做一个标记
					draw_list->AddCircleFilled(ImPlot::PlotToPixels(obj->lat, obj->lgt), (p3.x-p1.x)/2.0f, IM_COL32(255,0,0 , 128), 6);
					found_id = true;
				}
			

			 }  
			 else if(config.shape == 2) { // rectangle						
				if(!config.show_angle){ // 不旋转角度
					ImVec2 p1 = ImPlot::PlotToPixels(obj->lat-w2, obj->lgt+l2);
					ImVec2 p2 = ImPlot::PlotToPixels(obj->lat+w2, obj->lgt);
					if(!config.fill){
						draw_list->AddRect(p1, p2, col);
					}				
					else{
						draw_list->AddRectFilled(p1, p2, col, 1.f);				
					}					
						
				}else{
					//  旋转角度的矩形
					float theta = (- obj->angle) / 180.f * 3.14159; // 旋转角度的表达是 clockwise +, 但计算是 clockwise -
					float x0, y0;
					_rotate(-w2, 0, theta, x0, y0);
					ImVec2 pt_bl = ImPlot::PlotToPixels(obj->lat+x0, obj->lgt+y0);
					_rotate(-w2, l2, theta, x0, y0);
					ImVec2 pt_tl = ImPlot::PlotToPixels(obj->lat+x0, obj->lgt+y0);
					_rotate(w2, l2, theta, x0, y0);
					ImVec2 pt_tr = ImPlot::PlotToPixels(obj->lat+x0, obj->lgt+y0);
					_rotate(w2, 0, theta, x0, y0);
					ImVec2 pt_br = ImPlot::PlotToPixels(obj->lat+x0, obj->lgt+y0);

					draw_list->PathLineTo(pt_bl);
					draw_list->PathLineTo(pt_tl);
					draw_list->PathLineTo(pt_tr);
					draw_list->PathLineTo(pt_br);
					// draw_list->PathLineTo(pt_bl);
					if(config.fill)
						draw_list->PathFillConvex(col);
					else
						draw_list->PathStroke(col, ImDrawFlags_Closed, 2.f);
				}
				if(config.selected_id == obj->id){ // 选中的目标， 做一个标记
					auto p1 = ImPlot::PlotToPixels(obj->lat, obj->lgt);
					auto p3 = ImPlot::PlotToPixels(obj->lat+w2, obj->lgt);
					draw_list->AddCircleFilled(p1, (p3.x-p1.x)/2.0f, IM_COL32(255,0,0 , 128), 6);
					found_id = true;
				}						
			}

			// draw ID str
			if(config.show_id){
				ImVec2 p3 = ImPlot::PlotToPixels(obj->lat, obj->lgt);
				sprintf(id_str, "%d", (int)obj->id);
				
				draw_list->AddText(p3, colTxt, id_str);

			}
			// draw obj desc
			for(auto &od: config._objDesc){
				if(od.obj_id == (int)obj->id){
					ImVec2 p4 = ImPlot::PlotToPixels(obj->lat+1.4, obj->lgt+2.1);
					draw_list->AddText(p4, IM_COL32(255, 255, 0, 128), od.desc.c_str());
					break;
				}
			}

			// draw tooltip
			if(config.tooltip && ImPlot::IsPlotHovered()
				&& mp.x >= obj->lat - w2 && mp.x <= obj->lat + w2
				&& mp.y <= obj->lgt + l2 && mp.y >= obj->lgt && !show_detail)
			{
				show_detail = true;
				if(ImGui::IsMouseClicked(0)){ // (0=left, 1=right, 2=middle)
					//printf("----clicked on id: %u\n", obj->id);
					if(config.selected_id == obj->id)
						config.selected_id = 0;
					else{
						config.selected_id = obj->id;
						found_id = true;
					}
				}
				config.hovered_id = obj->id;
				// 绘制竖直贯穿的光标
				float half_width = 0.33; 
				float  tool_l = ImPlot::PlotToPixels(obj->lat - half_width * 1.1, 0).x;
				float  tool_r = ImPlot::PlotToPixels(obj->lat + half_width * 1.2, 0).x;
				float  tool_t = ImPlot::GetPlotPos().y;
				float  tool_b = tool_t + ImPlot::GetPlotSize().y;
				ImPlot::PushPlotClipRect();
				draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 255, 64));
				ImPlot::PopPlotClipRect();

				// 绘制横向贯穿的光标
				tool_l = ImPlot::GetPlotPos().x;
				tool_r = tool_l + ImPlot::GetPlotSize().x;
				tool_t = ImPlot::PlotToPixels(0, obj->lgt - half_width * 1.1).y;
				tool_b = ImPlot::PlotToPixels(0, obj->lgt + half_width * 1.1).y;
				ImPlot::PushPlotClipRect();
				draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 255, 64));
				ImPlot::PopPlotClipRect();

				// 添加tooltip
				ImGui::BeginTooltip();
				char *stru_ptr = ptr + sizeof(BaseOB);
				// for(int j=0; j<config._ff.size(); j++){
				// 	auto f = &config._ff[j];
				// 	if(f->dot_n > 0){
				// 		// ImGui::Text("%s  %s %c", config._fn[j].c_str(), f->unpack(stru_ptr+f->offset, f->dot_n), f->format);
				// 		ImGui::Text("%s  %s --", config._fn[j].c_str(), f->unpack(stru_ptr+f->offset, f->dot_n));
				// 	}
				// }
				if(config.flds.size() > 0){ // 设置了字段名称， 则按设定的显示
					for(auto &fd : config.flds){
						auto f = &config._ff[fd.fld_idx];
						if(fd.enum_names.size() > 0){ // 枚举类型展开
							int ival = stru_to_int(f, stru_ptr+f->offset);
							//_LOG(" enum: %s  [%s] %d %c", config._fn[fd.fld_idx].c_str(), f->unpack(stru_ptr+f->offset, f->dot_n), ival, f->format);
							if(fd.enum_names.find(ival) == fd.enum_names.end()){
								ImGui::Text("%s  %d(Unknown)", fd.mapped_name.c_str(), ival);
							}else{
								ImGui::Text("%s  %d(%s)", fd.mapped_name.c_str(), ival, fd.enum_names[ival].c_str());
							}
						}else{
							ImGui::Text("%s  %s %s", fd.mapped_name.c_str(), f->unpack(stru_ptr+f->offset, f->dot_n), fd.unit_name.c_str());
						}
					}
				}else{ // 否则显示原始内容
					for(int j=0; j<config._ff.size(); j++){
						auto f = &config._ff[j];
						if(f->dot_n > 0)
							ImGui::Text("%s  %s ", config._fn[j].c_str(), f->unpack(stru_ptr+f->offset, f->dot_n));
							//ImGui::Text("%s  %s %c", config._fn[j].c_str(), f->unpack(stru_ptr+f->offset, f->dot_n), f->format);
					}
				}
				ImGui::EndTooltip();
			}

		}
		if(!found_id)
			config.selected_id = 0; // 清理掉无效的选中id	

		//筛选目标
		char *ptr2 = (char *)val.data();
		for (int i = 0; i < count; i++, ptr2+=sizeof(BaseOB) + config._stru_sz) {
			obj = (BaseOB *)ptr2;
			
			auto col3 = IM_COL32(255,0,0,255);
			ImVec2 p111,p222,p333;
			uint8_t filter_flg = obj->reserve;
			
			if(filter_flg==1|| filter_flg == 2 || filter_flg == 3){  //前三角：fl,front, fr
				// ImVec2 p111,p222,p333;
				p111 = ImPlot::PlotToPixels(obj->lat-w2+0.15, obj->lgt+0.15);  // left-bottom
				p222 = ImPlot::PlotToPixels(obj->lat+w2-0.15, obj->lgt+0.15); // right-bottom
				p333 = ImPlot::PlotToPixels(obj->lat, obj->lgt+1.75); // top-center  1.75
				draw_list->AddTriangleFilled(p111, p222, p333, col3);
			}
			
			else if (filter_flg == 4){ // 左三角
				p111 = ImPlot::PlotToPixels(obj->lat+0.5, obj->lgt+0.15);  // bottom
				p222 = ImPlot::PlotToPixels(obj->lat+0.5, obj->lgt+2); // top
				p333 = ImPlot::PlotToPixels(obj->lat-0.5, obj->lgt+1.25); // left-center lat-0.45
				draw_list->AddTriangleFilled(p111, p222, p333, col3);
			}
			else if (filter_flg == 5) {  // 右三角
				p111 = ImPlot::PlotToPixels(obj->lat-0.5, obj->lgt+0.15);  // bottom
				p222 = ImPlot::PlotToPixels(obj->lat-0.5, obj->lgt+2); // top
				p333 = ImPlot::PlotToPixels(obj->lat+0.5, obj->lgt+1.25); // right-center  +0.45	
				draw_list->AddTriangleFilled(p111, p222, p333, col3);			
			}
			else if (filter_flg == 6 || filter_flg == 7 || filter_flg == 8) {  //倒三角：rl,rear, rr
				p111 = ImPlot::PlotToPixels(obj->lat-w2+0.15, obj->lgt+1.75);  // left-top
				p222 = ImPlot::PlotToPixels(obj->lat+w2-0.15, obj->lgt+1.75); // right-top
				p333 = ImPlot::PlotToPixels(obj->lat, obj->lgt+0.15); // bottom-center lgt+0.3
				draw_list->AddTriangleFilled(p111, p222, p333, col3);
			}
				
		}

		// end plot item
		ImPlot::EndItem();
	}
}


void  mvObstacleSeries::draw(ImDrawList* drawlist, float x, float y)  {
	mvAppItem& item = *this;
	mvObstacleSeriesConfig& config = configData;

	if (!item.config.show)
		return;

	if (item.font)
	{
		ImFont* fontptr = static_cast<mvFont*>(item.font.get())->getFontPtr();
		ImGui::PushFont(fontptr);
	}
	apply_local_theming(&item);
	{

		PlotMyShapes(item.info.internalLabel.c_str(), *this, config);

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

/**
 * 在 set_value 的时候， 如果有设置表格， 就更新表格单元数据
 */
void mvObstacleSeriesConfig::setTblCells(){
	auto &conf = *this;
	auto tbl = conf.relTbl;

	if(conf.flds.size() == 0 || tbl == nullptr){
		return;
	}else{
		std::lock_guard<std::recursive_mutex> lk(GContext->mutex);
		auto &val = *conf.value;
		char *ptr = (char *)val.data();
		char *stru_ptr;
		auto data_sz = val.size();
		int rownum = 0;
		int count = data_sz / (sizeof(BaseOB) + conf._stru_sz);

		for (auto& childitem : tbl->childslots[1]) // loop row
		{
			if (childitem == nullptr) continue;
			if(rownum >= count){
				// 清空cell
				for(auto& col: childitem->childslots[1]){ // loop cell
					if(col == nullptr) continue;
					if(col->type == mvAppItemType::mvText){
						col->setPyValue(ToPyString(""));
					}
				}
			}else{
				stru_ptr = ptr + (sizeof(BaseOB) + conf._stru_sz) * rownum + sizeof(BaseOB);
				rownum += 1;
				if (childitem->type == mvAppItemType::mvTableRow){
					int cellidx = 0;
					for(auto& col: childitem->childslots[1]){ // loop cell
						if(col == nullptr) continue;
						
						if(col->type == mvAppItemType::mvText){
							auto &fd = conf.flds[cellidx++];
							auto f = &conf._ff[fd.fld_idx];
							
							if(fd.enum_names.size() > 0){ // 枚举类型展开
								int ival = stru_to_int(f, stru_ptr+f->offset);
								char eval[64];
								if(fd.enum_names.find(ival) == fd.enum_names.end()){
									snprintf(eval, sizeof(eval), "%d(Unknown)", ival);
								}else{
									snprintf(eval, sizeof(eval), "%d(%s)", ival, fd.enum_names[ival] );
								}
								col->setPyValue(ToPyString(eval));
							}else{
								col->setPyValue(ToPyString(f->unpack(stru_ptr+f->offset, f->dot_n) ) );
							}
							
							if(cellidx >= conf.flds.size())
								break;
						}
					}
				}
			}
		}
	}
}


/*
改为获取目标的字符串列表，  格式如下：
第一行标题
后边每行一个目标

如果设置了字段显示格式， 则使用设置的内容
*/
PyObject* mvObstacleSeries::getPyValue() {
	auto &config = configData;
	auto &val = *configData.value;
	char *ptr = (char *)val.data();
	BaseOB *obj;

	// if(configData.relTbl != nullptr){
	// 	return setTblCells(configData);
	// }

	std::ostringstream os;
	if(config._stru_sz == 0){
		_LOG("not fmt, not str output");
		return ToPyString(os.str()); // 没有成功解析fmt, 不处理
	}
	auto data_sz = val.size();
	if(data_sz % (config._stru_sz + sizeof(BaseOB)) > 0){
		_LOG("invalid data size,  %d %% (24 + %d) > 0", data_sz, config._stru_sz);
		return ToPyString(os.str()); // 数据可能不匹配, 
	}
	int count = data_sz / (sizeof(BaseOB) + config._stru_sz);

	if(config.flds.size() > 0){
		// out header
		for(auto &fd : config.flds){
			if(fd.mapped_name.length() == 0)
				continue; // 不显示此字段
			if(fd.unit_name.length() > 0)
				os << fd.mapped_name << '(' << fd.unit_name << ')' << '\t';
			else
				os << fd.mapped_name << '\t';
		}
		os << '\n' << "----------\n";

		// out items
		for (int i = 0; i < count; i++, ptr+=sizeof(BaseOB) + config._stru_sz) {
			char *stru_ptr = ptr + sizeof(BaseOB);
			for(auto &fd : config.flds){
				auto f = &config._ff[fd.fld_idx];
				if(fd.enum_names.size() > 0){ // 枚举类型展开
					int ival = stru_to_int(f, stru_ptr+f->offset);
					if(fd.enum_names.find(ival) == fd.enum_names.end()){
						os << ival << "(Unknown)\t";
					}else{
						os << ival << '(' << fd.enum_names[ival] << ')' << '\t';
					}
				}else{
					os << f->unpack(stru_ptr+f->offset, f->dot_n) << '\t';
				}
			}
			os << '\n';
		}
	}else{
		// out header
		for(int j=0; j<config._ff.size(); j++){
			auto f = &config._ff[j];
			if(f->dot_n > 0)
				os << config._fn[j] << '\t';
		}
		os << '\n' << "----------\n";

		// out items
		for (int i = 0; i < count; i++, ptr+=sizeof(BaseOB) + config._stru_sz) {
			char *stru_ptr = ptr + sizeof(BaseOB);
			for(int j=0; j<config._ff.size(); j++){
				auto f = &config._ff[j];
				if(f->dot_n > 0)
					os << f->unpack(stru_ptr+f->offset, f->dot_n) << '\t';
			}
			os << '\n';
		}
	}

	return ToPyString(os.str());
}

