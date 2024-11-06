// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cstddef>
#include <cstdio>

#include "database/dude_field_parser.h"
#include "dude_database.h"

namespace Database {
DudeDatabase::DudeDatabase(const std::string& db_file) : db{db_file} {
    int rc = db.OpenDatabase();
    if (rc != 0) {
        printf("Error at '%s': %s\n", db_file.c_str(), db.GetError());
        return;
    }
    printf("Opened database successfully\n");
}

DudeDatabase::~DudeDatabase() {
    db.CloseDatabase();
}

int DudeDatabase::GetChartValuesRaw(SqlData& data) const {
    return db.GetTableData(data, "chart_values_raw");
}

int DudeDatabase::GetChartValues10Min(SqlData& data) const {
    return db.GetTableData(data, "chart_values_10min");
}

int DudeDatabase::GetChartValues2Hour(SqlData& data) const {
    return db.GetTableData(data, "chart_values_2hour");
}

int DudeDatabase::GetChartValues1Day(SqlData& data) const {
    return db.GetTableData(data, "chart_values_1day");
}

int DudeDatabase::GetObjs(SqlData& data) const {
    return db.GetTableData(data, "objs");
}

int DudeDatabase::GetOutages(SqlData& data) const {
    return db.GetTableData(data, "outages");
}

std::vector<DataFormat> DudeDatabase::ListUsedDataFormats() const {
    std::vector<DataFormat> data_formats{};
    Database::SqlData sql_data{};
    GetObjs(sql_data);

    for (auto& [id, blob] : sql_data) {
        DudeFieldParser parser{blob};
        for (auto& data_format : parser.GetFormat().data) {
            const DataFormat format = static_cast<DataFormat>(data_format);
            const auto it = std::find(data_formats.begin(), data_formats.end(), format);

            if (it != data_formats.end()) {
                continue;
            }

            printf("New Format %d in row %d \n", data_format, id);
            data_formats.push_back(format);
        }
    }

    return data_formats;
}

template <typename T>
std::vector<T> DudeDatabase::GetObjectData(DataFormat format,
                                           T (DudeDatabase::*RawToObjData)(DudeFieldParser& parser)
                                               const) const {
    std::vector<T> data{};
    Database::SqlData sql_data{};
    GetObjs(sql_data);

    for (auto& [id, blob] : sql_data) {
        DudeFieldParser parser{blob};

        if (parser.GetMainFormat() != format) {
            continue;
        }

        printf("Reading row %d\n", id);

        const T obj_data = (this->*RawToObjData)(parser);

        if (id != obj_data.object_id.value) {
            printf("Corrupted Entry\n");
        }

        data.push_back(obj_data);
    }

    return data;
}

std::vector<ServerConfigData> DudeDatabase::GetServerConfigData() const {
    return GetObjectData<ServerConfigData>(DataFormat::ServerConfig,
                                           &DudeDatabase::GetServerConfigData);
}

std::vector<ToolData> DudeDatabase::GetToolData() const {
    return GetObjectData<ToolData>(DataFormat::Tool, &DudeDatabase::GetToolData);
}

std::vector<FileData> DudeDatabase::GetFileData() const {
    return GetObjectData<FileData>(DataFormat::File, &DudeDatabase::GetFileData);
}

std::vector<NotesData> DudeDatabase::GetNotesData() const {
    return GetObjectData<NotesData>(DataFormat::Notes, &DudeDatabase::GetNotesData);
}

std::vector<MapData> DudeDatabase::GetMapData() const {
    return GetObjectData<MapData>(DataFormat::Map, &DudeDatabase::GetMapData);
}

std::vector<DeviceTypeData> DudeDatabase::GetDeviceTypeData() const {
    return GetObjectData<DeviceTypeData>(DataFormat::DeviceType, &DudeDatabase::GetDeviceTypeData);
}

std::vector<DeviceData> DudeDatabase::GetDeviceData() const {
    return GetObjectData<DeviceData>(DataFormat::Device, &DudeDatabase::GetDeviceData);
}

std::vector<NetworkData> DudeDatabase::GetNetworkData() const {
    return GetObjectData<NetworkData>(DataFormat::Network, &DudeDatabase::GetNetworkData);
}

std::vector<ServiceData> DudeDatabase::GetServiceData() const {
    return GetObjectData<ServiceData>(DataFormat::Service, &DudeDatabase::GetServiceData);
}

std::vector<NotificationData> DudeDatabase::GetNotificationData() const {
    return GetObjectData<NotificationData>(DataFormat::Notification,
                                           &DudeDatabase::GetNotificationData);
}

std::vector<LinkData> DudeDatabase::GetLinkData() const {
    return GetObjectData<LinkData>(DataFormat::Link, &DudeDatabase::GetLinkData);
}

std::vector<LinkTypeData> DudeDatabase::GetLinkTypeData() const {
    return GetObjectData<LinkTypeData>(DataFormat::LinkType, &DudeDatabase::GetLinkTypeData);
}

std::vector<DataSourceData> DudeDatabase::GetDataSourceData() const {
    return GetObjectData<DataSourceData>(DataFormat::DataSource, &DudeDatabase::GetDataSourceData);
}

std::vector<ObjectListData> DudeDatabase::GetObjectListData() const {
    return GetObjectData<ObjectListData>(DataFormat::ObjectList, &DudeDatabase::GetObjectListData);
}

std::vector<DeviceGroupData> DudeDatabase::GetDeviceGroupData() const {
    return GetObjectData<DeviceGroupData>(DataFormat::DeviceGroup,
                                          &DudeDatabase::GetDeviceGroupData);
}

std::vector<FunctionData> DudeDatabase::GetFunctionData() const {
    return GetObjectData<FunctionData>(DataFormat::Function, &DudeDatabase::GetFunctionData);
}

std::vector<SnmpProfileData> DudeDatabase::GetSnmpProfileData() const {
    return GetObjectData<SnmpProfileData>(DataFormat::SnmpProfile,
                                          &DudeDatabase::GetSnmpProfileData);
}

std::vector<PanelData> DudeDatabase::GetPanelData() const {
    return GetObjectData<PanelData>(DataFormat::Panel, &DudeDatabase::GetPanelData);
}

std::vector<SysLogRuleData> DudeDatabase::GetSysLogRuleData() const {
    return GetObjectData<SysLogRuleData>(DataFormat::SysLogRule, &DudeDatabase::GetSysLogRuleData);
}

std::vector<NetworkMapElementData> DudeDatabase::GetNetworkMapElementData() const {
    return GetObjectData<NetworkMapElementData>(DataFormat::NetworkMapElement,
                                                &DudeDatabase::GetNetworkMapElementData);
}

std::vector<ChartLineData> DudeDatabase::GetChartLineData() const {
    return GetObjectData<ChartLineData>(DataFormat::ChartLine, &DudeDatabase::GetChartLineData);
}

std::vector<PanelElementData> DudeDatabase::GetPanelElementData() const {
    return GetObjectData<PanelElementData>(DataFormat::PanelElement,
                                           &DudeDatabase::GetPanelElementData);
}

ServerConfigData DudeDatabase::GetServerConfigData(DudeFieldParser& parser) const {
    ServerConfigData data{};

    parser.ReadField(data.time_zone_history, FieldId::ServerConfig_TimeZoneHistory);
    parser.ReadField(data.discover_skip_types, FieldId::ServerConfig_DiscoverSkipTypes);
    parser.ReadField(data.discover_skip_probes, FieldId::ServerConfig_DiscoverSkipProbes);
    parser.ReadField(data.custom_colors, FieldId::ServerConfig_CustomColors);
    parser.ReadField(data.chart_line_colors, FieldId::ServerConfig_ChartLineColors);
    parser.ReadField(data.notify_ids, FieldId::ServerConfig_NotifyIDs);
    parser.ReadField(data.discover_identification, FieldId::ServerConfig_DiscoverIdentification);
    parser.ReadField(data.discover_networks, FieldId::ServerConfig_DiscoverNetworks);
    parser.ReadField(data.discover_links, FieldId::ServerConfig_DiscoverLinks);
    parser.ReadField(data.map_device_visible, FieldId::ServerConfig_MapDeviceVisible);
    parser.ReadField(data.discover_layer_2, FieldId::ServerConfig_DiscoverLayer2);
    parser.ReadField(data.first_connection, FieldId::ServerConfig_FirstConnection);
    parser.ReadField(data.discover_ppp, FieldId::ServerConfig_DiscoverPpp);
    parser.ReadField(data.discover_graph_services, FieldId::ServerConfig_DiscoverGraphServices);
    parser.ReadField(data.map_network_visible, FieldId::ServerConfig_MapNetworkVisible);
    parser.ReadField(data.discover_graph_links, FieldId::ServerConfig_DiscoverGraphLinks);
    parser.ReadField(data.discover_service_less, FieldId::ServerConfig_DiscoverServiceLess);
    parser.ReadField(data.map_submap_visible, FieldId::ServerConfig_MapSubmapVisible);
    parser.ReadField(data.probe_enabled, FieldId::ServerConfig_ProbeEnabled);
    parser.ReadField(data.map_static_visible, FieldId::ServerConfig_MapStaticVisible);
    parser.ReadField(data.syslog_enabled, FieldId::ServerConfig_SyslogEnabled);
    parser.ReadField(data.map_link_visible, FieldId::ServerConfig_MapLinkVisible);
    parser.ReadField(data.snmp_trap_enabled, FieldId::ServerConfig_SnmpTrapEnabled);
    parser.ReadField(data.confirm_remove, FieldId::ServerConfig_ConfirmRemove);
    parser.ReadField(data.resolve_mac_address_manufacturer,
                     FieldId::ServerConfig_ResolveMACAddressManufacturer);
    parser.ReadField(data.map_dep_visible, FieldId::ServerConfig_MapDepVisible);
    parser.ReadField(data.map_antialiased_geometry, FieldId::ServerConfig_MapAntialiasedGeometry);
    parser.ReadField(data.map_gradients, FieldId::ServerConfig_MapGradients);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.version, FieldId::ServerConfig_Version);
    parser.ReadField(data.snmp_profile_id, FieldId::ServerConfig_SnmpProfileID);
    parser.ReadField(data.agent_id, FieldId::ServerConfig_AgentID);
    parser.ReadField(data.probe_interval, FieldId::ServerConfig_ProbeInterval);
    parser.ReadField(data.probe_timeout, FieldId::ServerConfig_ProbeTimeout);
    parser.ReadField(data.probe_down_count, FieldId::ServerConfig_ProbeDownCount);
    parser.ReadField(data.syslog_port, FieldId::ServerConfig_SyslogPort);
    parser.ReadField(data.snmp_trap_port, FieldId::ServerConfig_SnmpTrapPort);
    parser.ReadField(data.map_background_color, FieldId::ServerConfig_MapBackgroundColor);
    parser.ReadField(data.map_label_refresh_interval,
                     FieldId::ServerConfig_MapLabelRefreshInterval);
    parser.ReadField(data.map_up_color, FieldId::ServerConfig_MapUpColor);
    parser.ReadField(data.map_down_partial_color, FieldId::ServerConfig_MapDownPartialColor);
    parser.ReadField(data.map_down_complete_color, FieldId::ServerConfig_MapDownCompleteColor);
    parser.ReadField(data.map_unknown_color, FieldId::ServerConfig_MapUnknownColor);
    parser.ReadField(data.map_acked_color, FieldId::ServerConfig_MapAckedColor);
    parser.ReadField(data.map_network_color, FieldId::ServerConfig_MapNetworkColor);
    parser.ReadField(data.map_submap_color, FieldId::ServerConfig_MapSubmapColor);
    parser.ReadField(data.map_submap_up_color, FieldId::ServerConfig_MapSubmapUpColor);
    parser.ReadField(data.map_submap_down_partial_color,
                     FieldId::ServerConfig_MapSubmapDownPartialColor);
    parser.ReadField(data.map_submap_down_complete_color,
                     FieldId::ServerConfig_MapSubmapDownCompleteColor);
    parser.ReadField(data.map_submap_acked_color, FieldId::ServerConfig_MapSubmapAckedColor);
    parser.ReadField(data.map_static_color, FieldId::ServerConfig_MapStaticColor);
    parser.ReadField(data.map_link_color, FieldId::ServerConfig_MapLinkColor);
    parser.ReadField(data.map_link_label_color, FieldId::ServerConfig_MapLinkLabelColor);
    parser.ReadField(data.map_link_full_color, FieldId::ServerConfig_MapLinkFullColor);
    parser.ReadField(data.map_device_shape, FieldId::ServerConfig_MapDeviceShape);
    parser.ReadField(data.map_network_shape, FieldId::ServerConfig_MapNetworkShape);
    parser.ReadField(data.map_submap_shape, FieldId::ServerConfig_MapSubmapShape);
    parser.ReadField(data.map_static_shape, FieldId::ServerConfig_MapStaticShape);
    parser.ReadField(data.map_link_thickness, FieldId::ServerConfig_MapLinkThickness);
    parser.ReadField(data.map_dep_color, FieldId::ServerConfig_MapDepColor);
    parser.ReadField(data.map_dep_thickness, FieldId::ServerConfig_MapDepThickness);
    parser.ReadField(data.map_dep_style, FieldId::ServerConfig_MapDepStyle);
    parser.ReadField(data.chart_value_keep_time_raw, FieldId::ServerConfig_ChartValueKeepTimeRaw);
    parser.ReadField(data.chart_value_keep_time_10_min,
                     FieldId::ServerConfig_ChartValueKeepTime10min);
    parser.ReadField(data.chart_value_keep_time_2_hour,
                     FieldId::ServerConfig_ChartValueKeepTime2hour);
    parser.ReadField(data.chart_value_keep_time_1_day,
                     FieldId::ServerConfig_ChartValueKeepTime1day);
    parser.ReadField(data.chart_background_color, FieldId::ServerConfig_ChartBackgroundColor);
    parser.ReadField(data.chart_grid_color, FieldId::ServerConfig_ChartGridColor);
    parser.ReadField(data.chart_text_color, FieldId::ServerConfig_ChartTextColor);
    parser.ReadField(data.discover_name_preference, FieldId::ServerConfig_DiscoverNamePreference);
    parser.ReadField(data.discover_mode, FieldId::ServerConfig_DiscoverMode);
    parser.ReadField(data.discover_hops, FieldId::ServerConfig_DiscoverHops);
    parser.ReadField(data.discover_hop_network_size_limit,
                     FieldId::ServerConfig_DiscoverHopNetworkSizeLimit);
    parser.ReadField(data.discover_simultaneous, FieldId::ServerConfig_DiscoverSimultaneous);
    parser.ReadField(data.discover_interval, FieldId::ServerConfig_DiscoverInterval);
    parser.ReadField(data.discover_item_width, FieldId::ServerConfig_DiscoverItemWidth);
    parser.ReadField(data.discover_item_height, FieldId::ServerConfig_DiscoverItemHeight);
    parser.ReadField(data.discover_big_row, FieldId::ServerConfig_DiscoverBigRow);
    parser.ReadField(data.discover_big_column, FieldId::ServerConfig_DiscoverBigColumn);
    parser.ReadField(data.discover_whole_row, FieldId::ServerConfig_DiscoverWholeRow);
    parser.ReadField(data.discover_whole_column, FieldId::ServerConfig_DiscoverWholeColumn);
    parser.ReadField(data.ros_conn_interval, FieldId::ServerConfig_RosConnInterval);
    parser.ReadField(data.ros_conn_interval_auth_failed,
                     FieldId::ServerConfig_RosConnIntervalAuthFailed);
    parser.ReadField(data.undo_queue_size, FieldId::ServerConfig_UndoQueueSize);
    parser.ReadField(data.mac_mapping_refresh_interval,
                     FieldId::ServerConfig_MacMappingRefreshInterval);
    parser.ReadField(data.contents_pane_behavior, FieldId::ServerConfig_ContentsPaneBehavior);
    parser.ReadField(data.last_chart_maintenance_time,
                     FieldId::ServerConfig_LastChartMaintenanceTime);
    parser.ReadField(data.discover_black_list, FieldId::ServerConfig_DiscoverBlackList);
    parser.ReadField(data.report_font, FieldId::ServerConfig_ReportFont);
    parser.ReadField(data.chart_font, FieldId::ServerConfig_ChartFont);
    parser.ReadField(data.map_link_font, FieldId::ServerConfig_MapLinkFont);
    parser.ReadField(data.map_link_tooltip, FieldId::ServerConfig_MapLinkTooltip);
    parser.ReadField(data.map_link_label, FieldId::ServerConfig_MapLinkLabel);
    parser.ReadField(data.map_static_font, FieldId::ServerConfig_MapStaticFont);
    parser.ReadField(data.map_submap_font, FieldId::ServerConfig_MapSubmapFont);
    parser.ReadField(data.map_submap_tooltip, FieldId::ServerConfig_MapSubmapTooltip);
    parser.ReadField(data.map_submap_label, FieldId::ServerConfig_MapSubmapLabel);
    parser.ReadField(data.map_network_font, FieldId::ServerConfig_MapNetworkFont);
    parser.ReadField(data.map_network_tooltip, FieldId::ServerConfig_MapNetworkTooltip);
    parser.ReadField(data.map_network_label, FieldId::ServerConfig_MapNetworkLabel);
    parser.ReadField(data.map_device_font, FieldId::ServerConfig_MapDeviceFont);
    parser.ReadField(data.map_device_tooltip, FieldId::ServerConfig_MapDeviceTooltip);
    parser.ReadField(data.map_device_label, FieldId::ServerConfig_MapDeviceLabel);
    parser.ReadField(data.unique_id, FieldId::ServerConfig_UniqueID);
    parser.ReadField(data.name, FieldId::SysName);
    return data;
}

