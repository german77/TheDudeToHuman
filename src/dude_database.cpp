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

			if (id != device.object_id.value) {
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
		std::size_t offset = 0;
		DeviceData data{};

		data.unk1 = GetIntArrayField(raw_data, offset, FieldId::Pid);
		data.unk2 = GetIntArrayField(raw_data, offset, FieldId::Unknown56);
		data.dns = GetStringArrayField(raw_data, offset, FieldId::DnsNames);
		data.ip = GetIpAddressField(raw_data, offset, FieldId::IpAddress);
		data.unk3 = GetBoolField(raw_data, offset, FieldId::Unknown49);
		data.router_os = GetBoolField(raw_data, offset, FieldId::RouterOs);
		data.unk5 = GetBoolField(raw_data, offset, FieldId::Unknown4B);
		data.unk6 = GetBoolField(raw_data, offset, FieldId::Unknown55);
		data.unk7 = GetBoolField(raw_data, offset, FieldId::Unknown51);
		data.unk8 = GetByteField(raw_data, offset, FieldId::Unknown42);
		data.dns_lookup_interval = GetByteField(raw_data, offset, FieldId::DnsLookupInterval);
		data.unk10 = GetByteField(raw_data, offset, FieldId::Unknown45);
		data.device_type_id = GetIntField(raw_data, offset, FieldId::DeviceTypeId);
		data.unk12 = GetIntField(raw_data, offset, FieldId::Unknown4D);
		data.unk13 = GetIntField(raw_data, offset, FieldId::Unknown4E);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.unk14 = GetByteField(raw_data, offset, FieldId::Unknown52); // Not always a byte
		data.unk15 = GetByteField(raw_data, offset, FieldId::Unknown53);
		data.unk16 = GetByteField(raw_data, offset, FieldId::Unknown54);
		data.custom_field_3 = GetTextField(raw_data, offset, FieldId::CustomField3);
		data.custom_field_2 = GetTextField(raw_data, offset, FieldId::CustomField2);
		data.custom_field_1 = GetTextField(raw_data, offset, FieldId::CustomField1);
		data.password = GetTextField(raw_data, offset, FieldId::Password);
		data.username = GetTextField(raw_data, offset, FieldId::Username);
		data.mac = GetMacAddressField(raw_data, offset, FieldId::MacAddress);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	BoolField DudeDatabase::GetBoolField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(BoolField::info);
		BoolField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);

		if (field.info.type == FieldType::BoolFalse) {
			field.value = false;
		}
		else if (field.info.type == FieldType::BoolTrue) {
			field.value = true;
		}
		else {
			printf("Invalid data type, expected 0/1, found %d\n", field.info.type);
			return {};
		}

		return field;
	}

	ByteField DudeDatabase::GetByteField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(ByteField::info) + sizeof(ByteField::value);
		ByteField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::Byte);

		// TODO: Type mismatch. Fix behaviour
		if (field.info.type == FieldType::Int) {
			offset += sizeof(IntField::value) - sizeof(ByteField::value);
		}

		return field;
	}

	IntField DudeDatabase::GetIntField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(IntField::info) + sizeof(IntField::value);
		IntField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::Int);

		return field;
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

	IntArrayField DudeDatabase::GetIntArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(IntArrayField::info) + sizeof(IntArrayField::entries);
		IntArrayField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::IntArray);

		if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
			return {};
		}

		field.data.resize(field.entries);
		memcpy(field.data.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return field;
	}

	IpAddressField DudeDatabase::GetIpAddressField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(IpAddressField::info) + sizeof(IpAddressField::entries);
		IpAddressField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::IntArray);

		if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
			return {};
		}

		field.ip_address.resize(field.entries);
		memcpy(field.ip_address.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return field;
	}

	MacAddressField DudeDatabase::GetMacAddressField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(MacAddressField::info) + sizeof(MacAddressField::data_size);
		MacAddressField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::LongArray);

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		field.mac_address.resize(field.data_size / sizeof(MacAddress));
		memcpy(field.mac_address.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		return field;
	}

	StringArrayField DudeDatabase::GetStringArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(StringArrayField::info) + sizeof(StringArrayField::entry_count);
		StringArrayField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		ValidateType(field.info.type.Value(), FieldType::StringArray);

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
