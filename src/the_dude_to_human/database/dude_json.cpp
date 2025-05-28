// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fstream>
#include <fmt/core.h>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_json.h"

namespace Database {

template <typename T>
static std::string SerializeData(std::vector<T> obj, bool has_credentials) {
    std::string json = "";

    for (const DudeObj& data : obj) {
        json += fmt::format("{{{}}},", data.SerializeJson(has_credentials));
    }
    if (!obj.empty()) {
        json.pop_back();
    }

    return json;
}

template <typename T>
static std::string SerializeTable(std::string table_name, std::vector<T> obj, bool has_credentials,
                                  bool has_coma = true) {
    return fmt::format("\"{}\": [{}]{}\n", table_name, SerializeData(obj, has_credentials),
                       has_coma ? "," : "");
}

int SerializeDatabaseJson(DudeDatabase* db, const std::string& db_file, bool has_credentials) {
    std::ofstream jsonFile(db_file);
    if (!jsonFile.is_open())
        return 1;

    jsonFile << "{\n";
    jsonFile << SerializeTable("serverConfig", db->GetServerConfigData(), has_credentials);
    jsonFile << SerializeTable("tool", db->GetToolData(), has_credentials);
    jsonFile << SerializeTable("file", db->GetFileData(), has_credentials);
    jsonFile << SerializeTable("notes", db->GetNotesData(), has_credentials);
    jsonFile << SerializeTable("Map", db->GetMapData(), has_credentials);
    jsonFile << SerializeTable("Probe", db->GetProbeData(), has_credentials);
    jsonFile << SerializeTable("deviceType", db->GetDeviceTypeData(), has_credentials);
    jsonFile << SerializeTable("Device", db->GetDeviceData(), has_credentials);
    jsonFile << SerializeTable("Network", db->GetNetworkData(), has_credentials);
    jsonFile << SerializeTable("Service", db->GetServiceData(), has_credentials);
    jsonFile << SerializeTable("Notification", db->GetNotificationData(), has_credentials);
    jsonFile << SerializeTable("Link", db->GetLinkData(), has_credentials);
    jsonFile << SerializeTable("LinkType", db->GetLinkTypeData(), has_credentials);
    jsonFile << SerializeTable("DataSource", db->GetDataSourceData(), has_credentials);
    jsonFile << SerializeTable("ObjectList", db->GetObjectListData(), has_credentials);
    jsonFile << SerializeTable("DeviceGroup", db->GetDeviceGroupData(), has_credentials);
    jsonFile << SerializeTable("Function", db->GetFunctionData(), has_credentials);
    jsonFile << SerializeTable("SnmpProfile", db->GetSnmpProfileData(), has_credentials);
    jsonFile << SerializeTable("Panel", db->GetPanelData(), has_credentials);
    jsonFile << SerializeTable("SysLogRule", db->GetSysLogRuleData(), has_credentials);
    jsonFile << SerializeTable("NetworkMapElement", db->GetNetworkMapElementData(),
                               has_credentials);
    jsonFile << SerializeTable("ChartLine", db->GetChartLineData(), has_credentials);
    jsonFile << SerializeTable("PanelElement", db->GetPanelElementData(), has_credentials, false);
    jsonFile << "}";

    jsonFile.close();
    return 0;
}

} // namespace Database