ToolData DudeDatabase::GetToolData(DudeFieldParser& parser) const {
    ToolData data{};

    parser.ReadField(data.builtin, FieldId::Tool_Builtin);
    parser.ReadField(data.type, FieldId::Tool_Type);
    parser.ReadField(data.device_id, FieldId::Tool_DeviceID);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.command, FieldId::Tool_Command);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

FileData DudeDatabase::GetFileData(DudeFieldParser& parser) const {
    FileData data{};

    parser.ReadField(data.parent_id, FieldId::File_ParentID);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.file_name, FieldId::File_FileName);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

NotesData DudeDatabase::GetNotesData(DudeFieldParser& parser) const {
    NotesData data{};

    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.parent_id, FieldId::Note_ObjID);
    parser.ReadField(data.time_added, FieldId::Note_TimeAdded);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

MapData DudeDatabase::GetMapData(DudeFieldParser& parser) const {
    MapData data{};

    parser.ReadField(data.notify_ids, FieldId::NetworkMap_NotifyIDs);
    parser.ReadField(data.use_static_color, FieldId::NetworkMap_UseStaticColor);
    parser.ReadField(data.use_link_color, FieldId::NetworkMap_UseLinkColor);
    parser.ReadField(data.use_link_label_color, FieldId::NetworkMap_UseLinkLabelColor);
    parser.ReadField(data.use_link_full_color, FieldId::NetworkMap_UseLinkFullColor);
    parser.ReadField(data.use_device_label, FieldId::NetworkMap_UseDeviceLabel);
    parser.ReadField(data.use_device_shape, FieldId::NetworkMap_UseDeviceShape);
    parser.ReadField(data.use_device_font, FieldId::NetworkMap_UseDeviceFont);
    parser.ReadField(data.use_network_label, FieldId::NetworkMap_UseNetworkLabel);
    parser.ReadField(data.use_network_shape, FieldId::NetworkMap_UseNetworkShape);
    parser.ReadField(data.use_network_font, FieldId::NetworkMap_UseNetworkFont);
    parser.ReadField(data.use_submap_label, FieldId::NetworkMap_UseSubmapLabel);
    parser.ReadField(data.use_submap_shape, FieldId::NetworkMap_UseSubmapShape);
    parser.ReadField(data.use_submap_font, FieldId::NetworkMap_UseSubmapFont);
    parser.ReadField(data.use_static_shape, FieldId::NetworkMap_UseStaticShape);
    parser.ReadField(data.use_static_font, FieldId::NetworkMap_UseStaticFont);
    parser.ReadField(data.use_link_label, FieldId::NetworkMap_UseLinkLabel);
    parser.ReadField(data.use_link_font, FieldId::NetworkMap_UseLinkFont);
    parser.ReadField(data.use_link_thickness, FieldId::NetworkMap_UseLinkThickness);
    parser.ReadField(data.ordered, FieldId::ObjectList_Ordered);
    parser.ReadField(data.prove_enabled, FieldId::NetworkMap_ProbeEnabled);
    parser.ReadField(data.notify_use, FieldId::NetworkMap_NotifyUse);
    parser.ReadField(data.report_scanning, FieldId::NetworkMap_ReportScanning);
    parser.ReadField(data.locked, FieldId::NetworkMap_Locked);
    parser.ReadField(data.image_tile, FieldId::NetworkMap_ImageTile);
    parser.ReadField(data.color_visible, FieldId::NetworkMap_ColorVisible);
    parser.ReadField(data.device_visible, FieldId::NetworkMap_DeviceVisible);
    parser.ReadField(data.network_visible, FieldId::NetworkMap_NetworkVisible);
    parser.ReadField(data.submap_visible, FieldId::NetworkMap_SubmapVisible);
    parser.ReadField(data.static_visible, FieldId::NetworkMap_StaticVisible);
    parser.ReadField(data.link_visible, FieldId::NetworkMap_LinkVisible);
    parser.ReadField(data.use_background_color, FieldId::NetworkMap_UseBackgroundColor);
    parser.ReadField(data.use_up_color, FieldId::NetworkMap_UseUpColor);
    parser.ReadField(data.use_down_partial_color, FieldId::NetworkMap_UseDownPartialColor);
    parser.ReadField(data.use_down_complete_color, FieldId::NetworkMap_UseDownCompleteColor);
    parser.ReadField(data.use_unknown_color, FieldId::NetworkMap_UseUnknownColor);
    parser.ReadField(data.use_acked_color, FieldId::NetworkMap_UseAckedColor);
    parser.ReadField(data.use_network_color, FieldId::NetworkMap_UseNetworkColor);
    parser.ReadField(data.use_submap_color, FieldId::NetworkMap_UseSubmapColor);
    parser.ReadField(data.use_submap_up_color, FieldId::NetworkMap_UseSubmapUpColor);
    parser.ReadField(data.use_submap_down_partial_color,
                     FieldId::NetworkMap_UseSubmapDownPartialColor);
    parser.ReadField(data.use_submap_down_complete_color,
                     FieldId::NetworkMap_UseSubmapDownCompleteColor);
    parser.ReadField(data.use_submap_acked_color, FieldId::NetworkMap_UseSubmapAckedColor);
    parser.ReadField(data.link_thickness, FieldId::NetworkMap_LinkThickness);
    parser.ReadField(data.layout_density, FieldId::NetworkMap_LayoutDensity);
    parser.ReadField(data.layout_quality, FieldId::NetworkMap_LayoutQuality);
    parser.ReadField(data.prove_interval, FieldId::NetworkMap_ProbeInterval);
    parser.ReadField(data.prove_timeout, FieldId::NetworkMap_ProbeTimeout);
    parser.ReadField(data.prove_down_count, FieldId::NetworkMap_ProbeDownCount);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.default_zoom, FieldId::NetworkMap_DefaultZoom);
    parser.ReadField(data.image_id, FieldId::NetworkMap_ImageID);
    parser.ReadField(data.image_scale, FieldId::NetworkMap_ImageScale);
    parser.ReadField(data.label_refresh_interval, FieldId::NetworkMap_LabelRefreshInterval);
    parser.ReadField(data.background_color, FieldId::NetworkMap_BackgroundColor);
    parser.ReadField(data.up_color, FieldId::NetworkMap_UpColor);
    parser.ReadField(data.down_partial_color, FieldId::NetworkMap_DownPartialColor);
    parser.ReadField(data.down_complete_color, FieldId::NetworkMap_DownCompleteColor);
    parser.ReadField(data.unknown_color, FieldId::NetworkMap_UnknownColor);
    parser.ReadField(data.acked_color, FieldId::NetworkMap_AckedColor);
    parser.ReadField(data.network_color, FieldId::NetworkMap_NetworkColor);
    parser.ReadField(data.submap_color, FieldId::NetworkMap_SubmapColor);
    parser.ReadField(data.submap_up_color, FieldId::NetworkMap_SubmapUpColor);
    parser.ReadField(data.submap_down_partial_color, FieldId::NetworkMap_SubmapDownPartialColor);
    parser.ReadField(data.submap_down_complete_color, FieldId::NetworkMap_SubmapDownCompleteColor);
    parser.ReadField(data.submap_acked_color, FieldId::NetworkMap_SubmapAckedColor);
    parser.ReadField(data.static_color, FieldId::NetworkMap_StaticColor);
    parser.ReadField(data.link_color, FieldId::NetworkMap_LinkColor);
    parser.ReadField(data.link_label_color, FieldId::NetworkMap_LinkLabelColor);
    parser.ReadField(data.link_full_color, FieldId::NetworkMap_LinkFullColor);
    parser.ReadField(data.device_shape, FieldId::NetworkMap_DeviceShape);
    parser.ReadField(data.network_shape, FieldId::NetworkMap_NetworkShape);
    parser.ReadField(data.submap_shape, FieldId::NetworkMap_SubmapShape);
    parser.ReadField(data.static_shape, FieldId::NetworkMap_StaticShape);
    parser.ReadField(data.link_font, FieldId::NetworkMap_LinkFont);
    parser.ReadField(data.link_label, FieldId::NetworkMap_LinkLabel);
    parser.ReadField(data.static_font, FieldId::NetworkMap_StaticFont);
    parser.ReadField(data.submap_font, FieldId::NetworkMap_SubmapFont);
    parser.ReadField(data.submap_label, FieldId::NetworkMap_SubmapLabel);
    parser.ReadField(data.network_font, FieldId::NetworkMap_NetworkFont);
    parser.ReadField(data.network_label, FieldId::NetworkMap_NetworkLabel);
    parser.ReadField(data.device_font, FieldId::NetworkMap_DeviceFont);
    parser.ReadField(data.device_label, FieldId::NetworkMap_DeviceLabel);
    parser.ReadField(data.list_type, FieldId::ObjectList_Type);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

