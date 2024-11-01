// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstddef>

#include "dude_database.h"

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

			if (id != device.object_id.id) {
				printf("Corrupted Entry\n");
			}

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
		data.unk = GetUnknownDeviceField1(raw_data, offset);
		data.dns = GetDnsField(raw_data, offset);
		data.ip = GetIpAddressField(raw_data, offset);
		offset += 0x3F;
		data.object_id = GetObjectIdField(raw_data, offset);
		data.unk1 = GetUnknownDeviceField2(raw_data, offset, FieldId::Unk1);
		data.unk2 = GetUnknownDeviceField2(raw_data, offset, FieldId::Unk2);
		data.unk3 = GetUnknownDeviceField2(raw_data, offset, FieldId::Unk3);
		data.custom_field_3 = GetTextField(raw_data, offset, FieldId::CustomField3);
		data.custom_field_2 = GetTextField(raw_data, offset, FieldId::CustomField2);
		data.custom_field_1 = GetTextField(raw_data, offset, FieldId::CustomField1);
		data.password = GetTextField(raw_data, offset, FieldId::Password);
		data.username = GetTextField(raw_data, offset, FieldId::Username);
		data.mac = GetMacAddressField(raw_data, offset);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	TextField DudeDatabase::GetTextField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(TextField::info) + sizeof(TextField::data_size);
		TextField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::ShortString);

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		std::vector<char> raw_text(field.data_size);
		memcpy(raw_text.data(), raw_data.data() + offset, field.data_size);
		field.text = std::string(raw_text.begin(), raw_text.end());
		offset += field.data_size;

		return field;
	}

	ObjectIdField DudeDatabase::GetObjectIdField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(ObjectIdField::info) + sizeof(ObjectIdField::id);
		ObjectIdField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), FieldId::ObjectId);
		ValidateType(field.info.type.Value(), FieldType::Int);

		return field;
	}

	UnknownDeviceField1 DudeDatabase::GetUnknownDeviceField1(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(UnknownDeviceField1::info) + sizeof(UnknownDeviceField1::entries);
		UnknownDeviceField1 field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), FieldId::Unknown56);
		ValidateType(field.info.type.Value(), FieldType::IntArray);

		if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
			return {};
		}

		field.data.resize(field.entries);
		memcpy(field.data.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return field;
	}

	UnknownDeviceField2 DudeDatabase::GetUnknownDeviceField2(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(UnknownDeviceField2::info) + sizeof(UnknownDeviceField2::data);
		UnknownDeviceField2 field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::Byte);

		return field;
	}

	IpAddressField DudeDatabase::GetIpAddressField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(IpAddressField::info) + sizeof(IpAddressField::data_size);
		IpAddressField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), FieldId::IpAddress);
		ValidateType(field.info.type.Value(), FieldType::IntArray);

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		field.ip_address.resize(field.data_size / sizeof(IpAddress));
		memcpy(field.ip_address.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		return field;
	}

	MacAddressField DudeDatabase::GetMacAddressField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(MacAddressField::info) + sizeof(MacAddressField::data_size);
		MacAddressField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), FieldId::MacAddress);
		ValidateType(field.info.type.Value(), FieldType::LongArray);

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		field.mac_address.resize(field.data_size / sizeof(MacAddress));
		memcpy(field.mac_address.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		return field;
	}

	DnsField DudeDatabase::GetDnsField(std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(DnsField::info) + sizeof(DnsField::entries);
		DnsField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), FieldId::DnsNames);
		ValidateType(field.info.type.Value(), FieldType::StringArray);

		if (field.entries == 0) {
			return field;
		}

		if (!CheckSize(raw_data.size(), offset, sizeof(u16))) {
			return {};
		}

		memcpy(&field.data_size, raw_data.data() + offset, sizeof(u16));
		offset += sizeof(u16);

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		std::vector<char> raw_text(field.data_size);
		memcpy(raw_text.data(), raw_data.data() + offset, field.data_size);
		field.dns = std::string(raw_text.begin(), raw_text.end());
		offset += field.data_size;

		return field;
	}

	bool DudeDatabase::CheckSize(std::size_t raw_data_size, std::size_t offset, std::size_t header_size) const {
		if (raw_data_size < header_size + offset) {
			printf("Invalid data type, expected %d, found %d\n", header_size + offset, raw_data_size);
			return false;
		}
		return true;

	}

	bool DudeDatabase::ValidateId(FieldId a, FieldId b) const {
		if (a != b) {
			printf("Invalid data id, expected %d, found %d\n", b, a);
			return false;
		}
		return true;
	}

	bool DudeDatabase::ValidateType(FieldType a, FieldType b) const {
		if (a != b) {
			printf("Invalid data type, expected %d, found %d\n", b, a);
			return false;
		}
		return true;
	}
}
