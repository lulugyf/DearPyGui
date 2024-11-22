#ifndef __EXT_H__
#define __EXT_H__

#include "../src/mvAppItem.h"

namespace ext {

    int  GetEntityDesciptionFlags(mvAppItemType type);
    StorageValueTypes GetEntityValueType(mvAppItemType type);
    const std::vector<std::pair<std::string, i32>>& GetAllowableParents(mvAppItemType type);
    void GetEntityParser(mvAppItemType type, std::vector<mvPythonDataElement> &args, mvPythonParserSetup &setup) ;
    constexpr const char* GetEntityCommand(mvAppItemType type);
}



#include "ext_objseries.h"
#include "ext_dotseries.h"
#include "ext_laneseries.h"
#include "ext_timeseries.h"
#include "ext_spanseries.h"
#include "ext_objstruseries.h"
#include "ext_rectseries.h"
#include "ext_obstacleseries.h"
#include "ext_obstacleboxseries.h"
#include "ext_objyawseries.h"
#include "ext_lights.h"
#include "ext_quardseries.h"



#endif
