// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fstream>
#include <fmt/core.h>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_json.h"

namespace Database {

template <typename T>
static std::string SerializeData(std::vector<T> obj) {
    std::string json = "";

    for (const DudeObj& data : obj) {
        json += fmt::format("{{{}}},", data.SerializeJson());
    }
    if (!obj.empty()) {
        json.pop_back();
    }

    return json;
}

int SerializeDatabaseJson(DudeDatabase* db, const std::string& db_file) {
    std::ofstream jsonFile(db_file);
    if (!jsonFile.is_open())
        return 1;

    jsonFile << "{\n";
    jsonFile << fmt::format("\"serverConfig\": [{}],\n", SerializeData(db->GetServerConfigData()));
    jsonFile << fmt::format("\"tool\": [{}],\n", SerializeData(db->GetToolData()));
    jsonFile << fmt::format("\"file\": [{}],\n", SerializeData(db->GetFileData()));
    jsonFile << fmt::format("\"notes\": [{}],\n", SerializeData(db->GetNotesData()));
    jsonFile << fmt::format("\"Map\": [{}],\n", SerializeData(db->GetMapData()));
    // jsonFile << fmt::format("\"Probe\": [{}],\n", SerializeData(db->GetProbeData()));
    jsonFile << fmt::format("\"deviceType\": [{}],\n", SerializeData(db->GetDeviceTypeData()));
    jsonFile << fmt::format("\"Device\": [{}],\n", SerializeData(db->GetDeviceData()));
    jsonFile << fmt::format("\"Network\": [{}],\n", SerializeData(db->GetNetworkData()));
    jsonFile << fmt::format("\"Service\": [{}],\n", SerializeData(db->GetServiceData()));
    jsonFile << fmt::format("\"Notification\": [{}],\n", SerializeData(db->GetNotificationData()));
    jsonFile << fmt::format("\"Link\": [{}],\n", SerializeData(db->GetLinkData()));
    jsonFile << fmt::format("\"LinkType\": [{}],\n", SerializeData(db->GetLinkTypeData()));
    jsonFile << fmt::format("\"DataSource\": [{}],\n", SerializeData(db->GetDataSourceData()));
    jsonFile << fmt::format("\"ObjectList\": [{}],\n", SerializeData(db->GetObjectListData()));
    jsonFile << fmt::format("\"DeviceGroup\": [{}],\n", SerializeData(db->GetDeviceGroupData()));
    jsonFile << fmt::format("\"Function\": [{}],\n", SerializeData(db->GetFunctionData()));
    jsonFile << fmt::format("\"SnmpProfile\": [{}],\n", SerializeData(db->GetSnmpProfileData()));
    jsonFile << fmt::format("\"Panel\": [{}],\n", SerializeData(db->GetPanelData()));
    jsonFile << fmt::format("\"SysLogRule\": [{}],\n", SerializeData(db->GetSysLogRuleData()));
    jsonFile << fmt::format("\"NetworkMapElement\": [{}],\n",
                            SerializeData(db->GetNetworkMapElementData()));
    jsonFile << fmt::format("\"ChartLine\": [{}],\n", SerializeData(db->GetChartLineData()));
    jsonFile << fmt::format("\"PanelElement\": [{}]\n", SerializeData(db->GetPanelElementData()));
    jsonFile << "}";

    jsonFile.close();
    return 0;
}

} // namespace Database
