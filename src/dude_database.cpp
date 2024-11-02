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

	std::vector<DataFormat> DudeDatabase::ListUsedDataFormats() const {
		std::vector<DataFormat> data_formats{};
		Database::SqlData sql_data{};
		GetObjs(sql_data);

		for (auto& [id, blob] : sql_data) {
			const RawObjData obj_data = BlobToRawObjData(blob);
			for (auto& data_format : obj_data.data_format.data) {
				const DataFormat format = static_cast<DataFormat>(data_format);
				const auto it = find(data_formats.begin(), data_formats.end(), format);

				if (it != data_formats.end()) {
					continue;
				}
				data_formats.push_back(format);
			}
		}

		return data_formats;
	}

	template <typename T>
	std::vector<T> DudeDatabase::GetObjectData(DataFormat format, T(DudeDatabase::* RawToObjData)(std::span<const u8> raw_data) const) const {
		std::vector<T> data{};
		Database::SqlData sql_data{};
		GetObjs(sql_data);

		for (auto& [id, blob] : sql_data) {
			const RawObjData raw_obj_data = BlobToRawObjData(blob);

			if (GetMainDataFormat(raw_obj_data) != format) {
				continue;
			}

			printf("Reading row %d\n", id);

			const T obj_data = (this->*RawToObjData)(raw_obj_data.data);

			if (id != obj_data.object_id.value) {
				printf("Corrupted Entry\n");
			}

			data.push_back(obj_data);
		}

		return data;
	}

	std::vector<NotesData> DudeDatabase::GetNotesData() const {
		return GetObjectData<NotesData>(DataFormat::Notes, &DudeDatabase::RawDataToNotesData);
	}

	std::vector<DeviceTypeData> DudeDatabase::GetDeviceTypeData() const {
		return GetObjectData<DeviceTypeData>(DataFormat::DeviceType, &DudeDatabase::RawDataToDeviceTypeData);
	}

	std::vector<DeviceData> DudeDatabase::GetDeviceData() const {
		return GetObjectData<DeviceData>(DataFormat::Device, &DudeDatabase::RawDataToDeviceData);
	}

	std::vector<LinkData> DudeDatabase::GetLinkData() const {
		return GetObjectData<LinkData>(DataFormat::Link, &DudeDatabase::RawDataToLinkData);
	}

	std::vector<SnmpProfileData> DudeDatabase::GetSnmpProfileData() const {
		return GetObjectData<SnmpProfileData>(DataFormat::SnmpProfile, &DudeDatabase::RawDataToSnmpProfileData);
	}

	std::vector<Unknown4aData> DudeDatabase::GetUnknown4aData() const {
		return GetObjectData<Unknown4aData>(DataFormat::Unknown4a, &DudeDatabase::RawDataToUnknown4aData);
	}


	RawObjData DudeDatabase::BlobToRawObjData(std::span<const u8> blob) const {
		constexpr std::size_t header_size = sizeof(RawObjData::magic);
		RawObjData data{};

		if (blob.size() < header_size) {
			printf("Invalid blob size: %d\n", blob.size());
			return {};
		}

		memcpy(&data, blob.data(), header_size);

		std::size_t offset = header_size;
		data.data_format = GetIntArrayField(blob, offset, FieldId::DataFormat);

		data.data.resize(blob.size() - offset);
		memcpy(data.data.data(), blob.data() + offset, data.data.size());

		return data;
	}

	NotesData DudeDatabase::RawDataToNotesData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		NotesData data{};

		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.parent_id = GetIntField(raw_data, offset, FieldId::ParentId);
		data.time = GetTimeField(raw_data, offset, FieldId::Time);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	DeviceTypeData DudeDatabase::RawDataToDeviceTypeData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		DeviceTypeData data{};

		data.ignored_services = GetIntArrayField(raw_data, offset, FieldId::IgnoredServices);
		data.allowed_services = GetIntArrayField(raw_data, offset, FieldId::AllowedServices);
		data.required_services = GetIntArrayField(raw_data, offset, FieldId::RequiredServices);
		data.image_id = GetIntField(raw_data, offset, FieldId::ImageId);
		data.scale = GetByteField(raw_data, offset, FieldId::Scale);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.secondary_object_id = GetIntField(raw_data, offset, FieldId::SecondaryObjectId);
		data.url = GetTextField(raw_data, offset, FieldId::UrlAddress);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	DeviceData DudeDatabase::RawDataToDeviceData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		DeviceData data{};

		data.unk1 = GetIntArrayField(raw_data, offset, FieldId::Unknown57);
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
		data.snmp_profile_id = GetIntField(raw_data, offset, FieldId::SnmpProfileId);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.unk14 = GetIntField(raw_data, offset, FieldId::Unknown52);
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

	LinkData DudeDatabase::RawDataToLinkData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		LinkData data{};

		data.unk1 = GetBoolField(raw_data, offset, FieldId::None);
		data.unk2 = GetIntField(raw_data, offset, FieldId::None);
		data.unk3 = GetByteField(raw_data, offset, FieldId::None);
		data.unk4 = GetByteField(raw_data, offset, FieldId::None);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.unk6 = GetByteField(raw_data, offset, FieldId::None);
		data.unk7 = GetByteField(raw_data, offset, FieldId::None);
		data.unk8 = GetByteField(raw_data, offset, FieldId::None);
		data.unk9 = GetByteField(raw_data, offset, FieldId::None);
		data.unk10 = GetTextField(raw_data, offset, FieldId::None);
		data.unit = GetTextField(raw_data, offset, FieldId::None);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	SnmpProfileData DudeDatabase::RawDataToSnmpProfileData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		SnmpProfileData data{};

		data.version = GetIntField(raw_data, offset, FieldId::SnmpVersion);
		data.port = GetIntField(raw_data, offset, FieldId::Port);
		data.unk3 = GetByteField(raw_data, offset, FieldId::Unknown6B);
		data.unk4 = GetByteField(raw_data, offset, FieldId::Unknown6C);
		data.unk5 = GetByteField(raw_data, offset, FieldId::Unknown6E);
		data.tries = GetByteField(raw_data, offset, FieldId::Tries);
		data.try_timeout = GetIntField(raw_data, offset, FieldId::TryTimeout);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.auth_password = GetTextField(raw_data, offset, FieldId::AuthPassword);
		data.crypt_password = GetTextField(raw_data, offset, FieldId::CryptPassword);
		data.community = GetTextField(raw_data, offset, FieldId::Community);
		data.name = GetTextField(raw_data, offset, FieldId::Name);

		return data;
	}

	Unknown4aData DudeDatabase::RawDataToUnknown4aData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		Unknown4aData data{};

		data.unk1 = GetBoolField(raw_data, offset, FieldId::Unknown5DCC);
		data.unk2 = GetBoolField(raw_data, offset, FieldId::Unknown5DCD);
		data.unk3 = GetBoolField(raw_data, offset, FieldId::Unknown5DCE);
		data.unk4 = GetBoolField(raw_data, offset, FieldId::Unknown5DCF);
		data.unk5 = GetBoolField(raw_data, offset, FieldId::Unknown5DD0);
		data.unk6 = GetBoolField(raw_data, offset, FieldId::Unknown5DD1);
		data.unk7 = GetBoolField(raw_data, offset, FieldId::Unknown5DDE);
		data.unk8 = GetBoolField(raw_data, offset, FieldId::Unknown5DC8);
		data.unk9 = GetBoolField(raw_data, offset, FieldId::Unknown5DC9);
		data.unk10 = GetBoolField(raw_data, offset, FieldId::Unknown5DCA);
		data.unk11 = GetBoolField(raw_data, offset, FieldId::Unknown5DCB);
		data.unk12 = GetIntField(raw_data, offset, FieldId::Unknown5DD2);
		data.unk13 = GetIntField(raw_data, offset, FieldId::Unknown5DD3);
		data.unk14 = GetIntField(raw_data, offset, FieldId::Unknown5DD4);
		data.unk15 = GetIntField(raw_data, offset, FieldId::Unknown5DD5);
		data.unk16 = GetIntField(raw_data, offset, FieldId::Unknown5DD6);
		data.unk17 = GetByteField(raw_data, offset, FieldId::Unknown5DD7);
		data.unk18 = GetIntField(raw_data, offset, FieldId::Unknown5DD9);
		data.unk19 = GetByteField(raw_data, offset, FieldId::Unknown5DDA);
		data.unk20 = GetIntField(raw_data, offset, FieldId::Unknown5DDB);
		data.unk21 = GetIntField(raw_data, offset, FieldId::Unknown5DDC);
		data.unk22 = GetIntField(raw_data, offset, FieldId::Unknown5DDD);
		data.unk23 = GetByteField(raw_data, offset, FieldId::Unknown5DDF);
		data.object_id = GetIntField(raw_data, offset, FieldId::ObjectId);
		data.unk25 = GetIntField(raw_data, offset, FieldId::Unknown5DC0);
		data.unk26 = GetByteField(raw_data, offset, FieldId::Unknown5DC2);
		data.unk27 = GetByteField(raw_data, offset, FieldId::Unknown5DC3);
		data.unk28 = GetIntField(raw_data, offset, FieldId::Unknown5DC4);
		data.unk29 = GetIntField(raw_data, offset, FieldId::Unknown5DC5);
		data.unk30 = GetIntField(raw_data, offset, FieldId::Unknown5DC6);
		data.unk31 = GetIntField(raw_data, offset, FieldId::Unknown5DC7);
		data.unk32 = GetLongArrayField(raw_data, offset, FieldId::Unknown5DD8);
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

		return field;
	}

	IntField DudeDatabase::GetIntField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(IntField::info);
		IntField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);

		switch (field.info.type.Value()) {
		case FieldType::Int:
			memcpy(&field.value, raw_data.data() + offset, sizeof(IntField::value));
			offset += sizeof(IntField::value);
			break;
		case FieldType::Byte:
			// Int sometimes are saved as byte to save space
			memcpy(&field.value, raw_data.data() + offset, sizeof(ByteField::value));
			offset += sizeof(ByteField::value);
			break;
		default:
			ValidateType(field.info.type.Value(), FieldType::Int);
			break;
		}

		return field;
	}

	TimeField DudeDatabase::GetTimeField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(TimeField::info) + sizeof(TimeField::date);
		TimeField field{};

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
		constexpr std::size_t header_size = sizeof(TextField::info);
		TextField field{};

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);

		switch (field.info.type.Value()) {
		case FieldType::ShortString:
			memcpy(&field.data_size, raw_data.data() + offset, sizeof(u8));
			offset += sizeof(u8);
			break;
		case FieldType::LongString:
			memcpy(&field.data_size, raw_data.data() + offset, sizeof(u16));
			offset += sizeof(u16);
			break;
		default:
			ValidateType(field.info.type.Value(), FieldType::ShortString);
			break;
		}

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

	LongArrayField DudeDatabase::GetLongArrayField(std::span<const u8> raw_data, std::size_t& offset, FieldId id) const {
		constexpr std::size_t header_size = sizeof(LongArrayField::info) + sizeof(LongArrayField::data_size);
		LongArrayField field{};

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

		field.data.resize(field.data_size);
		memcpy(field.data.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

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
			printf("Invalid data id, expected 0x%06x, found 0x%06x\n", b, a);
			return false;
		}
		return true;
	}

	bool DudeDatabase::ValidateType(FieldType a, FieldType b) const {
		if (a != b) {
			printf("Invalid data type, expected 0x%02x, found 0x%02x\n", b, a);
			return false;
		}
		return true;
	}

	DataFormat DudeDatabase::GetMainDataFormat(const RawObjData& obj_data) const {
		if (obj_data.data_format.entries == 0) {
			return DataFormat::None;
		}

		return static_cast<DataFormat>(obj_data.data_format.data[0]);
	}
}