DeviceTypeData DudeDatabase::GetDeviceTypeData(DudeFieldParser& parser) const {
    DeviceTypeData data{};

    parser.ReadField(data.ignored_services, FieldId::DeviceType_IgnoredServices);
    parser.ReadField(data.allowed_services, FieldId::DeviceType_AllowedServices);
    parser.ReadField(data.required_services, FieldId::DeviceType_RequiredServices);
    parser.ReadField(data.image_id, FieldId::DeviceType_ImageId);
    parser.ReadField(data.image_scale, FieldId::DeviceType_ImageScale);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.next_id, FieldId::SysNextId);
    parser.ReadField(data.url, FieldId::DeviceType_Url);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

DeviceData DudeDatabase::GetDeviceData(DudeFieldParser& parser) const {
    DeviceData data{};

    parser.ReadField(data.parent_ids, FieldId::Device_ParentIds);
    parser.ReadField(data.notify_ids, FieldId::Device_NotifyIds);
    parser.ReadField(data.dns_names, FieldId::Device_DnsNames);
    parser.ReadField(data.ip, FieldId::Device_IpAddress);
    parser.ReadField(data.secure_mode, FieldId::Device_SecureMode);
    parser.ReadField(data.router_os, FieldId::Device_RouterOs);
    parser.ReadField(data.dude_server, FieldId::Device_DudeServer);
    parser.ReadField(data.notify_use, FieldId::Device_NotifyUse);
    parser.ReadField(data.prove_enabled, FieldId::Device_ProveEnabled);
    parser.ReadField(data.lookup, FieldId::Device_Lookup);
    parser.ReadField(data.dns_lookup_interval, FieldId::Device_LookupInterval);
    parser.ReadField(data.mac_lookup, FieldId::Device_MacLookup);
    parser.ReadField(data.type_id, FieldId::Device_TypeId);
    parser.ReadField(data.agent_id, FieldId::Device_AgentId);
    parser.ReadField(data.snmp_profile_id, FieldId::Device_SnmpProfileId);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.prove_interval, FieldId::Device_ProveInterval);
    parser.ReadField(data.prove_timeout, FieldId::Device_ProveTimeout);
    parser.ReadField(data.prove_down_count, FieldId::Device_ProveDownCount);
    parser.ReadField(data.custom_field_3, FieldId::Device_CustomField3);
    parser.ReadField(data.custom_field_2, FieldId::Device_CustomField2);
    parser.ReadField(data.custom_field_1, FieldId::Device_CustomField1);
    parser.ReadField(data.password, FieldId::Device_Password);
    parser.ReadField(data.username, FieldId::Device_Username);
    parser.ReadField(data.mac, FieldId::Device_MacAddress);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

