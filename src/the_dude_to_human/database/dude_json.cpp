// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <format>
#include <fstream>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_json.h"

namespace Database {

template <typename T>
static std::string SerializeData(std::vector<T> obj) {
    std::string json = "";

    for (const DudeObj& data : obj) {
        json += std::format("{{{}}},", data.SerializeJson());
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
    jsonFile << std::format("\"serverConfig\": [{}],\n", SerializeData(db->GetServerConfigData()));
    jsonFile << std::format("\"tool\": [{}],\n", SerializeData(db->GetToolData()));
    jsonFile << std::format("\"file\": [{}],\n", SerializeData(db->GetFileData()));
    jsonFile << std::format("\"notes\": [{}],\n", SerializeData(db->GetNotesData()));
    jsonFile << std::format("\"Map\": [{}],\n", SerializeData(db->GetMapData()));
    //jsonFile << std::format("\"Probe\": [{}],\n", SerializeData(db->GetProbeData()));
    jsonFile << std::format("\"deviceType\": [{}],\n", SerializeData(db->GetDeviceTypeData()));
    jsonFile << std::format("\"Device\": [{}],\n", SerializeData(db->GetDeviceData()));
    jsonFile << std::format("\"Network\": [{}],\n", SerializeData(db->GetNetworkData()));
    jsonFile << std::format("\"Service\": [{}],\n", SerializeData(db->GetServiceData()));
    jsonFile << std::format("\"Notification\": [{}],\n", SerializeData(db->GetNotificationData()));
    jsonFile << std::format("\"Link\": [{}],\n", SerializeData(db->GetLinkData()));
    jsonFile << std::format("\"LinkType\": [{}],\n", SerializeData(db->GetLinkTypeData()));
    jsonFile << std::format("\"DataSource\": [{}],\n", SerializeData(db->GetDataSourceData()));
    jsonFile << std::format("\"ObjectList\": [{}],\n", SerializeData(db->GetObjectListData()));
    jsonFile << std::format("\"DeviceGroup\": [{}],\n", SerializeData(db->GetDeviceGroupData()));
    jsonFile << std::format("\"Function\": [{}],\n", SerializeData(db->GetFunctionData()));
    jsonFile << std::format("\"SnmpProfile\": [{}],\n", SerializeData(db->GetSnmpProfileData()));
    jsonFile << std::format("\"Panel\": [{}],\n", SerializeData(db->GetPanelData()));
    jsonFile << std::format("\"SysLogRule\": [{}],\n", SerializeData(db->GetSysLogRuleData()));
    jsonFile << std::format("\"NetworkMapElement\": [{}],\n",
                            SerializeData(db->GetNetworkMapElementData()));
    jsonFile << std::format("\"ChartLine\": [{}],\n", SerializeData(db->GetChartLineData()));
    jsonFile << std::format("\"PanelElement\": [{}]\n", SerializeData(db->GetPanelElementData()));
    jsonFile << "}";

    jsonFile.close();
    return 0;
}

} // namespace Database
