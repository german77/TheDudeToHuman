// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <format>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_json.h"

namespace Database {
static std::string serializeToolData(DudeDatabase* db) {
    std::vector<Database::ToolData> tool_data = db->GetToolData();
    std::string json = "";

    for (const ToolData& data : tool_data) {
        json += std::format("{{{}}},", data.serializeJson2());
    }
    if (!tool_data.empty()) {
        json.pop_back();
    }

    return json;
}

static std::string serializeFileData(DudeDatabase* db) {
    std::vector<Database::FileData> file_data = db->GetFileData();
    std::string json = "";

    for (const FileData& data : file_data) {
        json += std::format("{{{}}},", data.serializeJson2());
    }
    if (!file_data.empty()) {
        json.pop_back();
    }

    return json;
}

static std::string serializeNotesData(DudeDatabase* db) {
    std::vector<Database::NotesData> notes_data = db->GetNotesData();
    std::string json = "";

    for (const NotesData& data : notes_data) {
        json += std::format("{{{}}},", data.serializeJson2());
    }
    if (!notes_data.empty()) {
        json.pop_back();
    }

    return json;
}

static std::string serializeDeviceTypeData(DudeDatabase* db) {
    std::vector<Database::DeviceTypeData> device_data = db->GetDeviceTypeData();
    std::string json = "";

    for (const DeviceTypeData& data : device_data) {
        json += std::format("{{{}}},", data.serializeJson2());
    }
    if (!device_data.empty()) {
        json.pop_back();
    }

    return json;
}

std::string serializeDatabaseJson(DudeDatabase* db) {
    std::string tool_data = std::format("\"toolData\": [{}]", serializeToolData(db));
    std::string file_data = std::format("\"fileData\": [{}]", serializeFileData(db));
    std::string notes_data = std::format("\"notesData\": [{}]", serializeNotesData(db));
    std::string device_type_data =
        std::format("\"deviceTypeData\": [{}]", serializeDeviceTypeData(db));
    return std::format("{{\n{},\n {},\n {},\n {}\n}}\n", tool_data, file_data, notes_data,
                       device_type_data);
}

} // namespace Database
