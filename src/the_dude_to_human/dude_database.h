// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <string>
#include <vector>

#include "common/common_types.h"
#include "database/dude_types.h"
#include "sqlite_reader.h"

namespace Database {
class DudeFieldParser;

class DudeDatabase {
public:
    DudeDatabase(const std::string& db_file);
    ~DudeDatabase();

    int GetChartValuesRaw(SqlData& data) const;
    int GetChartValues10Min(SqlData& data) const;
    int GetChartValues2Hour(SqlData& data) const;
    int GetChartValues1Day(SqlData& data) const;

    int GetObjs(SqlData& data) const;
    int GetOutages(SqlData& data) const;

    // Usefull to find new unsuported types
    std::vector<DataFormat> ListUsedDataFormats() const;

    std::vector<ServerConfigData> GetServerConfigData() const;
    std::vector<ToolData> GetToolData() const;
    std::vector<FileData> GetFileData() const;
    std::vector<NotesData> GetNotesData() const;
    std::vector<MapData> GetMapData() const;
    std::vector<DeviceTypeData> GetDeviceTypeData() const;
    std::vector<DeviceData> GetDeviceData() const;
    std::vector<ServiceData> GetServiceData() const;
    std::vector<NotificationData> GetNotificationData() const;
    std::vector<LinkData> GetLinkData() const;
    std::vector<LinkTypeData> GetLinkTypeData() const;
    std::vector<DataSourceData> GetDataSourceData() const;
    std::vector<FunctionData> GetFunctionData() const;
    std::vector<SnmpProfileData> GetSnmpProfileData() const;
    std::vector<PanelData> GetPanelData() const;
    std::vector<SysLogRuleData> GetSysLogRuleData() const;
    std::vector<NetworkMapElementData> GetNetworkMapElementData() const;
    std::vector<ChartLineData> GetChartLineData() const;
    std::vector<PanelElementData> GetPanelElementData() const;

private:
    template <typename T>
    std::vector<T> GetObjectData(DataFormat format,
                                 T (DudeDatabase::*RawToObjData)(DudeFieldParser& parser)
                                     const) const;

    ServerConfigData GetServerConfigData(DudeFieldParser& parser) const;
    ToolData GetToolData(DudeFieldParser& parser) const;
    FileData GetFileData(DudeFieldParser& parser) const;
    NotesData GetNotesData(DudeFieldParser& parser) const;
    MapData GetMapData(DudeFieldParser& parser) const;
    DeviceTypeData GetDeviceTypeData(DudeFieldParser& parser) const;
    DeviceData GetDeviceData(DudeFieldParser& parser) const;
    ServiceData GetServiceData(DudeFieldParser& parser) const;
    NotificationData GetNotificationData(DudeFieldParser& parser) const;
    LinkData GetLinkData(DudeFieldParser& parser) const;
    LinkTypeData GetLinkTypeData(DudeFieldParser& parser) const;
    DataSourceData GetDataSourceData(DudeFieldParser& parser) const;
    FunctionData GetFunctionData(DudeFieldParser& parser) const;
    SnmpProfileData GetSnmpProfileData(DudeFieldParser& parser) const;
    PanelData GetPanelData(DudeFieldParser& parser) const;
    SysLogRuleData GetSysLogRuleData(DudeFieldParser& parser) const;
    NetworkMapElementData GetNetworkMapElementData(DudeFieldParser& parser) const;
    ChartLineData GetChartLineData(DudeFieldParser& parser) const;
    PanelElementData GetPanelElementData(DudeFieldParser& parser) const;

    Database::SqliteReader db;
};
} // namespace Database
