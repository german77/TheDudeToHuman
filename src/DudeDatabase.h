// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <span>
#include <string>
#include <vector>

#include "common_types.h"
#include "SqliteReader.h"

namespace Database {
	struct RawObjData {
		u64 magic;
		u32 object_type;
		std::vector<u8> raw_data;
	};

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
		std::vector<u32> ListObjectTypes() const;

	private:
		RawObjData BlobToRawObjData(std::span<u8> blob) const;

		Database::SqliteReader db;
	};
}
