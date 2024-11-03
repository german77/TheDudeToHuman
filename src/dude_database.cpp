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
		SetField(data.data_format, FieldId::DataFormat, blob, offset);

		data.data.resize(blob.size() - offset);
		memcpy(data.data.data(), blob.data() + offset, data.data.size());

		return data;
	}

	NotesData DudeDatabase::RawDataToNotesData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		NotesData data{};

		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.parent_id, FieldId::Note_ObjID, raw_data, offset);
		is_valid &= SetField(data.time_added, FieldId::Note_TimeAdded, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	DeviceTypeData DudeDatabase::RawDataToDeviceTypeData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		DeviceTypeData data{};

		is_valid &= SetField(data.ignored_services, FieldId::DeviceType_IgnoredServices, raw_data, offset);
		is_valid &= SetField(data.allowed_services, FieldId::DeviceType_AllowedServices, raw_data, offset);
		is_valid &= SetField(data.required_services, FieldId::DeviceType_RequiredServices, raw_data, offset);
		is_valid &= SetField(data.image_id, FieldId::DeviceType_ImageId, raw_data, offset);
		is_valid &= SetField(data.image_scale, FieldId::DeviceType_ImageScale, raw_data, offset);
		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.next_id, FieldId::SysNextId, raw_data, offset);
		is_valid &= SetField(data.url, FieldId::DeviceType_Url, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	DeviceData DudeDatabase::RawDataToDeviceData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		DeviceData data{};

		is_valid &= SetField(data.parent_ids, FieldId::Device_ParentIds, raw_data, offset);
		is_valid &= SetField(data.notify_ids, FieldId::Device_NotifyIds, raw_data, offset);
		is_valid &= SetField(data.dns_names, FieldId::Device_DnsNames, raw_data, offset);
		is_valid &= SetField(data.ip, FieldId::Device_IpAddress, raw_data, offset);
		is_valid &= SetField(data.secure_mode, FieldId::Device_SecureMode, raw_data, offset);
		is_valid &= SetField(data.router_os, FieldId::Device_RouterOs, raw_data, offset);
		is_valid &= SetField(data.dude_server, FieldId::Device_DudeServer, raw_data, offset);
		is_valid &= SetField(data.notify_use, FieldId::Device_NotifyUse, raw_data, offset);
		is_valid &= SetField(data.prove_enabled, FieldId::Device_ProveEnabled, raw_data, offset);
		is_valid &= SetField(data.lookup, FieldId::Device_Lookup, raw_data, offset);
		is_valid &= SetField(data.dns_lookup_interval, FieldId::Device_LookupInterval, raw_data, offset);
		is_valid &= SetField(data.mac_lookup, FieldId::Device_MacLookup, raw_data, offset);
		is_valid &= SetField(data.type_id, FieldId::Device_TypeId, raw_data, offset);
		is_valid &= SetField(data.agent_id, FieldId::Device_AgentId, raw_data, offset);
		is_valid &= SetField(data.snmp_profile_id, FieldId::Device_SnmpProfileId, raw_data, offset);
		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.prove_interval, FieldId::Device_ProveInterval, raw_data, offset);
		is_valid &= SetField(data.prove_timeout, FieldId::Device_ProveTimeout, raw_data, offset);
		is_valid &= SetField(data.prove_down_count, FieldId::Device_ProveDownCount, raw_data, offset);
		is_valid &= SetField(data.custom_field_3, FieldId::Device_CustomField3, raw_data, offset);
		is_valid &= SetField(data.custom_field_2, FieldId::Device_CustomField2, raw_data, offset);
		is_valid &= SetField(data.custom_field_1, FieldId::Device_CustomField1, raw_data, offset);
		is_valid &= SetField(data.password, FieldId::Device_Password, raw_data, offset);
		is_valid &= SetField(data.username, FieldId::Device_Username, raw_data, offset);
		is_valid &= SetField(data.mac, FieldId::Device_MacAddress, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	DataSourceData DudeDatabase::RawDataToDataSourceData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		DataSourceData data{};

		is_valid &= SetField(data.enabled, FieldId::DataSource_Enabled, raw_data, offset);
		is_valid &= SetField(data.function_device_id, FieldId::DataSource_FunctionDevice, raw_data, offset);
		is_valid &= SetField(data.function_interval, FieldId::DataSource_FunctionInterval, raw_data, offset);
		is_valid &= SetField(data.data_source_type, FieldId::DataSource_Type, raw_data, offset);
		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.keep_time_raw, FieldId::DataSource_KeepTimeRaw, raw_data, offset);
		is_valid &= SetField(data.keep_time_10min, FieldId::DataSource_KeepTime10min, raw_data, offset);
		is_valid &= SetField(data.keep_time_2hour, FieldId::DataSource_KeepTime2hour, raw_data, offset);
		is_valid &= SetField(data.keep_time_1Day, FieldId::DataSource_KeepTime1day, raw_data, offset);
		is_valid &= SetField(data.function_code, FieldId::DataSource_FunctionCode, raw_data, offset);
		is_valid &= SetField(data.unit, FieldId::DataSource_Unit, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	SnmpProfileData DudeDatabase::RawDataToSnmpProfileData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		SnmpProfileData data{};

		is_valid &= SetField(data.version, FieldId::SnmpProfile_Version, raw_data, offset);
		is_valid &= SetField(data.port, FieldId::SnmpProfile_Port, raw_data, offset);
		is_valid &= SetField(data.security, FieldId::SnmpProfile_V3Security, raw_data, offset);
		is_valid &= SetField(data.auth_method, FieldId::SnmpProfile_V3AuthMethod, raw_data, offset);
		is_valid &= SetField(data.crypth_method, FieldId::SnmpProfile_V3CryptMethod, raw_data, offset);
		is_valid &= SetField(data.try_count, FieldId::SnmpProfile_TryCount, raw_data, offset);
		is_valid &= SetField(data.try_timeout, FieldId::SnmpProfile_TryTimeout, raw_data, offset);
		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.crypt_password, FieldId::SnmpProfile_V3CryptPassword, raw_data, offset);
		is_valid &= SetField(data.auth_password, FieldId::SnmpProfile_V3AuthPassword, raw_data, offset);
		is_valid &= SetField(data.community, FieldId::SnmpProfile_Community, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	NetworkMapElementData DudeDatabase::RawDataToNetworkMapElementData(std::span<const u8> raw_data) const {
		std::size_t offset = 0;
		bool is_valid = true;
		NetworkMapElementData data{};

		is_valid &= SetField(data.item_use_acked_color, FieldId::NetworkMapElement_ItemUseAckedColor, raw_data, offset);
		is_valid &= SetField(data.item_use_label, FieldId::NetworkMapElement_ItemUseLabel, raw_data, offset);
		is_valid &= SetField(data.item_use_shapes, FieldId::NetworkMapElement_ItemUseShape, raw_data, offset);
		is_valid &= SetField(data.item_use_font, FieldId::NetworkMapElement_ItemUseFont, raw_data, offset);
		is_valid &= SetField(data.item_use_image, FieldId::NetworkMapElement_ItemUseImage, raw_data, offset);
		is_valid &= SetField(data.item_use_image_scale, FieldId::NetworkMapElement_ItemUseImageScale, raw_data, offset);
		is_valid &= SetField(data.item_use_width, FieldId::NetworkMapElement_LinkUseWidth, raw_data, offset);
		is_valid &= SetField(data.item_use_up_color, FieldId::NetworkMapElement_ItemUseUpColor, raw_data, offset);
		is_valid &= SetField(data.item_use_down_partial_color, FieldId::NetworkMapElement_ItemUseDownPartialColor, raw_data, offset);
		is_valid &= SetField(data.item_use_down_complete_color, FieldId::NetworkMapElement_ItemUseDownCompleteColor, raw_data, offset);
		is_valid &= SetField(data.item_use_unknown_color, FieldId::NetworkMapElement_ItemUseUnknownColor, raw_data, offset);
		is_valid &= SetField(data.item_up_color, FieldId::NetworkMapElement_ItemUpColor, raw_data, offset);
		is_valid &= SetField(data.item_down_partial_color, FieldId::NetworkMapElement_ItemDownPartialColor, raw_data, offset);
		is_valid &= SetField(data.item_down_complete_color, FieldId::NetworkMapElement_ItemDownCompleteColor, raw_data, offset);
		is_valid &= SetField(data.item_unknown_color, FieldId::NetworkMapElement_ItemUnknownColor, raw_data, offset);
		is_valid &= SetField(data.item_acked_color, FieldId::NetworkMapElement_ItemAckedColor, raw_data, offset);
		is_valid &= SetField(data.item_shape, FieldId::NetworkMapElement_ItemShape, raw_data, offset);
		is_valid &= SetField(data.item_image, FieldId::NetworkMapElement_ItemImage, raw_data, offset);
		is_valid &= SetField(data.item_image_scale, FieldId::NetworkMapElement_ItemImageScale, raw_data, offset);
		is_valid &= SetField(data.link_from, FieldId::NetworkMapElement_LinkFrom, raw_data, offset);
		is_valid &= SetField(data.link_to, FieldId::NetworkMapElement_LinkTo, raw_data, offset);
		is_valid &= SetField(data.link_id, FieldId::NetworkMapElement_LinkID, raw_data, offset);
		is_valid &= SetField(data.link_width, FieldId::NetworkMapElement_LinkWidth, raw_data, offset);
		is_valid &= SetField(data.object_id, FieldId::SysId, raw_data, offset);
		is_valid &= SetField(data.map_id, FieldId::NetworkMapElement_MapID, raw_data, offset);
		is_valid &= SetField(data.type, FieldId::NetworkMapElement_Type, raw_data, offset);
		is_valid &= SetField(data.item_type, FieldId::NetworkMapElement_ItemType, raw_data, offset);
		is_valid &= SetField(data.item_id, FieldId::NetworkMapElement_ItemID, raw_data, offset);
		is_valid &= SetField(data.item_x, FieldId::NetworkMapElement_ItemX, raw_data, offset);
		is_valid &= SetField(data.item_y, FieldId::NetworkMapElement_ItemY, raw_data, offset);
		is_valid &= SetField(data.label_refresh_interval, FieldId::NetworkMapElement_LabelRefreshInterval, raw_data, offset);
		is_valid &= SetField(data.item_font, FieldId::NetworkMapElement_ItemFont, raw_data, offset);
		is_valid &= SetField(data.name, FieldId::SysName, raw_data, offset);
		is_valid &= ValidateEndOfBlob(raw_data, offset);

		if (!is_valid) {
			return {};
		}

		return data;
	}

	bool DudeDatabase::SetField(BoolField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(BoolField::info);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);

		switch (field.info.type) {
		case FieldType::BoolFalse:
			field.value = false;
			break;
		case FieldType::BoolTrue:
			field.value = true;
			break;
		default:
			printf("Invalid data type, expected 0/1, found %u\n", field.info.type.Value());
			return {};
		}

		return true;
	}

	bool DudeDatabase::SetField(ByteField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(ByteField::info) + sizeof(ByteField::value);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::Byte)) {
			return {};
		}

		return true;
	}

	bool DudeDatabase::SetField(IntField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(IntField::info);

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
			memcpy(&field.value, raw_data.data() + offset, sizeof(ByteField::value));
			offset += sizeof(ByteField::value);
			break;
		default:
			printf("Invalid data type, expected 8/9, found %u\n", field.info.type.Value());
			return {};
		}

		return true;
	}

	bool DudeDatabase::SetField(TimeField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(TimeField::info) + sizeof(TimeField::date);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::Int)) {
			return {};
		}

		return true;
	}

	bool DudeDatabase::SetField(TextField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(TextField::info);

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
			printf("Invalid data type, expected 32/33, found %u\n", field.info.type.Value());
			return {};
		}

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		std::vector<char> raw_text(field.data_size);
		memcpy(raw_text.data(), raw_data.data() + offset, field.data_size);
		field.text = std::string(raw_text.begin(), raw_text.end());
		offset += field.data_size;

		return true;
	}

	bool DudeDatabase::SetField(IntArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(IntArrayField::info) + sizeof(IntArrayField::entries);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::IntArray)) {
			return {};
		}

		if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
			return {};
		}

		field.data.resize(field.entries);
		memcpy(field.data.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return true;
	}

	bool DudeDatabase::SetField(IpAddressField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(IpAddressField::info) + sizeof(IpAddressField::entries);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::IntArray)) {
			return {};
		}

		if (!CheckSize(raw_data.size(), offset, field.entries * sizeof(u32))) {
			return {};
		}

		field.ip_address.resize(field.entries);
		memcpy(field.ip_address.data(), raw_data.data() + offset, field.entries * sizeof(u32));
		offset += field.entries * sizeof(u32);

		return true;
	}

	bool DudeDatabase::SetField(LongArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(LongArrayField::info) + sizeof(LongArrayField::data_size);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::LongArray)) {
			return {};
		}

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		field.data.resize(field.data_size);
		memcpy(field.data.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		return true;
	}

	bool DudeDatabase::SetField(MacAddressField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(MacAddressField::info) + sizeof(MacAddressField::data_size);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::LongArray)) {
			return {};
		}

		if (!CheckSize(raw_data.size(), offset, field.data_size)) {
			return {};
		}

		field.mac_address.resize(field.data_size / sizeof(MacAddress));
		memcpy(field.mac_address.data(), raw_data.data() + offset, field.data_size);
		offset += field.data_size;

		return true;
	}

	bool DudeDatabase::SetField(StringArrayField& field, FieldId id, std::span<const u8> raw_data, std::size_t& offset) const {
		constexpr std::size_t header_size = sizeof(StringArrayField::info) + sizeof(StringArrayField::entry_count);

		if (!CheckSize(raw_data.size(), offset, header_size)) {
			return {};
		}

		memcpy(&field, raw_data.data() + offset, header_size);
		offset += header_size;

		ValidateId(field.info.id.Value(), id);
		if (!ValidateType(field.info.type.Value(), FieldType::StringArray)) {
			return {};
		}

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

		return true;
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
