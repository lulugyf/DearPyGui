
#include "ext.h"



// #include <iconv.h> //for gbk/big5/utf8
// // https://github.com/lytsing/gbk-utf8/blob/master/utf8.c


// inline int code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen,char *outbuf, size_t outlen)
// {
//     iconv_t cd;
//     int rc;
//     char **pin = &inbuf;
//     char **pout = &outbuf;
//     cd = iconv_open(to_charset, from_charset);
//     if (cd==0)
//         return -1;
//     memset(outbuf,0,outlen);
//     if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
//         return -1;
//     iconv_close(cd);
//     return outlen;
// }

// inline 
// std::string __code(const char *from_c, const char *to_c, const std::string &in)
// {
//     size_t outlen = strlen(in.c_str())*3;
//     std::vector<char> f(outlen);
//     int ret = code_convert(from_c, to_c, (char *)in.c_str(), (size_t)outlen/3, f.data(), outlen);
//     if(ret > 0){
//         std::string out(f.data(), strlen(f.data()));
//         return out;
//     }else
//         return std::string("");

// }

// std::string utf2gb(const std::string &in){
// 	return __code("UTF-8", "GB18030", in);
// }

// std::string gb2utf(const std::string &in){
// 	return __code("GB18030", "UTF-8", in);
// }


//////////////////////////////////////////////

void DearPyGui::set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<char>>& outValue) // azj
{
	if (dataSource == item.config.source) return;
	item.config.source = dataSource;

	mvAppItem* srcItem = GetItem((*GContext->itemRegistry), dataSource);
	if (!srcItem)
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
			"Source item not found: " + std::to_string(dataSource), &item);
		return;
	}
	if (DearPyGui::GetEntityValueType(srcItem->type) != DearPyGui::GetEntityValueType(item.type))
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
			"Values types do not match: " + std::to_string(dataSource), &item);
		return;
	}
	outValue = *static_cast<std::shared_ptr<std::vector<char>>*>(srcItem->getValue());
}

void DearPyGui::set_data_source(mvAppItem& item, mvUUID dataSource, std::shared_ptr<std::vector<int>>& outValue) // azj
{
	if (dataSource == item.config.source) return;
	item.config.source = dataSource;

	mvAppItem* srcItem = GetItem((*GContext->itemRegistry), dataSource);
	if (!srcItem)
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotFound, "set_value",
			"Source item not found: " + std::to_string(dataSource), &item);
		return;
	}
	if (DearPyGui::GetEntityValueType(srcItem->type) != DearPyGui::GetEntityValueType(item.type))
	{
		mvThrowPythonError(mvErrorCode::mvSourceNotCompatible, "set_value",
			"Values types do not match: " + std::to_string(dataSource), &item);
		return;
	}
	outValue = *static_cast<std::shared_ptr<std::vector<int>>*>(srcItem->getValue());
}

int ext::GetEntityDesciptionFlags(mvAppItemType type){
    switch(type){
        case mvAppItemType::mvObjSeries:
        case mvAppItemType::mvDotSeries:
        case mvAppItemType::mvLaneSeries:
        case mvAppItemType::mvTimeSeries:
        case mvAppItemType::mvSpanSeries:
        case mvAppItemType::mvObjstruSeries:
        case mvAppItemType::mvRectSeries:
        case mvAppItemType::mvObstacleSeries:
        case mvAppItemType::mvObstacleBoxSeries:
        case mvAppItemType::mvObjYawSeries:
        case mvAppItemType::mvLights:
        case mvAppItemType::mvQuardSeries:

            return MV_ITEM_DESC_CONTAINER;
        default: return MV_ITEM_DESC_DEFAULT;
    }
    
}

StorageValueTypes ext::GetEntityValueType(mvAppItemType type) {
    switch(type){
        case mvAppItemType::mvObjSeries:
        case mvAppItemType::mvDotSeries:
        case mvAppItemType::mvLaneSeries:
        case mvAppItemType::mvTimeSeries:
        case mvAppItemType::mvSpanSeries:
        case mvAppItemType::mvObjstruSeries:
        case mvAppItemType::mvRectSeries:
        case mvAppItemType::mvObstacleSeries:
        case mvAppItemType::mvObstacleBoxSeries:
        case mvAppItemType::mvObjYawSeries:
        case mvAppItemType::mvLights:
        case mvAppItemType::mvQuardSeries:   
            return StorageValueTypes::Series;
        default: return StorageValueTypes::None;
    }
}

