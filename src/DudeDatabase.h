// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <string>
#include <vector>

#include "common_types.h"
#include "DudeTypes.h"
#include "SqliteReader.h"

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

		template <typename T>
		DataField<T> GetDataField(std::span<const u8> raw_data, std::size_t& offset, FieldType type) const;
		UnknownDeviceField GetUnknownDeviceField(std::span<const u8> raw_data, std::size_t& offset) const;
		DnsField GetDnsField(std::span<const u8> raw_data, std::size_t& offset) const;

		Database::SqliteReader db;
	};
}
