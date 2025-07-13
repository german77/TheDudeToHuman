// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <vector>

#include "common/common_types.h"
#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_types.h"
#include "the_dude_to_human/database/dude_validator.h"

namespace Database {

static bool CheckNewDataFormats(DudeDatabase* db) {
    bool new_format_exist = false;

    for (DataFormat format : db->ListUsedDataFormats()) {
        switch (format) {
        case DataFormat::ServerConfig:
        case DataFormat::Tool:
        case DataFormat::File:
        case DataFormat::Notes:
        case DataFormat::Map:
        case DataFormat::Probe:
        case DataFormat::DeviceType:
        case DataFormat::Device:
        case DataFormat::Network:
        case DataFormat::Service:
        case DataFormat::Notification:
        case DataFormat::Link:
        case DataFormat::LinkType:
        case DataFormat::DataSource:
        case DataFormat::ObjectList:
        case DataFormat::DeviceGroup:
        case DataFormat::Function:
        case DataFormat::SnmpProfile:
        case DataFormat::Panel:
        case DataFormat::SysLogRule:
        case DataFormat::NetworkMapElement:
        case DataFormat::ChartLine:
        case DataFormat::PanelElement:
            continue;
        default:
            new_format_exist = true;
            printf("Unsupported data format %d\n", static_cast<u32>(format));
        }
    }
    return new_format_exist;
}

int ValidateDatabase(DudeDatabase* db) {
    if (CheckNewDataFormats(db)) {
        printf(
            "This database contains new data formats. Please contact developer to add support.\n");
        return 1;
    }

    printf("Database health: OK\n");
    return 0;
}

} // namespace Database