NetworkData DudeDatabase::GetNetworkData(DudeFieldParser& parser) const {
    NetworkData data{};

    parser.ReadField(data.subnets, FieldId::Network_Subnets);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.net_map_element, FieldId::Network_NetMapElementID);
    parser.ReadField(data.net_map_id, FieldId::Network_NetMapID);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}
ServiceData DudeDatabase::GetServiceData(DudeFieldParser& parser) const {
    ServiceData data{};

    parser.ReadField(data.notify_ids, FieldId::Service_NotifyIDs);
    parser.ReadField(data.enabled, FieldId::Service_Enabled);
    parser.ReadField(data.history, FieldId::Service_History);
    parser.ReadField(data.notify_use, FieldId::Service_NotifyUse);
    parser.ReadField(data.acked, FieldId::Service_Acked);
    parser.ReadField(data.probe_port, FieldId::Service_ProbePort);
    parser.ReadField(data.probe_interval, FieldId::Service_ProbeInterval);
    parser.ReadField(data.probe_timeout, FieldId::Service_ProbeTimeout);
    parser.ReadField(data.probe_down_count, FieldId::Service_ProbeDownCount);
    parser.ReadField(data.data_source_id, FieldId::Service_DataSourceID);
    parser.ReadField(data.status, FieldId::Service_Status);
    parser.ReadField(data.time_since_changed, FieldId::Service_TimeSinceChanged);
    parser.ReadField(data.time_since_last_up, FieldId::Service_TimeLastUp);
    parser.ReadField(data.time_since_last_down, FieldId::Service_TimeLastDown);
    parser.ReadField(data.time_previous_up, FieldId::Service_TimePrevUp);
    parser.ReadField(data.time_previous_down, FieldId::Service_TimePrevDown);
    parser.ReadField(data.proves_down, FieldId::Service_ProbesDown);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.device_id, FieldId::Service_DeviceID);
    parser.ReadField(data.agent_id, FieldId::Service_AgentID);
    parser.ReadField(data.prove_id, FieldId::Service_probeID);
    parser.ReadField(data.value, FieldId::Service_Value);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

