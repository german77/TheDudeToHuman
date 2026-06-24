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
    bool is_invalid = false;
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
            is_invalid = true;
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
                is_invalid = true;
            }
        }

        // TODO: Validate groupNotifyIds
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
                is_invalid = true;
            }
        }

        for (const s32& notify_id : map.notify_ids.data) {
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
                is_invalid = true;
            }
        }
    }

    std::vector<ProbeData> probes = db->GetProbeData();
    for (const ProbeData& probe : probes) {
        if (probe.snmp_profile_id != -1) {
            bool is_snmp_profile_found = false;
            for (const SnmpProfileData& profile : snmp_profiles) {
                if (probe.snmp_profile_id != profile.object_id)
                    continue;
                is_snmp_profile_found = true;
                break;
            }

            // TODO: Validate logicProbeIds

            if (!is_snmp_profile_found) {
                printf("Probe %d: Invalid snmp profile id found %d in probe %s\n",
                       probe.object_id.value, probe.snmp_profile_id.value, probe.name.text.c_str());
                is_invalid = true;
            }
        }
    }

    std::vector<LinkTypeData> link_types = db->GetLinkTypeData();
    for (const LinkTypeData& link_type : link_types) {
        bool is_next_id_found = false;
        for (const LinkTypeData& next_link : link_types) {
            // Can reference itself
            if (link_type.next_id != next_link.object_id)
                continue;
            is_next_id_found = true;
            break;
        }

        if (!is_next_id_found) {
            printf("Link type %d: Invalid next id found %d in link type %s\n",
                   link_type.object_id.value, link_type.next_id.value, link_type.name.text.c_str());
            is_invalid = true;
        }
    }

    std::vector<DeviceTypeData> device_types = db->GetDeviceTypeData();
    for (const DeviceTypeData& device_type : device_types) {
        for (s32 ignored_service_id : device_type.ignored_services.data) {
            bool is_probe_found = false;
            for (const ProbeData& probe : probes) {
                if (ignored_service_id != probe.object_id)
                    continue;
                is_probe_found = true;
                break;
            }
            if (!is_probe_found) {
                printf("Device type %d: Invalid ignored service found %d in device type %s\n",
                       device_type.object_id.value, ignored_service_id,
                       device_type.name.text.c_str());
                is_invalid = true;
            }
        }
        for (s32 ignored_service_id : device_type.allowed_services.data) {
            bool is_probe_found = false;
            for (const ProbeData& probe : probes) {
                if (ignored_service_id != probe.object_id)
                    continue;
                is_probe_found = true;
                break;
            }
            if (!is_probe_found) {
                printf("Device type %d: Invalid allowed service found %d in device type %s\n",
                       device_type.object_id.value, ignored_service_id,
                       device_type.name.text.c_str());
                is_invalid = true;
            }
        }
        for (s32 ignored_service_id : device_type.required_services.data) {
            bool is_probe_found = false;
            for (const ProbeData& probe : probes) {
                if (ignored_service_id != probe.object_id)
                    continue;
                is_probe_found = true;
                break;
            }
            if (!is_probe_found) {
                printf("Device type %d: Invalid required service found %d in device type %s\n",
                       device_type.object_id.value, ignored_service_id,
                       device_type.name.text.c_str());
                is_invalid = true;
            }
        }

        if (device_type.image_id != -1) {
            bool is_image_found = false;
            for (const FileData& image_file : files) {
                if (device_type.image_id != image_file.object_id)
                    continue;
                is_image_found = true;
                break;
            }

            if (!is_image_found) {
                printf("Device type %d: Invalid image file found %d in device type %s\n",
                       device_type.object_id.value, device_type.image_id.value,
                       device_type.name.text.c_str());
                is_invalid = true;
            }
        }

        bool is_next_id_found = false;
        for (const DeviceTypeData& next_device : device_types) {
            // Can reference itself
            if (device_type.next_id != next_device.object_id)
                continue;
            is_next_id_found = true;
            break;
        }

        if (!is_next_id_found) {
            printf("Device type %d: Invalid next id found %d in device type %s\n",
                   device_type.object_id.value, device_type.next_id.value,
                   device_type.name.text.c_str());
            is_invalid = true;
        }
    }

    std::vector<DeviceData> devices = db->GetDeviceData();
    for (const DeviceData& device : devices) {
        for (const s32& parent_id : device.parent_ids.data) {
            bool is_parent_found = false;
            for (const DeviceData& parent : devices) {
                if (device.object_id == parent.object_id)
                    continue;
                if (parent_id != parent.object_id)
                    continue;
                is_parent_found = true;
                break;
            }
            if (!is_parent_found) {
                printf("Device %d: Invalid parent found %d in device %s\n", device.object_id.value,
                       parent_id, device.name.text.c_str());
                is_invalid = true;
            }
        }

        for (const s32& notify_id : device.notify_ids.data) {
            bool is_notify_id_found = false;
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
            if (!is_notify_id_found) {
                printf("Device %d: Invalid notify id found %d in device %s\n",
                       device.object_id.value, notify_id, device.name.text.c_str());
                is_invalid = true;
            }
        }

        if (device.type_id != -1) {
            bool is_device_type_found = false;
            for (const DeviceTypeData& device_type : device_types) {
                if (device.type_id != device_type.object_id)
                    continue;
                is_device_type_found = true;
                break;
            }
            if (!is_device_type_found) {
                printf("Device %d: Invalid device type found %d in device %s\n",
                       device.object_id.value, device.type_id.value, device.name.text.c_str());
                is_invalid = true;
            }
        }

        if (device.agent_id != -1) {
            bool is_agent_found = false;
            for (const DeviceData& agent : devices) {
                if (device.object_id == agent.object_id)
                    continue;
                if (device.agent_id != agent.object_id)
                    continue;
                is_agent_found = true;
                break;
            }
            if (!is_agent_found) {
                printf("Device %d: Invalid agent found %d in device %s\n", device.object_id.value,
                       device.agent_id.value, device.name.text.c_str());
                is_invalid = true;
            }
        }

        if (device.snmp_profile_id != -1) {
            bool is_snmp_profile_found = false;
            for (const SnmpProfileData& profile : snmp_profiles) {
                if (device.snmp_profile_id != profile.object_id)
                    continue;
                is_snmp_profile_found = true;
                break;
            }
            if (!is_snmp_profile_found) {
                printf("Device %d: Invalid snmp profile id found %d in device %s\n",
                       device.object_id.value, device.snmp_profile_id.value,
                       device.name.text.c_str());
                is_invalid = true;
            }
        }
    }

    std::vector<DataSourceData> data_sources = db->GetDataSourceData();
    /* for (const DataSourceData& data_source : data_sources) {
        // TODO: Validate functionDeviceId
    }*/

    std::vector<ChartLineData> charts = db->GetChartLineData();
    for (const ChartLineData& chart : charts) {
        bool is_chart_found = false;
        for (const ObjectListData& object : object_lists) {
            if (chart.chart_id != object.object_id)
                continue;
            is_chart_found = true;
            break;
        }
        if (!is_chart_found) {
            printf("Chart %d: Invalid chart id found %d in chart %s\n", chart.object_id.value,
                   chart.chart_id.value, chart.name.text.c_str());
            is_invalid = true;
        }

        bool is_data_source_found = false;
        for (const DataSourceData& data_source : data_sources) {
            if (chart.source_id != data_source.object_id)
                continue;
            is_data_source_found = true;
            break;
        }
        if (!is_data_source_found) {
            printf("Chart %d: Invalid data source found %d in chart %s\n", chart.object_id.value,
                   chart.source_id.value, chart.name.text.c_str());
            is_invalid = true;
        }

        if (chart.next_id != -1) {
            bool is_next_id_found = false;
            for (const ChartLineData& next_chart : charts) {
                // Can reference itself
                if (chart.next_id != next_chart.object_id)
                    continue;
                is_next_id_found = true;
                break;
            }

            if (!is_next_id_found) {
                printf("Chart %d: Invalid next id found %d in chart %s\n", chart.object_id.value,
                       chart.next_id.value, chart.name.text.c_str());
                is_invalid = true;
            }
        }
    }

    std::vector<SysLogRuleData> sys_log_rules = db->GetSysLogRuleData();
    for (const SysLogRuleData& sys_log_rule : sys_log_rules) {
        if (sys_log_rule.next_id != -1) {
            bool is_next_id_found = false;
            for (const SysLogRuleData& next_rule : sys_log_rules) {
                if (sys_log_rule.object_id == next_rule.object_id)
                    continue;
                if (sys_log_rule.next_id != next_rule.object_id)
                    continue;
                is_next_id_found = true;
                break;
            }

            if (!is_next_id_found) {
                printf("Syslog rule %d: Invalid next id found %d in rule %s\n",
                       sys_log_rule.object_id.value, sys_log_rule.next_id.value,
                       sys_log_rule.name.text.c_str());
                is_invalid = true;
            }
        }
    }

    return is_invalid;
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
