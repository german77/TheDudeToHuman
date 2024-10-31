// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>

#include "DudeDatabase.h"

namespace Database {
	DudeDatabase::DudeDatabase(const std::string& db_file) : db{ db_file } {
		int rc = db.OpenDatabase();
		if (rc != 0) {
			printf("Can't open database: %s\n", db_file);
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

	std::vector<u32> DudeDatabase::ListObjectTypes() const {
		std::vector<u32> object_types{};
		Database::SqlData data{};
		GetObjs(data);

		for (auto& [id, blob] : data) {
			RawObjData obj_data = BlobToRawObjData(blob);
			auto it = find(object_types.begin(), object_types.end(), obj_data.object_type);

			if (it != object_types.end()) {
				continue;
			}
			object_types.push_back(obj_data.object_type);
		}

		return object_types;
	}

	RawObjData DudeDatabase::BlobToRawObjData(std::span<u8> blob) const {
		constexpr std::size_t header_size = sizeof(u64) + sizeof(u32);
		RawObjData data{};

		if (blob.size() < header_size) {
			printf("Invalid blob size: %d\n", blob.size());
			return {};
		}

		memcpy(&data, blob.data(), header_size);
		data.raw_data.resize(blob.size() - header_size);
		memcpy(data.raw_data.data(), blob.data() + header_size, data.raw_data.size());

		return data;
	}
}
