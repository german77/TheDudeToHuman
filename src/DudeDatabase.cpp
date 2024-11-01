// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstddef>

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

	std::vector<ObjectType> DudeDatabase::ListUsedObjectTypes() const {
		std::vector<ObjectType> object_types{};
		Database::SqlData sql_data{};
		GetObjs(sql_data);

		for (auto& [id, blob] : sql_data) {
			const RawObjData obj_data = BlobToRawObjData(blob);
			const auto it = find(object_types.begin(), object_types.end(), obj_data.object_type);

			if (it != object_types.end()) {
				continue;
			}
			object_types.push_back(obj_data.object_type);
		}

		return object_types;
	}

	std::vector<std::pair<int, DeviceData>> DudeDatabase::GetDeviceData() const {
		std::vector<std::pair<int, DeviceData>> device_data{};
		Database::SqlData sql_data{};
		GetObjs(sql_data);

		for (auto& [id, blob] : sql_data) {
			const RawObjData obj_data = BlobToRawObjData(blob);

			if (obj_data.object_type != ObjectType::Device) {
				continue;
			}

			printf("Reading row %d\n", id);

			const DeviceData device = RawDataToDeviceData(obj_data.data);

			device_data.push_back({ id, device });
		}

		return device_data;
	}

	RawObjData DudeDatabase::BlobToRawObjData(std::span<const u8> blob) const {
		constexpr std::size_t header_size = sizeof(u64) + sizeof(u32);
		RawObjData data{};

		if (blob.size() < header_size) {
			printf("Invalid blob size: %d\n", blob.size());
			return {};
		}

		memcpy(&data, blob.data(), header_size);
		data.data.resize(blob.size() - header_size);
		memcpy(data.data.data(), blob.data() + header_size, data.data.size());

		return data;
	}

	DeviceData DudeDatabase::RawDataToDeviceData(std::span<const u8> raw_data) const {
		DeviceData data{};

		std::size_t offset = offsetof(DeviceData, unk);
		data.unk = GetUnknownDeviceField(raw_data, offset);
		offset += 5; // padding

		data.dns = GetDnsField(raw_data, offset);
		offset += 6; // padding

		memcpy(&data.ip, raw_data.data() + offset, sizeof(IpAddress));
		offset += sizeof(IpAddress) + 0x43;

		data.unk1 = GetDataField<char>(raw_data, offset);
		data.unk2 = GetDataField<char>(raw_data, offset);
		data.unk3 = GetDataField<char>(raw_data, offset);
		data.custom_field_3 = GetDataField<char>(raw_data, offset);
		data.custom_field_2 = GetDataField<char>(raw_data, offset);
		data.custom_field_1 = GetDataField<char>(raw_data, offset);
		data.password = GetDataField<char>(raw_data, offset);
		data.user = GetDataField<char>(raw_data, offset);
		data.mac = GetDataField<MacAddress>(raw_data, offset);
		data.name = GetDataField<char>(raw_data, offset);

		return data;
	}

	template <typename T>
	DataField<T> DudeDatabase::GetDataField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(u8) + sizeof(u16) + sizeof(FieldType) + sizeof(u8);
		DataField<T> field{};
		auto& field_data = field.data;
		auto& data = field.data.data;

		if (raw_data.size() < header_size + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		if (raw_data.size() < field_data.data_size + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		if (field_data.data_size % sizeof(T) != 0) {
			printf("Invalid object size: %d\n", field_data.data_size);
			return {};
		}

		data.resize(field_data.data_size / sizeof(T));
		memcpy(data.data(), raw_data.data() + offset, field_data.data_size);
		offset += field_data.data_size;

		// Add null terminator to strings
		if (std::is_same<T, char>::value) {
			data.push_back({});
			printf("%d, ", field_data.data_size);

			for (auto& ch : data) {
				printf("%c", ch);
			}
			printf("\n");
		}

		return field;
	}

	UnknownDeviceField DudeDatabase::GetUnknownDeviceField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(u8);
		UnknownDeviceField field{};

		if (raw_data.size() < header_size + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		if (raw_data.size() < (field.entries * sizeof(u32)) + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		field.data.resize(field.entries);
		memcpy(field.data.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return field;
	}

	DnsField DudeDatabase::GetDnsField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(u8) + sizeof(u8);
		DnsField field{};

		if (raw_data.size() < header_size + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		if (!field.has_dns) {
			return field;
		}

		if (raw_data.size() < sizeof(u16) + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		memcpy(&field.data_size, raw_data.data() + offset, sizeof(u16));
		offset += sizeof(u16);

		if (raw_data.size() < field.data_size + offset) {
			printf("Invalid data size: %d\n", raw_data.size());
			return {};
		}

		field.data.resize(field.data_size);
		memcpy(field.data.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		field.data.push_back({});
		printf("dns %d, ", field.data_size);

		for (auto& ch : field.data) {
			printf("%c", ch);
		}
		printf("\n");

		return field;
	}
}
