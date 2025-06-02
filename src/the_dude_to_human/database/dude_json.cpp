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
    jsonFile << SerializeTable("map", db->GetMapData(), has_credentials);
    jsonFile << SerializeTable("probe", db->GetProbeData(), has_credentials);
    jsonFile << SerializeTable("deviceType", db->GetDeviceTypeData(), has_credentials);
    jsonFile << SerializeTable("device", db->GetDeviceData(), has_credentials);
    jsonFile << SerializeTable("network", db->GetNetworkData(), has_credentials);
    jsonFile << SerializeTable("service", db->GetServiceData(), has_credentials);
    jsonFile << SerializeTable("notification", db->GetNotificationData(), has_credentials);
    jsonFile << SerializeTable("link", db->GetLinkData(), has_credentials);
    jsonFile << SerializeTable("linkType", db->GetLinkTypeData(), has_credentials);
    jsonFile << SerializeTable("dataSource", db->GetDataSourceData(), has_credentials);
    jsonFile << SerializeTable("objectList", db->GetObjectListData(), has_credentials);
    jsonFile << SerializeTable("deviceGroup", db->GetDeviceGroupData(), has_credentials);
    jsonFile << SerializeTable("function", db->GetFunctionData(), has_credentials);
    jsonFile << SerializeTable("snmpProfile", db->GetSnmpProfileData(), has_credentials);
    jsonFile << SerializeTable("panel", db->GetPanelData(), has_credentials);
    jsonFile << SerializeTable("sysLogRule", db->GetSysLogRuleData(), has_credentials);
    jsonFile << SerializeTable("networkMapElement", db->GetNetworkMapElementData(),
                               has_credentials);
    jsonFile << SerializeTable("chartLine", db->GetChartLineData(), has_credentials);
    jsonFile << SerializeTable("panelElement", db->GetPanelElementData(), has_credentials, false);
    jsonFile << "}";

    jsonFile.close();
    return 0;
}

} // namespace Database
