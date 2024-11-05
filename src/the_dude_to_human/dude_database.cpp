// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <cstdio>

#include "dude_database.h"

namespace Database {
DudeDatabase::DudeDatabase(const std::string& db_file) : db{db_file} {
    int rc = db.OpenDatabase();
    if (rc != 0) {
        printf("Can't open database: %s\n", db_file.c_str());
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
        const RawObjData obj_data = BlobToRawObjData(blob);
        for (auto& data_format : obj_data.data_format.data) {
            const DataFormat format = static_cast<DataFormat>(data_format);
            const auto it = find(data_formats.begin(), data_formats.end(), format);

            if (it != data_formats.end()) {
                continue;
            }

            printf("New Format %d in row %d \n", format, id);
            data_formats.push_back(format);
        }
    }

    return data_formats;
}

template <typename T>
std::vector<T> DudeDatabase::GetObjectData(
    DataFormat format, T (DudeDatabase::*RawToObjData)(std::span<const u8> raw_data) const) const {
    std::vector<T> data{};
    Database::SqlData sql_data{};
    GetObjs(sql_data);

    for (auto& [id, blob] : sql_data) {
        const RawObjData raw_obj_data = BlobToRawObjData(blob);

        if (GetMainDataFormat(raw_obj_data) != format) {
            continue;
        }

        printf("Reading row %d\n", id);

        const T obj_data = (this->*RawToObjData)(raw_obj_data.data);

        if (id != obj_data.object_id.value) {
            printf("Corrupted Entry\n");
        }

        data.push_back(obj_data);
    }

    return data;
}

std::vector<ServerConfigurationData> DudeDatabase::GetServerConfigurationData() const {
    return GetObjectData<ServerConfigurationData>(DataFormat::ServerConfiguration,
                                                  &DudeDatabase::RawDataToServerConfigurationData);
}

std::vector<ToolData> DudeDatabase::GetToolData() const {
    return GetObjectData<ToolData>(DataFormat::Tool, &DudeDatabase::RawDataToToolData);
}

std::vector<FileData> DudeDatabase::GetFileData() const {
    return GetObjectData<FileData>(DataFormat::File, &DudeDatabase::RawDataToFileData);
}

std::vector<NotesData> DudeDatabase::GetNotesData() const {
    return GetObjectData<NotesData>(DataFormat::Notes, &DudeDatabase::RawDataToNotesData);
}

std::vector<MapData> DudeDatabase::GetMapData() const {
    return GetObjectData<MapData>(DataFormat::Map, &DudeDatabase::RawDataToMapData);
}

std::vector<DeviceTypeData> DudeDatabase::GetDeviceTypeData() const {
    return GetObjectData<DeviceTypeData>(DataFormat::DeviceType,
                                         &DudeDatabase::RawDataToDeviceTypeData);
}

std::vector<DeviceData> DudeDatabase::GetDeviceData() const {
    return GetObjectData<DeviceData>(DataFormat::Device, &DudeDatabase::RawDataToDeviceData);
}

std::vector<ServiceData> DudeDatabase::GetServiceData() const {
    return GetObjectData<ServiceData>(DataFormat::Service, &DudeDatabase::RawDataToServiceData);
}

std::vector<NotificationData> DudeDatabase::GetNotificationData() const {
    return GetObjectData<NotificationData>(DataFormat::Notification,
                                           &DudeDatabase::RawDataToNotificationData);
}

std::vector<LinkData> DudeDatabase::GetLinkData() const {
    return GetObjectData<LinkData>(DataFormat::Link, &DudeDatabase::RawDataToLinkData);
}

std::vector<LinkTypeData> DudeDatabase::GetLinkTypeData() const {
    return GetObjectData<LinkTypeData>(DataFormat::LinkType, &DudeDatabase::RawDataToLinkTypeData);
}

std::vector<DataSourceData> DudeDatabase::GetDataSourceData() const {
    return GetObjectData<DataSourceData>(DataFormat::DataSource,
                                         &DudeDatabase::RawDataToDataSourceData);
}

std::vector<FunctionData> DudeDatabase::GetFunctionData() const {
    return GetObjectData<FunctionData>(DataFormat::Function, &DudeDatabase::RawDataToFunctionData);
}

std::vector<SnmpProfileData> DudeDatabase::GetSnmpProfileData() const {
    return GetObjectData<SnmpProfileData>(DataFormat::SnmpProfile,
                                          &DudeDatabase::RawDataToSnmpProfileData);
}

std::vector<PanelData> DudeDatabase::GetPanelData() const {
    return GetObjectData<PanelData>(DataFormat::Panel, &DudeDatabase::RawDataToPanelData);
}

std::vector<NetworkMapElementData> DudeDatabase::GetNetworkMapElementData() const {
    return GetObjectData<NetworkMapElementData>(DataFormat::NetworkMapElement,
                                                &DudeDatabase::RawDataToNetworkMapElementData);
}

std::vector<ChartLineData> DudeDatabase::GetChartLineData() const {
    return GetObjectData<ChartLineData>(DataFormat::ChartLine,
                                        &DudeDatabase::RawDataToChartLineData);
}

std::vector<PanelElementData> DudeDatabase::GetPanelElementData() const {
    return GetObjectData<PanelElementData>(DataFormat::PanelElement,
                                           &DudeDatabase::RawDataToPanelElementData);
}

RawObjData DudeDatabase::BlobToRawObjData(std::span<const u8> blob) const {
    constexpr std::size_t header_size = sizeof(RawObjData::magic);
    RawObjData data{};

    if (blob.size() < header_size) {
        printf("Invalid blob size: %zu\n", blob.size());
        return {};
    }

    memcpy(&data, blob.data(), header_size);

    std::size_t offset = header_size;
    SetField(data.data_format, FieldId::DataFormat, blob, offset);

    data.data.resize(blob.size() - offset);
    memcpy(data.data.data(), blob.data() + offset, data.data.size());

    return data;
}

ServerConfigurationData DudeDatabase::RawDataToServerConfigurationData(
    std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    ServerConfigurationData data{};

    is_valid &=
        SetField(data.time_zone_history, FieldId::ServerConfig_TimeZoneHistory, raw_data, offset);
    is_valid &= SetField(data.discover_skip_types, FieldId::ServerConfig_DiscoverSkipTypes,
                         raw_data, offset);
    is_valid &= SetField(data.discover_skip_probes, FieldId::ServerConfig_DiscoverSkipProbes,
                         raw_data, offset);
    is_valid &= SetField(data.custom_colors, FieldId::ServerConfig_CustomColors, raw_data, offset);
    is_valid &=
        SetField(data.chart_line_colors, FieldId::ServerConfig_ChartLineColors, raw_data, offset);
    is_valid &= SetField(data.notify_ids, FieldId::ServerConfig_NotifyIDs, raw_data, offset);
    is_valid &= SetField(data.discover_identification, FieldId::ServerConfig_DiscoverIdentification,
                         raw_data, offset);
    is_valid &=
        SetField(data.discover_networks, FieldId::ServerConfig_DiscoverNetworks, raw_data, offset);
    is_valid &=
        SetField(data.discover_links, FieldId::ServerConfig_DiscoverLinks, raw_data, offset);
    is_valid &=
        SetField(data.map_device_visible, FieldId::ServerConfig_MapDeviceVisible, raw_data, offset);
    is_valid &=
        SetField(data.discover_layer_2, FieldId::ServerConfig_DiscoverLayer2, raw_data, offset);
    is_valid &=
        SetField(data.first_connection, FieldId::ServerConfig_FirstConnection, raw_data, offset);
    is_valid &= SetField(data.discover_ppp, FieldId::ServerConfig_DiscoverPpp, raw_data, offset);
    is_valid &= SetField(data.discover_graph_services, FieldId::ServerConfig_DiscoverGraphServices,
                         raw_data, offset);
    is_valid &= SetField(data.map_network_visible, FieldId::ServerConfig_MapNetworkVisible,
                         raw_data, offset);
    is_valid &= SetField(data.discover_graph_links, FieldId::ServerConfig_DiscoverGraphLinks,
                         raw_data, offset);
    is_valid &= SetField(data.discover_service_less, FieldId::ServerConfig_DiscoverServiceLess,
                         raw_data, offset);
    is_valid &=
        SetField(data.map_submap_visible, FieldId::ServerConfig_MapSubmapVisible, raw_data, offset);
    is_valid &= SetField(data.probe_enabled, FieldId::ServerConfig_ProbeEnabled, raw_data, offset);
    is_valid &=
        SetField(data.map_static_visible, FieldId::ServerConfig_MapStaticVisible, raw_data, offset);
    is_valid &=
        SetField(data.syslog_enabled, FieldId::ServerConfig_SyslogEnabled, raw_data, offset);
    is_valid &=
        SetField(data.map_link_visible, FieldId::ServerConfig_MapLinkVisible, raw_data, offset);
    is_valid &=
        SetField(data.snmp_trap_enabled, FieldId::ServerConfig_SnmpTrapEnabled, raw_data, offset);
    is_valid &=
        SetField(data.confirm_remove, FieldId::ServerConfig_ConfirmRemove, raw_data, offset);
    is_valid &= SetField(data.resolve_mac_address_manufacturer,
                         FieldId::ServerConfig_ResolveMACAddressManufacturer, raw_data, offset);
    is_valid &=
        SetField(data.map_dep_visible, FieldId::ServerConfig_MapDepVisible, raw_data, offset);
    is_valid &= SetField(data.map_antialiased_geometry,
                         FieldId::ServerConfig_MapAntialiasedGeometry, raw_data, offset);
    is_valid &= SetField(data.map_gradients, FieldId::ServerConfig_MapGradients, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.version, FieldId::ServerConfig_Version, raw_data, offset);
    is_valid &=
        SetField(data.snmp_profile_id, FieldId::ServerConfig_SnmpProfileID, raw_data, offset);
    is_valid &= SetField(data.agent_id, FieldId::ServerConfig_AgentID, raw_data, offset);
    is_valid &=
        SetField(data.probe_interval, FieldId::ServerConfig_ProbeInterval, raw_data, offset);
    is_valid &= SetField(data.probe_timeout, FieldId::ServerConfig_ProbeTimeout, raw_data, offset);
    is_valid &=
        SetField(data.probe_down_count, FieldId::ServerConfig_ProbeDownCount, raw_data, offset);
    is_valid &= SetField(data.syslog_port, FieldId::ServerConfig_SyslogPort, raw_data, offset);
    is_valid &= SetField(data.snmp_trap_port, FieldId::ServerConfig_SnmpTrapPort, raw_data, offset);
    is_valid &= SetField(data.map_background_color, FieldId::ServerConfig_MapBackgroundColor,
                         raw_data, offset);
    is_valid &= SetField(data.map_label_refresh_interval,
                         FieldId::ServerConfig_MapLabelRefreshInterval, raw_data, offset);
    is_valid &= SetField(data.map_up_color, FieldId::ServerConfig_MapUpColor, raw_data, offset);
    is_valid &= SetField(data.map_down_partial_color, FieldId::ServerConfig_MapDownPartialColor,
                         raw_data, offset);
    is_valid &= SetField(data.map_down_complete_color, FieldId::ServerConfig_MapDownCompleteColor,
                         raw_data, offset);
    is_valid &=
        SetField(data.map_unknown_color, FieldId::ServerConfig_MapUnknownColor, raw_data, offset);
    is_valid &=
        SetField(data.map_acked_color, FieldId::ServerConfig_MapAckedColor, raw_data, offset);
    is_valid &=
        SetField(data.map_network_color, FieldId::ServerConfig_MapNetworkColor, raw_data, offset);
    is_valid &=
        SetField(data.map_submap_color, FieldId::ServerConfig_MapSubmapColor, raw_data, offset);
    is_valid &= SetField(data.map_submap_up_color, FieldId::ServerConfig_MapSubmapUpColor, raw_data,
                         offset);
    is_valid &= SetField(data.map_submap_down_partial_color,
                         FieldId::ServerConfig_MapSubmapDownPartialColor, raw_data, offset);
    is_valid &= SetField(data.map_submap_down_complete_color,
                         FieldId::ServerConfig_MapSubmapDownCompleteColor, raw_data, offset);
    is_valid &= SetField(data.map_submap_acked_color, FieldId::ServerConfig_MapSubmapAckedColor,
                         raw_data, offset);
    is_valid &=
        SetField(data.map_static_color, FieldId::ServerConfig_MapStaticColor, raw_data, offset);
    is_valid &= SetField(data.map_link_color, FieldId::ServerConfig_MapLinkColor, raw_data, offset);
    is_valid &= SetField(data.map_link_label_color, FieldId::ServerConfig_MapLinkLabelColor,
                         raw_data, offset);
    is_valid &= SetField(data.map_link_full_color, FieldId::ServerConfig_MapLinkFullColor, raw_data,
                         offset);
    is_valid &=
        SetField(data.map_device_shape, FieldId::ServerConfig_MapDeviceShape, raw_data, offset);
    is_valid &=
        SetField(data.map_network_shape, FieldId::ServerConfig_MapNetworkShape, raw_data, offset);
    is_valid &=
        SetField(data.map_submap_shape, FieldId::ServerConfig_MapSubmapShape, raw_data, offset);
    is_valid &=
        SetField(data.map_static_shape, FieldId::ServerConfig_MapStaticShape, raw_data, offset);
    is_valid &=
        SetField(data.map_link_thickness, FieldId::ServerConfig_MapLinkThickness, raw_data, offset);
    is_valid &= SetField(data.map_dep_color, FieldId::ServerConfig_MapDepColor, raw_data, offset);
    is_valid &=
        SetField(data.map_dep_thickness, FieldId::ServerConfig_MapDepThickness, raw_data, offset);
    is_valid &= SetField(data.map_dep_style, FieldId::ServerConfig_MapDepStyle, raw_data, offset);
    is_valid &= SetField(data.chart_value_keep_time_raw,
                         FieldId::ServerConfig_ChartValueKeepTimeRaw, raw_data, offset);
    is_valid &= SetField(data.chart_value_keep_time_10_min,
                         FieldId::ServerConfig_ChartValueKeepTime10min, raw_data, offset);
    is_valid &= SetField(data.chart_value_keep_time_2_hour,
                         FieldId::ServerConfig_ChartValueKeepTime2hour, raw_data, offset);
    is_valid &= SetField(data.chart_value_keep_time_1_day,
                         FieldId::ServerConfig_ChartValueKeepTime1day, raw_data, offset);
    is_valid &= SetField(data.chart_background_color, FieldId::ServerConfig_ChartBackgroundColor,
                         raw_data, offset);
    is_valid &=
        SetField(data.chart_grid_color, FieldId::ServerConfig_ChartGridColor, raw_data, offset);
    is_valid &=
        SetField(data.chart_text_color, FieldId::ServerConfig_ChartTextColor, raw_data, offset);
    is_valid &= SetField(data.discover_name_preference,
                         FieldId::ServerConfig_DiscoverNamePreference, raw_data, offset);
    is_valid &= SetField(data.discover_mode, FieldId::ServerConfig_DiscoverMode, raw_data, offset);
    is_valid &= SetField(data.discover_hops, FieldId::ServerConfig_DiscoverHops, raw_data, offset);
    is_valid &= SetField(data.discover_hop_network_size_limit,
                         FieldId::ServerConfig_DiscoverHopNetworkSizeLimit, raw_data, offset);
    is_valid &= SetField(data.discover_simultaneous, FieldId::ServerConfig_DiscoverSimultaneous,
                         raw_data, offset);
    is_valid &=
        SetField(data.discover_interval, FieldId::ServerConfig_DiscoverInterval, raw_data, offset);
    is_valid &= SetField(data.discover_item_width, FieldId::ServerConfig_DiscoverItemWidth,
                         raw_data, offset);
    is_valid &= SetField(data.discover_item_height, FieldId::ServerConfig_DiscoverItemHeight,
                         raw_data, offset);
    is_valid &=
        SetField(data.discover_big_row, FieldId::ServerConfig_DiscoverBigRow, raw_data, offset);
    is_valid &= SetField(data.discover_big_column, FieldId::ServerConfig_DiscoverBigColumn,
                         raw_data, offset);
    is_valid &=
        SetField(data.discover_whole_row, FieldId::ServerConfig_DiscoverWholeRow, raw_data, offset);
    is_valid &= SetField(data.discover_whole_column, FieldId::ServerConfig_DiscoverWholeColumn,
                         raw_data, offset);
    is_valid &=
        SetField(data.ros_conn_interval, FieldId::ServerConfig_RosConnInterval, raw_data, offset);
    is_valid &= SetField(data.ros_conn_interval_auth_failed,
                         FieldId::ServerConfig_RosConnIntervalAuthFailed, raw_data, offset);
    is_valid &=
        SetField(data.undo_queue_size, FieldId::ServerConfig_UndoQueueSize, raw_data, offset);
    is_valid &= SetField(data.mac_mapping_refresh_interval,
                         FieldId::ServerConfig_MacMappingRefreshInterval, raw_data, offset);
    is_valid &= SetField(data.contents_pane_behavior, FieldId::ServerConfig_ContentsPaneBehavior,
                         raw_data, offset);
    is_valid &= SetField(data.last_chart_maintenance_time,
                         FieldId::ServerConfig_LastChartMaintenanceTime, raw_data, offset);
    is_valid &= SetField(data.discover_black_list, FieldId::ServerConfig_DiscoverBlackList,
                         raw_data, offset);
    is_valid &= SetField(data.report_font, FieldId::ServerConfig_ReportFont, raw_data, offset);
    is_valid &= SetField(data.chart_font, FieldId::ServerConfig_ChartFont, raw_data, offset);
    is_valid &= SetField(data.map_link_font, FieldId::ServerConfig_MapLinkFont, raw_data, offset);
    is_valid &=
        SetField(data.map_link_tooltip, FieldId::ServerConfig_MapLinkTooltip, raw_data, offset);
    is_valid &= SetField(data.map_link_label, FieldId::ServerConfig_MapLinkLabel, raw_data, offset);
    is_valid &=
        SetField(data.map_static_font, FieldId::ServerConfig_MapStaticFont, raw_data, offset);
    is_valid &=
        SetField(data.map_submap_font, FieldId::ServerConfig_MapSubmapFont, raw_data, offset);
    is_valid &=
        SetField(data.map_submap_tooltip, FieldId::ServerConfig_MapSubmapTooltip, raw_data, offset);
    is_valid &=
        SetField(data.map_submap_label, FieldId::ServerConfig_MapSubmapLabel, raw_data, offset);
    is_valid &=
        SetField(data.map_network_font, FieldId::ServerConfig_MapNetworkFont, raw_data, offset);
    is_valid &= SetField(data.map_network_tooltip, FieldId::ServerConfig_MapNetworkTooltip,
                         raw_data, offset);
    is_valid &=
        SetField(data.map_network_label, FieldId::ServerConfig_MapNetworkLabel, raw_data, offset);
    is_valid &=
        SetField(data.map_device_font, FieldId::ServerConfig_MapDeviceFont, raw_data, offset);
    is_valid &=
        SetField(data.map_device_tooltip, FieldId::ServerConfig_MapDeviceTooltip, raw_data, offset);
    is_valid &=
        SetField(data.map_device_label, FieldId::ServerConfig_MapDeviceLabel, raw_data, offset);
    is_valid &= SetField(data.unique_id, FieldId::ServerConfig_UniqueID, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

ToolData DudeDatabase::RawDataToToolData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    ToolData data{};

    is_valid &= SetField(data.builtin, FieldId::Tool_Builtin, raw_data, offset);
    is_valid &= SetField(data.type, FieldId::Tool_Type, raw_data, offset);
    is_valid &= SetField(data.device_id, FieldId::Tool_DeviceID, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.command, FieldId::Tool_Command, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

FileData DudeDatabase::RawDataToFileData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    FileData data{};

    is_valid &= SetField(data.parent_id, FieldId::File_ParentID, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.file_name, FieldId::File_FileName, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

NotesData DudeDatabase::RawDataToNotesData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    NotesData data{};

    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.parent_id, FieldId::Note_ObjID, raw_data, offset);
    is_valid &= SetField(data.time_added, FieldId::Note_TimeAdded, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

MapData DudeDatabase::RawDataToMapData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    MapData data{};

    is_valid &= SetField(data.notify_ids, FieldId::NetworkMap_NotifyIDs, raw_data, offset);
    is_valid &=
        SetField(data.use_static_color, FieldId::NetworkMap_UseStaticColor, raw_data, offset);
    is_valid &= SetField(data.use_link_color, FieldId::NetworkMap_UseLinkColor, raw_data, offset);
    is_valid &= SetField(data.use_link_label_color, FieldId::NetworkMap_UseLinkLabelColor, raw_data,
                         offset);
    is_valid &=
        SetField(data.use_link_full_color, FieldId::NetworkMap_UseLinkFullColor, raw_data, offset);
    is_valid &=
        SetField(data.use_device_label, FieldId::NetworkMap_UseDeviceLabel, raw_data, offset);
    is_valid &=
        SetField(data.use_device_shape, FieldId::NetworkMap_UseDeviceShape, raw_data, offset);
    is_valid &= SetField(data.use_device_font, FieldId::NetworkMap_UseDeviceFont, raw_data, offset);
    is_valid &=
        SetField(data.use_network_label, FieldId::NetworkMap_UseNetworkLabel, raw_data, offset);
    is_valid &=
        SetField(data.use_network_shape, FieldId::NetworkMap_UseNetworkShape, raw_data, offset);
    is_valid &=
        SetField(data.use_network_font, FieldId::NetworkMap_UseNetworkFont, raw_data, offset);
    is_valid &=
        SetField(data.use_submap_label, FieldId::NetworkMap_UseSubmapLabel, raw_data, offset);
    is_valid &=
        SetField(data.use_submap_shape, FieldId::NetworkMap_UseSubmapShape, raw_data, offset);
    is_valid &= SetField(data.use_submap_font, FieldId::NetworkMap_UseSubmapFont, raw_data, offset);
    is_valid &=
        SetField(data.use_static_shape, FieldId::NetworkMap_UseStaticShape, raw_data, offset);
    is_valid &= SetField(data.use_static_font, FieldId::NetworkMap_UseStaticFont, raw_data, offset);
    is_valid &= SetField(data.use_link_label, FieldId::NetworkMap_UseLinkLabel, raw_data, offset);
    is_valid &= SetField(data.use_link_font, FieldId::NetworkMap_UseLinkFont, raw_data, offset);
    is_valid &=
        SetField(data.use_link_thickness, FieldId::NetworkMap_UseLinkThickness, raw_data, offset);
    is_valid &= SetField(data.ordered, FieldId::ObjectList_Ordered, raw_data, offset);
    is_valid &= SetField(data.prove_enabled, FieldId::NetworkMap_ProbeEnabled, raw_data, offset);
    is_valid &= SetField(data.notify_use, FieldId::NetworkMap_NotifyUse, raw_data, offset);
    is_valid &=
        SetField(data.report_scanning, FieldId::NetworkMap_ReportScanning, raw_data, offset);
    is_valid &= SetField(data.locked, FieldId::NetworkMap_Locked, raw_data, offset);
    is_valid &= SetField(data.image_tile, FieldId::NetworkMap_ImageTile, raw_data, offset);
    is_valid &= SetField(data.color_visible, FieldId::NetworkMap_ColorVisible, raw_data, offset);
    is_valid &= SetField(data.device_visible, FieldId::NetworkMap_DeviceVisible, raw_data, offset);
    is_valid &=
        SetField(data.network_visible, FieldId::NetworkMap_NetworkVisible, raw_data, offset);
    is_valid &= SetField(data.submap_visible, FieldId::NetworkMap_SubmapVisible, raw_data, offset);
    is_valid &= SetField(data.static_visible, FieldId::NetworkMap_StaticVisible, raw_data, offset);
    is_valid &= SetField(data.link_visible, FieldId::NetworkMap_LinkVisible, raw_data, offset);
    is_valid &= SetField(data.use_background_color, FieldId::NetworkMap_UseBackgroundColor,
                         raw_data, offset);
    is_valid &= SetField(data.use_up_color, FieldId::NetworkMap_UseUpColor, raw_data, offset);
    is_valid &= SetField(data.use_down_partial_color, FieldId::NetworkMap_UseDownPartialColor,
                         raw_data, offset);
    is_valid &= SetField(data.use_down_complete_color, FieldId::NetworkMap_UseDownCompleteColor,
                         raw_data, offset);
    is_valid &=
        SetField(data.use_unknown_color, FieldId::NetworkMap_UseUnknownColor, raw_data, offset);
    is_valid &= SetField(data.use_acked_color, FieldId::NetworkMap_UseAckedColor, raw_data, offset);
    is_valid &=
        SetField(data.use_network_color, FieldId::NetworkMap_UseNetworkColor, raw_data, offset);
    is_valid &=
        SetField(data.use_submap_color, FieldId::NetworkMap_UseSubmapColor, raw_data, offset);
    is_valid &=
        SetField(data.use_submap_up_color, FieldId::NetworkMap_UseSubmapUpColor, raw_data, offset);
    is_valid &= SetField(data.use_submap_down_partial_color,
                         FieldId::NetworkMap_UseSubmapDownPartialColor, raw_data, offset);
    is_valid &= SetField(data.use_submap_down_complete_color,
                         FieldId::NetworkMap_UseSubmapDownCompleteColor, raw_data, offset);
    is_valid &= SetField(data.use_submap_acked_color, FieldId::NetworkMap_UseSubmapAckedColor,
                         raw_data, offset);
    is_valid &= SetField(data.link_thickness, FieldId::NetworkMap_LinkThickness, raw_data, offset);
    is_valid &= SetField(data.layout_density, FieldId::NetworkMap_LayoutDensity, raw_data, offset);
    is_valid &= SetField(data.layout_quality, FieldId::NetworkMap_LayoutQuality, raw_data, offset);
    is_valid &= SetField(data.prove_interval, FieldId::NetworkMap_ProbeInterval, raw_data, offset);
    is_valid &= SetField(data.prove_timeout, FieldId::NetworkMap_ProbeTimeout, raw_data, offset);
    is_valid &=
        SetField(data.prove_down_count, FieldId::NetworkMap_ProbeDownCount, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.default_zoom, FieldId::NetworkMap_DefaultZoom, raw_data, offset);
    is_valid &= SetField(data.image_id, FieldId::NetworkMap_ImageID, raw_data, offset);
    is_valid &= SetField(data.image_scale, FieldId::NetworkMap_ImageScale, raw_data, offset);
    is_valid &= SetField(data.label_refresh_interval, FieldId::NetworkMap_LabelRefreshInterval,
                         raw_data, offset);
    is_valid &=
        SetField(data.background_color, FieldId::NetworkMap_BackgroundColor, raw_data, offset);
    is_valid &= SetField(data.up_color, FieldId::NetworkMap_UpColor, raw_data, offset);
    is_valid &=
        SetField(data.down_partial_color, FieldId::NetworkMap_DownPartialColor, raw_data, offset);
    is_valid &=
        SetField(data.down_complete_color, FieldId::NetworkMap_DownCompleteColor, raw_data, offset);
    is_valid &= SetField(data.unknown_color, FieldId::NetworkMap_UnknownColor, raw_data, offset);
    is_valid &= SetField(data.acked_color, FieldId::NetworkMap_AckedColor, raw_data, offset);
    is_valid &= SetField(data.network_color, FieldId::NetworkMap_NetworkColor, raw_data, offset);
    is_valid &= SetField(data.submap_color, FieldId::NetworkMap_SubmapColor, raw_data, offset);
    is_valid &= SetField(data.submap_up_color, FieldId::NetworkMap_SubmapUpColor, raw_data, offset);
    is_valid &= SetField(data.submap_down_partial_color, FieldId::NetworkMap_SubmapDownPartialColor,
                         raw_data, offset);
    is_valid &= SetField(data.submap_down_complete_color,
                         FieldId::NetworkMap_SubmapDownCompleteColor, raw_data, offset);
    is_valid &=
        SetField(data.submap_acked_color, FieldId::NetworkMap_SubmapAckedColor, raw_data, offset);
    is_valid &= SetField(data.static_color, FieldId::NetworkMap_StaticColor, raw_data, offset);
    is_valid &= SetField(data.link_color, FieldId::NetworkMap_LinkColor, raw_data, offset);
    is_valid &=
        SetField(data.link_label_color, FieldId::NetworkMap_LinkLabelColor, raw_data, offset);
    is_valid &= SetField(data.link_full_color, FieldId::NetworkMap_LinkFullColor, raw_data, offset);
    is_valid &= SetField(data.device_shape, FieldId::NetworkMap_DeviceShape, raw_data, offset);
    is_valid &= SetField(data.network_shape, FieldId::NetworkMap_NetworkShape, raw_data, offset);
    is_valid &= SetField(data.submap_shape, FieldId::NetworkMap_SubmapShape, raw_data, offset);
    is_valid &= SetField(data.static_shape, FieldId::NetworkMap_StaticShape, raw_data, offset);
    is_valid &= SetField(data.link_font, FieldId::NetworkMap_LinkFont, raw_data, offset);
    is_valid &= SetField(data.link_label, FieldId::NetworkMap_LinkLabel, raw_data, offset);
    is_valid &= SetField(data.static_font, FieldId::NetworkMap_StaticFont, raw_data, offset);
    is_valid &= SetField(data.submap_font, FieldId::NetworkMap_SubmapFont, raw_data, offset);
    is_valid &= SetField(data.submap_label, FieldId::NetworkMap_SubmapLabel, raw_data, offset);
    is_valid &= SetField(data.network_font, FieldId::NetworkMap_NetworkFont, raw_data, offset);
    is_valid &= SetField(data.network_label, FieldId::NetworkMap_NetworkLabel, raw_data, offset);
    is_valid &= SetField(data.device_font, FieldId::NetworkMap_DeviceFont, raw_data, offset);
    is_valid &= SetField(data.device_label, FieldId::NetworkMap_DeviceLabel, raw_data, offset);
    is_valid &= SetField(data.list_type, FieldId::ObjectList_Type, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

DeviceTypeData DudeDatabase::RawDataToDeviceTypeData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    DeviceTypeData data{};

    is_valid &=
        SetField(data.ignored_services, FieldId::DeviceType_IgnoredServices, raw_data, offset);
    is_valid &=
        SetField(data.allowed_services, FieldId::DeviceType_AllowedServices, raw_data, offset);
    is_valid &=
        SetField(data.required_services, FieldId::DeviceType_RequiredServices, raw_data, offset);
    is_valid &= SetField(data.image_id, FieldId::DeviceType_ImageId, raw_data, offset);
    is_valid &= SetField(data.image_scale, FieldId::DeviceType_ImageScale, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.next_id, FieldId::SysNextId, raw_data, offset);
    is_valid &= SetField(data.url, FieldId::DeviceType_Url, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

DeviceData DudeDatabase::RawDataToDeviceData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    DeviceData data{};

    is_valid &= SetField(data.parent_ids, FieldId::Device_ParentIds, raw_data, offset);
    is_valid &= SetField(data.notify_ids, FieldId::Device_NotifyIds, raw_data, offset);
    is_valid &= SetField(data.dns_names, FieldId::Device_DnsNames, raw_data, offset);
    is_valid &= SetField(data.ip, FieldId::Device_IpAddress, raw_data, offset);
    is_valid &= SetField(data.secure_mode, FieldId::Device_SecureMode, raw_data, offset);
    is_valid &= SetField(data.router_os, FieldId::Device_RouterOs, raw_data, offset);
    is_valid &= SetField(data.dude_server, FieldId::Device_DudeServer, raw_data, offset);
    is_valid &= SetField(data.notify_use, FieldId::Device_NotifyUse, raw_data, offset);
    is_valid &= SetField(data.prove_enabled, FieldId::Device_ProveEnabled, raw_data, offset);
    is_valid &= SetField(data.lookup, FieldId::Device_Lookup, raw_data, offset);
    is_valid &=
        SetField(data.dns_lookup_interval, FieldId::Device_LookupInterval, raw_data, offset);
    is_valid &= SetField(data.mac_lookup, FieldId::Device_MacLookup, raw_data, offset);
    is_valid &= SetField(data.type_id, FieldId::Device_TypeId, raw_data, offset);
    is_valid &= SetField(data.agent_id, FieldId::Device_AgentId, raw_data, offset);
    is_valid &= SetField(data.snmp_profile_id, FieldId::Device_SnmpProfileId, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.prove_interval, FieldId::Device_ProveInterval, raw_data, offset);
    is_valid &= SetField(data.prove_timeout, FieldId::Device_ProveTimeout, raw_data, offset);
    is_valid &= SetField(data.prove_down_count, FieldId::Device_ProveDownCount, raw_data, offset);
    is_valid &= SetField(data.custom_field_3, FieldId::Device_CustomField3, raw_data, offset);
    is_valid &= SetField(data.custom_field_2, FieldId::Device_CustomField2, raw_data, offset);
    is_valid &= SetField(data.custom_field_1, FieldId::Device_CustomField1, raw_data, offset);
    is_valid &= SetField(data.password, FieldId::Device_Password, raw_data, offset);
    is_valid &= SetField(data.username, FieldId::Device_Username, raw_data, offset);
    is_valid &= SetField(data.mac, FieldId::Device_MacAddress, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

ServiceData DudeDatabase::RawDataToServiceData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    ServiceData data{};

    is_valid &= SetField(data.notify_ids, FieldId::Service_NotifyIDs, raw_data, offset);
    is_valid &= SetField(data.enabled, FieldId::Service_Enabled, raw_data, offset);
    is_valid &= SetField(data.history, FieldId::Service_History, raw_data, offset);
    is_valid &= SetField(data.notify_use, FieldId::Service_NotifyUse, raw_data, offset);
    is_valid &= SetField(data.acked, FieldId::Service_Acked, raw_data, offset);
    is_valid &= SetField(data.probe_port, FieldId::Service_ProbePort, raw_data, offset);
    is_valid &= SetField(data.probe_interval, FieldId::Service_ProbeInterval, raw_data, offset);
    is_valid &= SetField(data.probe_timeout, FieldId::Service_ProbeTimeout, raw_data, offset);
    is_valid &= SetField(data.probe_down_count, FieldId::Service_ProbeDownCount, raw_data, offset);
    is_valid &= SetField(data.data_source_id, FieldId::Service_DataSourceID, raw_data, offset);
    is_valid &= SetField(data.status, FieldId::Service_Status, raw_data, offset);
    is_valid &=
        SetField(data.time_since_changed, FieldId::Service_TimeSinceChanged, raw_data, offset);
    is_valid &= SetField(data.time_since_last_up, FieldId::Service_TimeLastUp, raw_data, offset);
    is_valid &=
        SetField(data.time_since_last_down, FieldId::Service_TimeLastDown, raw_data, offset);
    is_valid &= SetField(data.time_previous_up, FieldId::Service_TimePrevUp, raw_data, offset);
    is_valid &= SetField(data.time_previous_down, FieldId::Service_TimePrevDown, raw_data, offset);
    is_valid &= SetField(data.proves_down, FieldId::Service_ProbesDown, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.device_id, FieldId::Service_DeviceID, raw_data, offset);
    is_valid &= SetField(data.agent_id, FieldId::Service_AgentID, raw_data, offset);
    is_valid &= SetField(data.prove_id, FieldId::Service_probeID, raw_data, offset);
    is_valid &= SetField(data.value, FieldId::Service_Value, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

NotificationData DudeDatabase::RawDataToNotificationData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    NotificationData data{};

    is_valid &= SetField(data.status_list, FieldId::Notification_StatusList, raw_data, offset);
    is_valid &=
        SetField(data.group_notify_ids, FieldId::Notification_GroupNotifyIDs, raw_data, offset);
    is_valid &= SetField(data.mail_cc, FieldId::Notification_MailCc, raw_data, offset);
    is_valid &= SetField(data.activity, FieldId::Notification_Activity, raw_data, offset);
    is_valid &= SetField(data.log_use_color, FieldId::Notification_LogUseColor, raw_data, offset);
    is_valid &= SetField(data.enabled, FieldId::Notification_Enabled, raw_data, offset);
    is_valid &= SetField(data.mail_tls_mode, FieldId::Notification_MailTlsMode, raw_data, offset);
    is_valid &= SetField(data.sys_log_server, FieldId::Notification_SyslogServer, raw_data, offset);
    is_valid &= SetField(data.sys_log_port, FieldId::Notification_SyslogPort, raw_data, offset);
    is_valid &= SetField(data.sound_file_id, FieldId::Notification_SoundFileID, raw_data, offset);
    is_valid &= SetField(data.log_color, FieldId::Notification_LogColor, raw_data, offset);
    is_valid &= SetField(data.speak_rate, FieldId::Notification_SpeakRate, raw_data, offset);
    is_valid &= SetField(data.speak_volume, FieldId::Notification_SpeakVolume, raw_data, offset);
    is_valid &=
        SetField(data.delay_interval, FieldId::Notification_DelayInterval, raw_data, offset);
    is_valid &=
        SetField(data.repeat_interval, FieldId::Notification_RepeatInterval, raw_data, offset);
    is_valid &= SetField(data.repeat_count, FieldId::Notification_RepeatCount, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.rype_id, FieldId::Notification_RypeID, raw_data, offset);
    is_valid &= SetField(data.mail_server, FieldId::Notification_MailServer, raw_data, offset);
    is_valid &= SetField(data.mail_port, FieldId::Notification_MailPort, raw_data, offset);
    is_valid &= SetField(data.log_prefix, FieldId::Notification_LogPrefix, raw_data, offset);
    is_valid &= SetField(data.mail_subject, FieldId::Notification_MailSubject, raw_data, offset);
    is_valid &= SetField(data.mail_to, FieldId::Notification_MailTo, raw_data, offset);
    is_valid &= SetField(data.mail_from, FieldId::Notification_MailFrom, raw_data, offset);
    is_valid &= SetField(data.mail_password, FieldId::Notification_MailPassword, raw_data, offset);
    is_valid &= SetField(data.mail_user, FieldId::Notification_MailUser, raw_data, offset);
    is_valid &=
        SetField(data.mail_server_dns, FieldId::Notification_MailServerDns, raw_data, offset);
    is_valid &= SetField(data.mail_server6, FieldId::Notification_MailServer6, raw_data, offset);
    is_valid &= SetField(data.text_template, FieldId::Notification_TextTemplate, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

LinkData DudeDatabase::RawDataToLinkData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    LinkData data{};

    is_valid &= SetField(data.history, FieldId::Link_History, raw_data, offset);
    is_valid &= SetField(data.mastering_type, FieldId::Link_MasteringType, raw_data, offset);
    is_valid &= SetField(data.master_device, FieldId::Link_MasterDevice, raw_data, offset);
    is_valid &= SetField(data.master_interface, FieldId::Link_MasterInterface, raw_data, offset);
    is_valid &= SetField(data.net_map_id, FieldId::Link_NetMapID, raw_data, offset);
    is_valid &= SetField(data.net_map_element_id, FieldId::Link_NetMapElementID, raw_data, offset);
    is_valid &= SetField(data.type_id, FieldId::Link_TypeID, raw_data, offset);
    is_valid &= SetField(data.tx_data_source_id, FieldId::Link_TxDataSourceID, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.rx_data_source_id, FieldId::Link_RxDataSourceID, raw_data, offset);
    is_valid &= SetField(data.speed, FieldId::Link_Speed, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

LinkTypeData DudeDatabase::RawDataToLinkTypeData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    LinkTypeData data{};

    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.style, FieldId::LinkType_Style, raw_data, offset);
    is_valid &= SetField(data.thickness, FieldId::LinkType_Thickness, raw_data, offset);
    is_valid &= SetField(data.snmp_type, FieldId::LinkType_SnmpType, raw_data, offset);
    is_valid &= SetField(data.next_id, FieldId::SysNextId, raw_data, offset);
    is_valid &= SetField(data.snmp_speed, FieldId::LinkType_SnmpSpeed, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

DataSourceData DudeDatabase::RawDataToDataSourceData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    DataSourceData data{};

    is_valid &= SetField(data.enabled, FieldId::DataSource_Enabled, raw_data, offset);
    is_valid &=
        SetField(data.function_device_id, FieldId::DataSource_FunctionDevice, raw_data, offset);
    is_valid &=
        SetField(data.function_interval, FieldId::DataSource_FunctionInterval, raw_data, offset);
    is_valid &= SetField(data.data_source_type, FieldId::DataSource_Type, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.keep_time_raw, FieldId::DataSource_KeepTimeRaw, raw_data, offset);
    is_valid &= SetField(data.keep_time_10min, FieldId::DataSource_KeepTime10min, raw_data, offset);
    is_valid &= SetField(data.keep_time_2hour, FieldId::DataSource_KeepTime2hour, raw_data, offset);
    is_valid &= SetField(data.keep_time_1Day, FieldId::DataSource_KeepTime1day, raw_data, offset);
    is_valid &= SetField(data.function_code, FieldId::DataSource_FunctionCode, raw_data, offset);
    is_valid &= SetField(data.unit, FieldId::DataSource_Unit, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

FunctionData DudeDatabase::RawDataToFunctionData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    FunctionData data{};

    is_valid &=
        SetField(data.argument_descriptors, FieldId::Function_ArgumentDescrs, raw_data, offset);
    is_valid &= SetField(data.builtin, FieldId::Function_Builtin, raw_data, offset);
    is_valid &= SetField(data.min_arguments, FieldId::Function_MinArguments, raw_data, offset);
    is_valid &= SetField(data.max_arguments, FieldId::Function_MaxArguments, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.description, FieldId::Function_Descr, raw_data, offset);
    is_valid &= SetField(data.code, FieldId::Function_Code, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

SnmpProfileData DudeDatabase::RawDataToSnmpProfileData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    SnmpProfileData data{};

    is_valid &= SetField(data.version, FieldId::SnmpProfile_Version, raw_data, offset);
    is_valid &= SetField(data.port, FieldId::SnmpProfile_Port, raw_data, offset);
    is_valid &= SetField(data.security, FieldId::SnmpProfile_V3Security, raw_data, offset);
    is_valid &= SetField(data.auth_method, FieldId::SnmpProfile_V3AuthMethod, raw_data, offset);
    is_valid &= SetField(data.crypth_method, FieldId::SnmpProfile_V3CryptMethod, raw_data, offset);
    is_valid &= SetField(data.try_count, FieldId::SnmpProfile_TryCount, raw_data, offset);
    is_valid &= SetField(data.try_timeout, FieldId::SnmpProfile_TryTimeout, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &=
        SetField(data.crypt_password, FieldId::SnmpProfile_V3CryptPassword, raw_data, offset);
    is_valid &= SetField(data.auth_password, FieldId::SnmpProfile_V3AuthPassword, raw_data, offset);
    is_valid &= SetField(data.community, FieldId::SnmpProfile_Community, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

PanelData DudeDatabase::RawDataToPanelData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    PanelData data{};

    is_valid &= SetField(data.ordered, FieldId::ObjectList_Ordered, raw_data, offset);
    is_valid &= SetField(data.locked, FieldId::Panel_Locked, raw_data, offset);
    is_valid &= SetField(data.title_bars, FieldId::Panel_TitleBars, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.top_element_id, FieldId::Panel_TopElementID, raw_data, offset);
    is_valid &= SetField(data.admin, FieldId::Panel_Admin, raw_data, offset);
    is_valid &= SetField(data.type, FieldId::ObjectList_Type, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

NetworkMapElementData DudeDatabase::RawDataToNetworkMapElementData(
    std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    NetworkMapElementData data{};

    is_valid &= SetField(data.item_use_acked_color, FieldId::NetworkMapElement_ItemUseAckedColor,
                         raw_data, offset);
    is_valid &=
        SetField(data.item_use_label, FieldId::NetworkMapElement_ItemUseLabel, raw_data, offset);
    is_valid &=
        SetField(data.item_use_shapes, FieldId::NetworkMapElement_ItemUseShape, raw_data, offset);
    is_valid &=
        SetField(data.item_use_font, FieldId::NetworkMapElement_ItemUseFont, raw_data, offset);
    is_valid &=
        SetField(data.item_use_image, FieldId::NetworkMapElement_ItemUseImage, raw_data, offset);
    is_valid &= SetField(data.item_use_image_scale, FieldId::NetworkMapElement_ItemUseImageScale,
                         raw_data, offset);
    is_valid &=
        SetField(data.item_use_width, FieldId::NetworkMapElement_LinkUseWidth, raw_data, offset);
    is_valid &= SetField(data.item_use_up_color, FieldId::NetworkMapElement_ItemUseUpColor,
                         raw_data, offset);
    is_valid &= SetField(data.item_use_down_partial_color,
                         FieldId::NetworkMapElement_ItemUseDownPartialColor, raw_data, offset);
    is_valid &= SetField(data.item_use_down_complete_color,
                         FieldId::NetworkMapElement_ItemUseDownCompleteColor, raw_data, offset);
    is_valid &= SetField(data.item_use_unknown_color,
                         FieldId::NetworkMapElement_ItemUseUnknownColor, raw_data, offset);
    is_valid &=
        SetField(data.item_up_color, FieldId::NetworkMapElement_ItemUpColor, raw_data, offset);
    is_valid &= SetField(data.item_down_partial_color,
                         FieldId::NetworkMapElement_ItemDownPartialColor, raw_data, offset);
    is_valid &= SetField(data.item_down_complete_color,
                         FieldId::NetworkMapElement_ItemDownCompleteColor, raw_data, offset);
    is_valid &= SetField(data.item_unknown_color, FieldId::NetworkMapElement_ItemUnknownColor,
                         raw_data, offset);
    is_valid &= SetField(data.item_acked_color, FieldId::NetworkMapElement_ItemAckedColor, raw_data,
                         offset);
    is_valid &= SetField(data.item_shape, FieldId::NetworkMapElement_ItemShape, raw_data, offset);
    is_valid &= SetField(data.item_image, FieldId::NetworkMapElement_ItemImage, raw_data, offset);
    is_valid &= SetField(data.item_image_scale, FieldId::NetworkMapElement_ItemImageScale, raw_data,
                         offset);
    is_valid &= SetField(data.link_from, FieldId::NetworkMapElement_LinkFrom, raw_data, offset);
    is_valid &= SetField(data.link_to, FieldId::NetworkMapElement_LinkTo, raw_data, offset);
    is_valid &= SetField(data.link_id, FieldId::NetworkMapElement_LinkID, raw_data, offset);
    is_valid &= SetField(data.link_width, FieldId::NetworkMapElement_LinkWidth, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.map_id, FieldId::NetworkMapElement_MapID, raw_data, offset);
    is_valid &= SetField(data.type, FieldId::NetworkMapElement_Type, raw_data, offset);
    is_valid &= SetField(data.item_type, FieldId::NetworkMapElement_ItemType, raw_data, offset);
    is_valid &= SetField(data.item_id, FieldId::NetworkMapElement_ItemID, raw_data, offset);
    is_valid &= SetField(data.item_x, FieldId::NetworkMapElement_ItemX, raw_data, offset);
    is_valid &= SetField(data.item_y, FieldId::NetworkMapElement_ItemY, raw_data, offset);
    is_valid &= SetField(data.label_refresh_interval,
                         FieldId::NetworkMapElement_LabelRefreshInterval, raw_data, offset);
    is_valid &= SetField(data.item_font, FieldId::NetworkMapElement_ItemFont, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

ChartLineData DudeDatabase::RawDataToChartLineData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    ChartLineData data{};

    is_valid &= SetField(data.chart_id, FieldId::ChartLine_ChartID, raw_data, offset);
    is_valid &= SetField(data.source_id, FieldId::ChartLine_SourceID, raw_data, offset);
    is_valid &= SetField(data.line_style, FieldId::ChartLine_LineStyle, raw_data, offset);
    is_valid &= SetField(data.line_color, FieldId::ChartLine_LineColor, raw_data, offset);
    is_valid &= SetField(data.line_opacity, FieldId::ChartLine_LineOpacity, raw_data, offset);
    is_valid &= SetField(data.fill_color, FieldId::ChartLine_FillColor, raw_data, offset);
    is_valid &= SetField(data.fill_opacity, FieldId::ChartLine_FillOpacity, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.next_id, FieldId::SysNextId, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

PanelElementData DudeDatabase::RawDataToPanelElementData(std::span<const u8> raw_data) const {
    std::size_t offset = 0;
    bool is_valid = true;
    PanelElementData data{};

    is_valid &= SetField(data.split, FieldId::PanelElement_Split, raw_data, offset);
    is_valid &= SetField(data.panel_id, FieldId::PanelElement_PanelID, raw_data, offset);
    is_valid &= SetField(data.split_type, FieldId::PanelElement_SplitType, raw_data, offset);
    is_valid &= SetField(data.split_share, FieldId::PanelElement_SplitShare, raw_data, offset);
    is_valid &= SetField(data.first_id, FieldId::PanelElement_FirstID, raw_data, offset);
    is_valid &= SetField(data.second_id, FieldId::PanelElement_SecondID, raw_data, offset);
    is_valid &= SetField(data.obj_id, FieldId::PanelElement_ObjID, raw_data, offset);
    is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
    is_valid &= SetField(data.obj_meta, FieldId::PanelElement_ObjMeta, raw_data, offset);
    is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
    is_valid &= ValidateEndOfBlob(raw_data, offset);

    if (!is_valid) {
        return {};
    }

    return data;
}

bool DudeDatabase::SetField(BoolField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(BoolField::info);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    switch (field.info.type) {
    case FieldType::BoolFalse:
        field.value = false;
        break;
    case FieldType::BoolTrue:
        field.value = true;
        break;
    default:
        printf("Invalid data type, expected 0/1, found %u\n", field.info.type.Value());
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(ByteField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(ByteField::info) + sizeof(ByteField::value);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::Byte)) {
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(IntField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(IntField::info);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        return {};
    }

    switch (field.info.type.Value()) {
    case FieldType::Int:
        memcpy(&field.value, raw_data.data() + offset, sizeof(IntField::value));
        offset += sizeof(IntField::value);
        break;
    case FieldType::Byte:
        memcpy(&field.value, raw_data.data() + offset, sizeof(ByteField::value));
        offset += sizeof(ByteField::value);
        break;
    default:
        printf("Invalid data type, expected 8/9, found %u\n", field.info.type.Value());
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(TimeField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(TimeField::info) + sizeof(TimeField::date);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::Int)) {
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(LongField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(LongField::info) + sizeof(LongField::value);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::Long)) {
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(LongLongField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(LongLongField::info) + sizeof(LongLongField::value);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::LongLong)) {
        return {};
    }

    return true;
}

bool DudeDatabase::SetField(TextField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size = sizeof(TextField::info);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    switch (field.info.type.Value()) {
    case FieldType::ShortString:
        memcpy(&field.data_size, raw_data.data() + offset, sizeof(u8));
        offset += sizeof(u8);
        break;
    case FieldType::LongString:
        memcpy(&field.data_size, raw_data.data() + offset, sizeof(u16));
        offset += sizeof(u16);
        break;
    default:
        printf("Invalid data type, expected 32/33, found %u\n", field.info.type.Value());
        return {};
    }

    if (!CheckSize(raw_data.size(), offset, field.data_size)) {
        return {};
    }

    std::vector<char> raw_text(field.data_size);
    memcpy(raw_text.data(), raw_data.data() + offset, field.data_size);
    field.text = std::string(raw_text.begin(), raw_text.end());
    offset += field.data_size;

    return true;
}

bool DudeDatabase::SetField(IntArrayField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size =
        sizeof(IntArrayField::info) + sizeof(IntArrayField::entries);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::IntArray)) {
        return {};
    }

    if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
        return {};
    }

    field.data.resize(field.entries);
    memcpy(field.data.data(), raw_data.data() + offset, field.entries * sizeof(u32));
    offset += field.entries * sizeof(u32);

    return true;
}

bool DudeDatabase::SetField(IpAddressField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size =
        sizeof(IpAddressField::info) + sizeof(IpAddressField::entries);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::IntArray)) {
        return {};
    }

    if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
        return {};
    }

    field.ip_address.resize(field.entries);
    memcpy(field.ip_address.data(), raw_data.data() + offset, field.entries * sizeof(u32));
    offset += field.entries * sizeof(u32);

    return true;
}

bool DudeDatabase::SetField(LongArrayField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size =
        sizeof(LongArrayField::info) + sizeof(LongArrayField::data_size);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::LongArray)) {
        return {};
    }

    if (!CheckSize(raw_data.size(), offset, field.data_size)) {
        return {};
    }

    field.data.resize(field.data_size);
    memcpy(field.data.data(), raw_data.data() + offset, field.data_size);
    offset += field.data_size;

    return true;
}

bool DudeDatabase::SetField(MacAddressField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size =
        sizeof(MacAddressField::info) + sizeof(MacAddressField::data_size);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::LongArray)) {
        return {};
    }

    if (!CheckSize(raw_data.size(), offset, field.data_size)) {
        return {};
    }

    field.mac_address.resize(field.data_size / sizeof(MacAddress));
    memcpy(field.mac_address.data(), raw_data.data() + offset, field.data_size);
    offset += field.data_size;

    return true;
}

bool DudeDatabase::SetField(StringArrayField& field, FieldId id, std::span<const u8> raw_data,
                            std::size_t& offset) const {
    constexpr std::size_t header_size =
        sizeof(StringArrayField::info) + sizeof(StringArrayField::entry_count);

    if (!CheckSize(raw_data.size(), offset, header_size)) {
        return {};
    }

    memcpy(&field, raw_data.data() + offset, header_size);
    offset += header_size;

    if (!ValidateId(field.info.id.Value(), id)) {
        offset -= header_size;
        field = {};
        return true;
    }

    if (!ValidateType(field.info.type.Value(), FieldType::StringArray)) {
        return {};
    }

    for (std::size_t i = 0; i < field.entry_count; ++i) {
        constexpr std::size_t entry_header_size = sizeof(StringArrayEntry::data_size);
        StringArrayEntry entry{};

        if (!CheckSize(raw_data.size(), offset, entry_header_size)) {
            return {};
        }

        memcpy(&entry, raw_data.data() + offset, entry_header_size);
        offset += entry_header_size;

        if (!CheckSize(raw_data.size(), offset, entry.data_size)) {
            return {};
        }

        std::vector<char> raw_text(entry.data_size);
        memcpy(raw_text.data(), raw_data.data() + offset, entry.data_size);
        entry.text = std::string(raw_text.begin(), raw_text.end());
        offset += entry.data_size;

        field.entries.push_back(entry);
    }

    return true;
}

bool DudeDatabase::CheckSize(std::size_t raw_data_size, std::size_t offset,
                             std::size_t header_size) const {
    if (raw_data_size < header_size + offset) {
        printf("Invalid data size, expected %zu, found %zu\n", header_size + offset, raw_data_size);
        return false;
    }
    return true;
}

bool DudeDatabase::ValidateEndOfBlob(std::span<const u8> raw_data, std::size_t offset) const {
    if (raw_data.size() != offset) {
        printf("Data size mismatch, expected %zu, found %zu\n", raw_data.size(), offset);
        return false;
    }
    return true;
}

bool DudeDatabase::ValidateId(FieldId a, FieldId b) const {
    if (a != b) {
        printf("Invalid data id, expected 0x%06x, found 0x%06x\n", b, a);
        return false;
    }
    return true;
}

bool DudeDatabase::ValidateType(FieldType a, FieldType b) const {
    if (a != b) {
        printf("Invalid data type, expected 0x%02x, found 0x%02x\n", b, a);
        return false;
    }
    return true;
}

DataFormat DudeDatabase::GetMainDataFormat(const RawObjData& obj_data) const {
    if (obj_data.data_format.entries == 0) {
        return DataFormat::None;
    }

    return static_cast<DataFormat>(obj_data.data_format.data[0]);
}
} // namespace Database