const std::vector<std::pair<std::string, i32>>& ext::GetAllowableParents(mvAppItemType type){
    #define MV_ADD_PARENT(x){#x, (int)x}
    #define MV_START_PARENTS {static std::vector<std::pair<std::string, i32>> parents = {
    #define MV_END_PARENTS };return parents;}

    switch(type){
    case mvAppItemType::mvObjSeries:
    case mvAppItemType::mvDotSeries:
    case mvAppItemType::mvLaneSeries:
    case mvAppItemType::mvTimeSeries:
    case mvAppItemType::mvSpanSeries:
    case mvAppItemType::mvObjstruSeries:
    case mvAppItemType::mvRectSeries:
    case mvAppItemType::mvObstacleSeries:
    case mvAppItemType::mvObstacleBoxSeries:
    case mvAppItemType::mvObjYawSeries:
    case mvAppItemType::mvLights:
    case mvAppItemType::mvQuardSeries:

       MV_START_PARENTS
        MV_ADD_PARENT(mvAppItemType::mvPlotAxis),
        MV_ADD_PARENT(mvAppItemType::mvTemplateRegistry),
        MV_END_PARENTS
    default:
    {
        static std::vector<std::pair<std::string, i32>> parents = { {"All", 0} };
        return parents;
    }
    }
    #undef MV_ADD_PARENT
    #undef MV_START_PARENTS
    #undef MV_END_PARENTS
}

void ext::GetEntityParser(mvAppItemType type, std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup) {
    switch(type){
    case mvAppItemType::mvObjSeries:                
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvObjSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvDotSeries:                
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvDotSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvLaneSeries:                
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvLaneSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvTimeSeries:               
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvTimeSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvSpanSeries:               
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvSpanSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvObjstruSeries:
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvObjstruSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvRectSeries:
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvRectSeries::defineArgs(args, setup);
        break;
    }

    case mvAppItemType::mvObstacleSeries:
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvObstacleSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvObstacleBoxSeries:
    {
        AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvObstacleBoxSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvObjYawSeries:
    {
         AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvObjYawSeries::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvLights:
    {
         AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvLights::defineArgs(args, setup);
        break;
    }
    case mvAppItemType::mvQuardSeries:      
    {
         AddCommonArgs(args, (CommonParserArgs)(
            MV_PARSER_ARG_ID |
            MV_PARSER_ARG_PARENT |
            MV_PARSER_ARG_BEFORE |
            MV_PARSER_ARG_SOURCE |
            MV_PARSER_ARG_SHOW)
        );
        mvQuardSeries::defineArgs(args, setup);
        break;
    }

    }
}

constexpr const char*
ext::GetEntityCommand(mvAppItemType type) {
    switch(type){
        case mvAppItemType::mvObjSeries:                   return "add_obj_series";
        case mvAppItemType::mvDotSeries:                   return "add_dot_series";
        case mvAppItemType::mvLaneSeries:                  return "add_lane_series";
        case mvAppItemType::mvTimeSeries:                  return "add_time_series";
        case mvAppItemType::mvSpanSeries:                  return "add_span_series";
        case mvAppItemType::mvObjstruSeries:               return "add_objstru_series";
        case mvAppItemType::mvRectSeries:                  return "add_rect_series";
        case mvAppItemType::mvObstacleSeries:              return "add_obstacle_series";
        case mvAppItemType::mvObstacleBoxSeries:           return "add_obstaclebox_series";
        case mvAppItemType::mvLights:                      return "add_light_series";
        case mvAppItemType::mvQuardSeries:                 return "add_quard_series";
 
        default:
        {
            assert(false);
            return "no command";
        }
    }
}

namespace Str{
    using str = std::string;
    std::vector<str> split(const str &ss, str delimiter)
    {
        std::vector<str> ret;
        auto s = ss;

        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) // 逐行添加
        {
            token = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
            ret.push_back(token);
        }
        if (s.length() > 0)
            ret.push_back(s);
        return ret;
    }
}



namespace ImPlot
{
    IMPLOT_API void PlotText(const char* text, double x, double y, float font_size, bool vertical=false, const ImVec2& pix_offset=ImVec2(0,0));

    void PlotText(const char* text, double x, double y, float font_size, bool vertical, const ImVec2& pixel_offset) {
        IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != NULL, "PlotText() needs to be called between BeginPlot() and EndPlot()!");
        ImDrawList & DrawList = *GetPlotDrawList();
        PushPlotClipRect();
        ImU32 colTxt = GetStyleColorU32(ImPlotCol_InlayText);
        if (vertical) {
            ImVec2 ctr = CalcTextSizeVertical(text) * 0.5f;
            ImVec2 pos = PlotToPixels(ImPlotPoint(x,y)) + ImVec2(-ctr.x, ctr.y) + pixel_offset;
            AddTextVertical(&DrawList, pos, colTxt, text);
        }
        else {
            ImVec2 pos = PlotToPixels(ImPlotPoint(x,y)) - ImGui::CalcTextSize(text) * 0.5f + pixel_offset;
            DrawList.AddText(pos, colTxt, text);
        }
        PopPlotClipRect();
    }

}

namespace ext
{
    void AddQuadrilateral(ImDrawList *dl, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness)
    {
        if ((col & IM_COL32_A_MASK) == 0)
            return;

        dl->PathLineTo(p1);
        dl->PathLineTo(p2);
        dl->PathLineTo(p3);
        dl->PathLineTo(p4);
        dl->PathStroke(col, ImDrawFlags_Closed, thickness);
    }
}