NotificationData DudeDatabase::GetNotificationData(DudeFieldParser& parser) const {
    NotificationData data{};

    parser.ReadField(data.status_list, FieldId::Notification_StatusList);
    parser.ReadField(data.group_notify_ids, FieldId::Notification_GroupNotifyIDs);
    parser.ReadField(data.mail_cc, FieldId::Notification_MailCc);
    parser.ReadField(data.activity, FieldId::Notification_Activity);
    parser.ReadField(data.log_use_color, FieldId::Notification_LogUseColor);
    parser.ReadField(data.enabled, FieldId::Notification_Enabled);
    parser.ReadField(data.mail_tls_mode, FieldId::Notification_MailTlsMode);
    parser.ReadField(data.sys_log_server, FieldId::Notification_SyslogServer);
    parser.ReadField(data.sys_log_port, FieldId::Notification_SyslogPort);
    parser.ReadField(data.sound_file_id, FieldId::Notification_SoundFileID);
    parser.ReadField(data.log_color, FieldId::Notification_LogColor);
    parser.ReadField(data.speak_rate, FieldId::Notification_SpeakRate);
    parser.ReadField(data.speak_volume, FieldId::Notification_SpeakVolume);
    parser.ReadField(data.delay_interval, FieldId::Notification_DelayInterval);
    parser.ReadField(data.repeat_interval, FieldId::Notification_RepeatInterval);
    parser.ReadField(data.repeat_count, FieldId::Notification_RepeatCount);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.rype_id, FieldId::Notification_RypeID);
    parser.ReadField(data.mail_server, FieldId::Notification_MailServer);
    parser.ReadField(data.mail_port, FieldId::Notification_MailPort);
    parser.ReadField(data.log_prefix, FieldId::Notification_LogPrefix);
    parser.ReadField(data.mail_subject, FieldId::Notification_MailSubject);
    parser.ReadField(data.mail_to, FieldId::Notification_MailTo);
    parser.ReadField(data.mail_from, FieldId::Notification_MailFrom);
    parser.ReadField(data.mail_password, FieldId::Notification_MailPassword);
    parser.ReadField(data.mail_user, FieldId::Notification_MailUser);
    parser.ReadField(data.mail_server_dns, FieldId::Notification_MailServerDns);
    parser.ReadField(data.mail_server6, FieldId::Notification_MailServer6);
    parser.ReadField(data.text_template, FieldId::Notification_TextTemplate);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

