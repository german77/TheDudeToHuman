// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstddef>

#include "dude_database.h"

namespace Database {
	DudeDatabase::DudeDatabase(const std::string& db_file) : db{ db_file } {
		int rc = db.OpenDatabase();
		if (rc != 0) {
			printf("Can't open database: %s\n", db_file.c_str());
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

	std::vector<DataSourceData> DudeDatabase::GetDataSourceData() const {
		return GetObjectData<DataSourceData>(DataFormat::DataSource, &DudeDatabase::RawDataToDataSourceData);
	}

	std::vector<SnmpProfileData> DudeDatabase::GetSnmpProfileData() const {
		return GetObjectData<SnmpProfileData>(DataFormat::SnmpProfile, &DudeDatabase::RawDataToSnmpProfileData);
	}

	std::vector<NetworkMapElementData> DudeDatabase::GetNetworkMapElementData() const {
		return GetObjectData<NetworkMapElementData>(DataFormat::NetworkMapElement, &DudeDatabase::RawDataToNetworkMapElementData);
	}


	RawObjData DudeDatabase::BlobToRawObjData(std::span<const u8> blob) const {
		constexpr std::size_t header_size = sizeof(RawObjData::magic);
		RawObjData data{};

		if (blob.size() < header_size) {
			printf("Invalid blob size: %zu\n", blob.size());
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

		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.parent_id = GetIntField(raw_data, offset, FieldId::Note_ObjID);
		data.time_added = GetTimeField(raw_data, offset, FieldId::Note_TimeAdded);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

		return data;
	}

	DeviceTypeData DudeDatabase::RawDataToDeviceTypeData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		DeviceTypeData data{};

		data.ignored_services = GetIntArrayField(raw_data, offset, FieldId::DeviceType_IgnoredServices);
		data.allowed_services = GetIntArrayField(raw_data, offset, FieldId::DeviceType_AllowedServices);
		data.required_services = GetIntArrayField(raw_data, offset, FieldId::DeviceType_RequiredServices);
		data.image_id = GetIntField(raw_data, offset, FieldId::DeviceType_ImageId);
		data.image_scale = GetByteField(raw_data, offset, FieldId::DeviceType_ImageScale);
		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.next_id = GetIntField(raw_data, offset, FieldId::SysNextId);
		data.url = GetTextField(raw_data, offset, FieldId::DeviceType_Url);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

		return data;
	}

	DeviceData DudeDatabase::RawDataToDeviceData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		DeviceData data{};

		data.parent_ids = GetIntArrayField(raw_data, offset, FieldId::Device_ParentIds);
		data.notify_ids = GetIntArrayField(raw_data, offset, FieldId::Device_NotifyIds);
		data.dns_names = GetStringArrayField(raw_data, offset, FieldId::Device_DnsNames);
		data.ip = GetIpAddressField(raw_data, offset, FieldId::Device_IpAddress);
		data.secure_mode = GetBoolField(raw_data, offset, FieldId::Device_SecureMode);
		data.router_os = GetBoolField(raw_data, offset, FieldId::Device_RouterOs);
		data.dude_server = GetBoolField(raw_data, offset, FieldId::Device_DudeServer);
		data.notify_use = GetBoolField(raw_data, offset, FieldId::Device_NotifyUse);
		data.prove_enabled = GetBoolField(raw_data, offset, FieldId::Device_ProveEnabled);
		data.lookup = GetByteField(raw_data, offset, FieldId::Device_Lookup);
		data.dns_lookup_interval = GetByteField(raw_data, offset, FieldId::Device_LookupInterval);
		data.mac_lookup = GetByteField(raw_data, offset, FieldId::Device_MacLookup);
		data.type_id = GetIntField(raw_data, offset, FieldId::Device_TypeId);
		data.agent_id = GetIntField(raw_data, offset, FieldId::Device_AgentId);
		data.snmp_profile_id = GetIntField(raw_data, offset, FieldId::Device_SnmpProfileId);
		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.prove_interval = GetIntField(raw_data, offset, FieldId::Device_ProveInterval);
		data.prove_timeout = GetByteField(raw_data, offset, FieldId::Device_ProveTimeout);
		data.prove_down_count = GetByteField(raw_data, offset, FieldId::Device_ProveDownCount);
		data.custom_field_3 = GetTextField(raw_data, offset, FieldId::Device_CustomField3);
		data.custom_field_2 = GetTextField(raw_data, offset, FieldId::Device_CustomField2);
		data.custom_field_1 = GetTextField(raw_data, offset, FieldId::Device_CustomField1);
		data.password = GetTextField(raw_data, offset, FieldId::Device_Password);
		data.username = GetTextField(raw_data, offset, FieldId::Device_Username);
		data.mac = GetMacAddressField(raw_data, offset, FieldId::Device_MacAddress);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

		return data;
	}

	DataSourceData DudeDatabase::RawDataToDataSourceData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		DataSourceData data{};

		data.enabled = GetBoolField(raw_data, offset, FieldId::DataSource_Enabled);
		data.function_device_id = GetIntField(raw_data, offset, FieldId::DataSource_FunctionDevice);
		data.function_interval = GetByteField(raw_data, offset, FieldId::DataSource_FunctionInterval);
		data.data_source_type = GetByteField(raw_data, offset, FieldId::DataSource_Type);
		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.keep_time_raw = GetByteField(raw_data, offset, FieldId::DataSource_KeepTimeRaw);
		data.keep_time_10min = GetByteField(raw_data, offset, FieldId::DataSource_KeepTime10min);
		data.keep_time_2hour = GetByteField(raw_data, offset, FieldId::DataSource_KeepTime2hour);
		data.keep_time_1Day = GetByteField(raw_data, offset, FieldId::DataSource_KeepTime1day);
		data.function_code = GetTextField(raw_data, offset, FieldId::DataSource_FunctionCode);
		data.unit = GetTextField(raw_data, offset, FieldId::DataSource_Unit);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

		return data;
	}

