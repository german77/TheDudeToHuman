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

static bool CheckDatabaseIds(DudeDatabase* db) {
    // Load objects that don't depend on others
    std::vector<SnmpProfileData> snmp_profiles = db->GetSnmpProfileData();
    std::vector<FunctionData> functions = db->GetFunctionData();
    std::vector<ObjectListData> object_lists = db->GetObjectListData();
    // Validate file tree
    std::vector<FileData> files = db->GetFileData();
    for (const FileData& file : files) {
        // Folder root
        if (file.parent_id == -1) {
            continue;
        }
        bool is_parent_found = false;
        for (const FileData& parentFile : files) {
            if (file.object_id == parentFile.object_id)
                continue;
            if (file.parent_id != parentFile.object_id)
                continue;
            is_parent_found = true;
            break;
        }
        if (!is_parent_found) {
            printf("File %d: Invalid file parent found %d in file %s\n", file.object_id.value,
                   file.parent_id.value, file.name.text.c_str());
        }
    }

    std::vector<NotificationData> notifications = db->GetNotificationData();
    for (const NotificationData& notification : notifications) {
        if (notification.sound_file_id != -1 && notification.sound_file_id != 0) {
            bool is_sound_found = false;
            for (const FileData& sound_file : files) {
                if (notification.sound_file_id != sound_file.object_id)
                    continue;
                is_sound_found = true;
                break;
            }
            if (!is_sound_found) {
                printf("Notification %d: Invalid sound file found %d in notification %s\n",
                       notification.object_id.value, notification.sound_file_id.value,
                       notification.name.text.c_str());
            }
        }
    }

    // Validate map tree
    std::vector<MapData> maps = db->GetMapData();
    for (const MapData& map : maps) {
        if (map.image_id != -1) {
            bool is_image_found = false;
            for (const FileData& image_file : files) {
                if (map.image_id != image_file.object_id)
                    continue;
                is_image_found = true;
                break;
            }
            if (!is_image_found) {
                printf("Map %d: Invalid image file found %d in map %s\n", map.object_id.value,
                       map.image_id.value, map.name.text.c_str());
            }
        }

        for (const u32& notify_id : map.notify_ids.data) {
            bool is_notify_id_found = false;
            if (notify_id != map.object_id) {
                for (const MapData& notify_map : maps) {
                    if (notify_id != notify_map.object_id)
                        continue;
                    is_notify_id_found = true;
                    break;
                }
                for (const NotificationData& notification : notifications) {
                    if (notify_id != notification.object_id)
                        continue;
                    is_notify_id_found = true;
                    break;
                }
            }
            if (!is_notify_id_found) {
                printf("Map %d: Invalid notify id found %d in map %s\n", map.object_id.value,
                       notify_id, map.name.text.c_str());
            }
        }
    }
    return false;
}

int ValidateDatabase(DudeDatabase* db) {
    if (CheckNewDataFormats(db)) {
        printf(
            "This database contains new data formats. Please contact developer to add support.\n");
        return 1;
    }

    if (CheckDatabaseIds(db)) {
        printf("This database contains invalid object references.\n");
        return 2;
    }

    printf("Database health: OK\n");
    return 0;
}

} // namespace Database
