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

		std::vector<std::pair<int, DeviceData>> GetDeviceData() const;

	private:
		RawObjData BlobToRawObjData(std::span<const u8> blob) const;
		DeviceData RawDataToDeviceData(std::span<const u8> raw_data) const;

		TextField GetTextField(std::span<const u8> raw_data, std::size_t& offset, FieldType type) const;
		ObjectIdField GetObjectIdField(std::span<const u8> raw_data, std::size_t& offset) const;
		UnknownDeviceField1 GetUnknownDeviceField1(std::span<const u8> raw_data, std::size_t& offset) const;
		UnknownDeviceField2 GetUnknownDeviceField2(std::span<const u8> raw_data, std::size_t& offset, FieldType type) const;
		IpAddressField GetIpAddressField(std::span<const u8> raw_data, std::size_t& offset) const;
		MacAddressField GetMacAddressField(std::span<const u8> raw_data, std::size_t& offset) const;
		DnsField GetDnsField(std::span<const u8> raw_data, std::size_t& offset) const;

		bool CheckSize(std::size_t raw_data_size, std::size_t offset, std::size_t header_size) const;
		bool ValidateType(FieldType a, FieldType b) const;

		Database::SqliteReader db;
	};
}