LinkData DudeDatabase::GetLinkData(DudeFieldParser& parser) const {
    LinkData data{};

    parser.ReadField(data.history, FieldId::Link_History);
    parser.ReadField(data.mastering_type, FieldId::Link_MasteringType);
    parser.ReadField(data.master_device, FieldId::Link_MasterDevice);
    parser.ReadField(data.master_interface, FieldId::Link_MasterInterface);
    parser.ReadField(data.net_map_id, FieldId::Link_NetMapID);
    parser.ReadField(data.net_map_element_id, FieldId::Link_NetMapElementID);
    parser.ReadField(data.type_id, FieldId::Link_TypeID);
    parser.ReadField(data.tx_data_source_id, FieldId::Link_TxDataSourceID);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.rx_data_source_id, FieldId::Link_RxDataSourceID);
    parser.ReadField(data.speed, FieldId::Link_Speed);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

LinkTypeData DudeDatabase::GetLinkTypeData(DudeFieldParser& parser) const {
    LinkTypeData data{};

    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.style, FieldId::LinkType_Style);
    parser.ReadField(data.thickness, FieldId::LinkType_Thickness);
    parser.ReadField(data.snmp_type, FieldId::LinkType_SnmpType);
    parser.ReadField(data.next_id, FieldId::SysNextId);
    parser.ReadField(data.snmp_speed, FieldId::LinkType_SnmpSpeed);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

