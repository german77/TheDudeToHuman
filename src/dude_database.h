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
		std::vector<ObjectType> ListUsedObjectTypes() const;

		std::vector<std::pair<int, DeviceTypeData>> GetDeviceTypeData() const;
		std::vector<std::pair<int, DeviceData>> GetDeviceData() const;
		std::vector<std::pair<int, SnmpProfileData>> GetSnmpProfileData() const;
		std::vector<std::pair<int, Unknown4aData>> GetUnknown4aData() const;

	private:
		RawObjData BlobToRawObjData(std::span<const u8> blob) const;
		DeviceTypeData RawDataToDeviceTypeData(std::span<const u8> raw_data) const;
		DeviceData RawDataToDeviceData(std::span<const u8> raw_data) const;
		SnmpProfileData RawDataToSnmpProfileData(std::span<const u8> raw_data) const;
		Unknown4aData RawDataToUnknown4aData(std::span<const u8> raw_data) const;

		BoolField GetBoolField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		ByteField GetByteField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		IntField GetIntField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		TextField GetTextField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		IntArrayField GetIntArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		IpAddressField GetIpAddressField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		LongArrayField GetLongArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		MacAddressField GetMacAddressField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;
		StringArrayField GetStringArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const;

		bool CheckSize(std::size_t raw_data_size, std::size_t offset, std::size_t header_size) const;
		bool ValidateId(FieldId a, FieldId b) const;
		bool ValidateType(FieldType a, FieldType b) const;

		Database::SqliteReader db;
	};
}
