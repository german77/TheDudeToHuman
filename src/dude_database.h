// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <string>
#include <vector>

#include "common/common_types.h"
#include "dude_types.h"
#include "sqlite_reader.h"

namespace Database {
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

		std::vector<MapData> GetMapData() const;
		std::vector<NotesData> GetNotesData() const;
		std::vector<DeviceTypeData> GetDeviceTypeData() const;
		std::vector<DeviceData> GetDeviceData() const;
		std::vector<ServiceData> GetServiceData() const;
		std::vector<DataSourceData> GetDataSourceData() const;
		std::vector<SnmpProfileData> GetSnmpProfileData() const;
		std::vector<NetworkMapElementData> GetNetworkMapElementData() const;
		std::vector<PanelElementData> GetPanelElementData() const;

	private:
		template <typename T>
		std::vector<T> GetObjectData(DataFormat format, T(DudeDatabase::* RawToObjData)(std::span<const u8> raw_data) const) const;

		RawObjData BlobToRawObjData(std::span<const u8> blob) const;
		MapData RawDataToMapData(std::span<const u8> raw_data) const;
		NotesData RawDataToNotesData(std::span<const u8> raw_data) const;
		DeviceTypeData RawDataToDeviceTypeData(std::span<const u8> raw_data) const;
		DeviceData RawDataToDeviceData(std::span<const u8> raw_data) const;
		ServiceData RawDataToServiceData(std::span<const u8> raw_data) const;
		DataSourceData RawDataToDataSourceData(std::span<const u8> raw_data) const;
		SnmpProfileData RawDataToSnmpProfileData(std::span<const u8> raw_data) const;
		NetworkMapElementData RawDataToNetworkMapElementData(std::span<const u8> raw_data) const;
		PanelElementData RawDataToPanelElementData(std::span<const u8> raw_data) const;

		bool SetField(BoolField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(ByteField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(IntField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(TimeField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(LongField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(TextField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(IntArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(IpAddressField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(LongArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(MacAddressField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;
		bool SetField(StringArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const;

		bool CheckSize(std::size_t raw_data_size, std::size_t offset, std::size_t header_size) const;
		bool ValidateEndOfBlob(std::span<const u8> raw_data, std::size_t offset) const;
		bool ValidateId(FieldId a, FieldId b) const;
		bool ValidateType(FieldType a, FieldType b) const;
		DataFormat GetMainDataFormat(const RawObjData& obj_data) const;

		Database::SqliteReader db;
	};
}