	SnmpProfileData DudeDatabase::RawDataToSnmpProfileData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		SnmpProfileData data{};

		data.version = GetIntField(raw_data, offset, FieldId::SnmpProfile_Version);
		data.port = GetIntField(raw_data, offset, FieldId::SnmpProfile_Port);
		data.security = GetByteField(raw_data, offset, FieldId::SnmpProfile_V3Security);
		data.auth_method = GetByteField(raw_data, offset, FieldId::SnmpProfile_V3AuthMethod);
		data.crypth_method = GetByteField(raw_data, offset, FieldId::SnmpProfile_V3CryptMethod);
		data.try_count = GetByteField(raw_data, offset, FieldId::SnmpProfile_TryCount);
		data.try_timeout = GetIntField(raw_data, offset, FieldId::SnmpProfile_TryTimeout);
		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.crypt_password = GetTextField(raw_data, offset, FieldId::SnmpProfile_V3CryptPassword);
		data.auth_password = GetTextField(raw_data, offset, FieldId::SnmpProfile_V3AuthPassword);
		data.community = GetTextField(raw_data, offset, FieldId::SnmpProfile_Community);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

		return data;
	}

	NetworkMapElementData DudeDatabase::RawDataToNetworkMapElementData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		NetworkMapElementData data{};

		data.item_use_acked_color = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseAckedColor);
		data.item_use_label = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseLabel);
		data.item_use_shapes = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseShape);
		data.item_use_font = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseFont);
		data.item_use_image = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseImage);
		data.item_use_image_scale = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseImageScale);
		data.item_use_width = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_LinkUseWidth);
		data.item_use_up_color = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseUpColor);
		data.item_use_down_partial_color = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseDownPartialColor);
		data.item_use_down_complete_color = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseDownCompleteColor);
		data.item_use_unknown_color = GetBoolField(raw_data, offset, FieldId::NetworkMapElement_ItemUseUnknownColor);
		data.item_up_color = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemUpColor);
		data.item_down_partial_color = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemDownPartialColor);
		data.item_down_complete_color = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemDownCompleteColor);
		data.item_unknown_color = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemUnknownColor);
		data.item_acked_color = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemAckedColor);
		data.item_shape = GetByteField(raw_data, offset, FieldId::NetworkMapElement_ItemShape);
		data.item_image = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemImage);
		data.item_image_scale = GetByteField(raw_data, offset, FieldId::NetworkMapElement_ItemImageScale);
		data.link_from = GetIntField(raw_data, offset, FieldId::NetworkMapElement_LinkFrom);
		data.link_to = GetIntField(raw_data, offset, FieldId::NetworkMapElement_LinkTo);
		data.link_id = GetIntField(raw_data, offset, FieldId::NetworkMapElement_LinkID);
		data.link_width = GetByteField(raw_data, offset, FieldId::NetworkMapElement_LinkWidth);
		data.object_id = GetIntField(raw_data, offset, FieldId::SysId);
		data.map_id = GetIntField(raw_data, offset, FieldId::NetworkMapElement_MapID);
		data.type = GetByteField(raw_data, offset, FieldId::NetworkMapElement_Type);
		data.item_type = GetByteField(raw_data, offset, FieldId::NetworkMapElement_ItemType);
		data.item_id = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemID);
		data.item_x = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemX);
		data.item_y = GetIntField(raw_data, offset, FieldId::NetworkMapElement_ItemY);
		data.label_refresh_interval = GetIntField(raw_data, offset, FieldId::NetworkMapElement_LabelRefreshInterval);
		data.item_font = GetLongArrayField(raw_data, offset, FieldId::NetworkMapElement_ItemFont);
		data.name = GetTextField(raw_data, offset, FieldId::SysName);
		ValidateEndOfBlob(raw_data, offset);

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
			printf("Invalid data type, expected 0/1, found %u\n", field.info.type.Value());
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
			printf("Invalid data type, expected %zu, found %zu\n", header_size + offset, raw_data_size);
			return false;
		}
		return true;
	}

	bool DudeDatabase::ValidateEndOfBlob(std::span<const u8> raw_data, std::size_t offset) const {
		if (raw_data.size() != offset) {
			printf("Data size mismatch, expected %zu, found %zu\n", raw_data.size(), offset);
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