DataSourceData DudeDatabase::GetDataSourceData(DudeFieldParser& parser) const {
    DataSourceData data{};

    parser.ReadField(data.enabled, FieldId::DataSource_Enabled);
    parser.ReadField(data.function_device_id, FieldId::DataSource_FunctionDevice);
    parser.ReadField(data.function_interval, FieldId::DataSource_FunctionInterval);
    parser.ReadField(data.data_source_type, FieldId::DataSource_Type);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.keep_time_raw, FieldId::DataSource_KeepTimeRaw);
    parser.ReadField(data.keep_time_10min, FieldId::DataSource_KeepTime10min);
    parser.ReadField(data.keep_time_2hour, FieldId::DataSource_KeepTime2hour);
    parser.ReadField(data.keep_time_1Day, FieldId::DataSource_KeepTime1day);
    parser.ReadField(data.function_code, FieldId::DataSource_FunctionCode);
    parser.ReadField(data.unit, FieldId::DataSource_Unit);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

ObjectListData DudeDatabase::GetObjectListData(DudeFieldParser& parser) const {
    ObjectListData data{};

    parser.ReadField(data.ordered, FieldId::ObjectList_Ordered);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.type, FieldId::ObjectList_Type);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

DeviceGroupData DudeDatabase::GetDeviceGroupData(DudeFieldParser& parser) const {
    DeviceGroupData data{};

    parser.ReadField(data.device_ids, FieldId::DeviceGroup_DeviceIDs);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

FunctionData DudeDatabase::GetFunctionData(DudeFieldParser& parser) const {
    FunctionData data{};

    parser.ReadField(data.argument_descriptors, FieldId::Function_ArgumentDescrs);
    parser.ReadField(data.builtin, FieldId::Function_Builtin);
    parser.ReadField(data.min_arguments, FieldId::Function_MinArguments);
    parser.ReadField(data.max_arguments, FieldId::Function_MaxArguments);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.description, FieldId::Function_Descr);
    parser.ReadField(data.code, FieldId::Function_Code);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

SnmpProfileData DudeDatabase::GetSnmpProfileData(DudeFieldParser& parser) const {
    SnmpProfileData data{};

    parser.ReadField(data.version, FieldId::SnmpProfile_Version);
    parser.ReadField(data.port, FieldId::SnmpProfile_Port);
    parser.ReadField(data.security, FieldId::SnmpProfile_V3Security);
    parser.ReadField(data.auth_method, FieldId::SnmpProfile_V3AuthMethod);
    parser.ReadField(data.crypth_method, FieldId::SnmpProfile_V3CryptMethod);
    parser.ReadField(data.try_count, FieldId::SnmpProfile_TryCount);
    parser.ReadField(data.try_timeout, FieldId::SnmpProfile_TryTimeout);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.crypt_password, FieldId::SnmpProfile_V3CryptPassword);
    parser.ReadField(data.auth_password, FieldId::SnmpProfile_V3AuthPassword);
    parser.ReadField(data.community, FieldId::SnmpProfile_Community);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

PanelData DudeDatabase::GetPanelData(DudeFieldParser& parser) const {
    PanelData data{};

    parser.ReadField(data.ordered, FieldId::ObjectList_Ordered);
    parser.ReadField(data.locked, FieldId::Panel_Locked);
    parser.ReadField(data.title_bars, FieldId::Panel_TitleBars);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.top_element_id, FieldId::Panel_TopElementID);
    parser.ReadField(data.admin, FieldId::Panel_Admin);
    parser.ReadField(data.type, FieldId::ObjectList_Type);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

SysLogRuleData DudeDatabase::GetSysLogRuleData(DudeFieldParser& parser) const {
    SysLogRuleData data{};

    parser.ReadField(data.regexp_not, FieldId::SysLogRule_RegexpNot);
    parser.ReadField(data.source_set, FieldId::SysLogRule_SrcSet);
    parser.ReadField(data.regexp_set, FieldId::SysLogRule_RegexpSet);
    parser.ReadField(data.enabled, FieldId::SysLogRule_Enabled);
    parser.ReadField(data.source_not, FieldId::SysLogRule_SrcNot);
    parser.ReadField(data.source_first, FieldId::SysLogRule_SrcFirst);
    parser.ReadField(data.source_second, FieldId::SysLogRule_SrcSecond);
    parser.ReadField(data.action, FieldId::SysLogRule_Action);
    parser.ReadField(data.notify_id, FieldId::SysLogRule_NotifyID);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.next_id, FieldId::SysNextId);
    parser.ReadField(data.regexp, FieldId::SysLogRule_Regexp);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

NetworkMapElementData DudeDatabase::GetNetworkMapElementData(DudeFieldParser& parser) const {
    NetworkMapElementData data{};

    parser.ReadField(data.item_use_acked_color, FieldId::NetworkMapElement_ItemUseAckedColor);
    parser.ReadField(data.item_use_label, FieldId::NetworkMapElement_ItemUseLabel);
    parser.ReadField(data.item_use_shapes, FieldId::NetworkMapElement_ItemUseShape);
    parser.ReadField(data.item_use_font, FieldId::NetworkMapElement_ItemUseFont);
    parser.ReadField(data.item_use_image, FieldId::NetworkMapElement_ItemUseImage);
    parser.ReadField(data.item_use_image_scale, FieldId::NetworkMapElement_ItemUseImageScale);
    parser.ReadField(data.item_use_width, FieldId::NetworkMapElement_LinkUseWidth);
    parser.ReadField(data.item_use_up_color, FieldId::NetworkMapElement_ItemUseUpColor);
    parser.ReadField(data.item_use_down_partial_color,
                     FieldId::NetworkMapElement_ItemUseDownPartialColor);
    parser.ReadField(data.item_use_down_complete_color,
                     FieldId::NetworkMapElement_ItemUseDownCompleteColor);
    parser.ReadField(data.item_use_unknown_color, FieldId::NetworkMapElement_ItemUseUnknownColor);
    parser.ReadField(data.item_up_color, FieldId::NetworkMapElement_ItemUpColor);
    parser.ReadField(data.item_down_partial_color, FieldId::NetworkMapElement_ItemDownPartialColor);
    parser.ReadField(data.item_down_complete_color,
                     FieldId::NetworkMapElement_ItemDownCompleteColor);
    parser.ReadField(data.item_unknown_color, FieldId::NetworkMapElement_ItemUnknownColor);
    parser.ReadField(data.item_acked_color, FieldId::NetworkMapElement_ItemAckedColor);
    parser.ReadField(data.item_shape, FieldId::NetworkMapElement_ItemShape);
    parser.ReadField(data.item_image, FieldId::NetworkMapElement_ItemImage);
    parser.ReadField(data.item_image_scale, FieldId::NetworkMapElement_ItemImageScale);
    parser.ReadField(data.link_from, FieldId::NetworkMapElement_LinkFrom);
    parser.ReadField(data.link_to, FieldId::NetworkMapElement_LinkTo);
    parser.ReadField(data.link_id, FieldId::NetworkMapElement_LinkID);
    parser.ReadField(data.link_width, FieldId::NetworkMapElement_LinkWidth);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.map_id, FieldId::NetworkMapElement_MapID);
    parser.ReadField(data.type, FieldId::NetworkMapElement_Type);
    parser.ReadField(data.item_type, FieldId::NetworkMapElement_ItemType);
    parser.ReadField(data.item_id, FieldId::NetworkMapElement_ItemID);
    parser.ReadField(data.item_x, FieldId::NetworkMapElement_ItemX);
    parser.ReadField(data.item_y, FieldId::NetworkMapElement_ItemY);
    parser.ReadField(data.label_refresh_interval, FieldId::NetworkMapElement_LabelRefreshInterval);
    parser.ReadField(data.item_font, FieldId::NetworkMapElement_ItemFont);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

ChartLineData DudeDatabase::GetChartLineData(DudeFieldParser& parser) const {
    ChartLineData data{};

    parser.ReadField(data.chart_id, FieldId::ChartLine_ChartID);
    parser.ReadField(data.source_id, FieldId::ChartLine_SourceID);
    parser.ReadField(data.line_style, FieldId::ChartLine_LineStyle);
    parser.ReadField(data.line_color, FieldId::ChartLine_LineColor);
    parser.ReadField(data.line_opacity, FieldId::ChartLine_LineOpacity);
    parser.ReadField(data.fill_color, FieldId::ChartLine_FillColor);
    parser.ReadField(data.fill_opacity, FieldId::ChartLine_FillOpacity);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.next_id, FieldId::SysNextId);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

PanelElementData DudeDatabase::GetPanelElementData(DudeFieldParser& parser) const {
    PanelElementData data{};

    parser.ReadField(data.split, FieldId::PanelElement_Split);
    parser.ReadField(data.panel_id, FieldId::PanelElement_PanelID);
    parser.ReadField(data.split_type, FieldId::PanelElement_SplitType);
    parser.ReadField(data.split_share, FieldId::PanelElement_SplitShare);
    parser.ReadField(data.first_id, FieldId::PanelElement_FirstID);
    parser.ReadField(data.second_id, FieldId::PanelElement_SecondID);
    parser.ReadField(data.obj_id, FieldId::PanelElement_ObjID);
    parser.ReadField(data.object_id, FieldId::SysId);
    parser.ReadField(data.obj_meta, FieldId::PanelElement_ObjMeta);
    parser.ReadField(data.name, FieldId::SysName);

    return data;
}

} // namespace Database
