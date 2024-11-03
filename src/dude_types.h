// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "dude_field_id.h"

namespace Database {
	using IpAddress = std::array<u8, 4>;
	using MacAddress = std::array<u8, 6>;

	enum class DataFormat : u32 {
		None,
		Unknown1 = 0x01,
		Unknown3 = 0x03,
		Unknown4 = 0x04,
		Unknown5 = 0x05,
		Notes = 0x09,
		Map = 0x0a,
		UnknownD = 0x0d,
		DeviceType = 0x0e,
		Device = 0x0f,
		Unknown10 = 0x10,
		Unknown11 = 0x11,
		Unknown18 = 0x18,
		Unknown1f = 0x1f,
		Unknown22 = 0x22,
		DataSource = 0x29,
		Unknown2a = 0x2a,
		Unknown31 = 0x31,
		Unknown39 = 0x39,
		SnmpProfile = 0x3a,
		Unknown3b = 0x3b,
		Unknown43 = 0x43,
		NetworkMapElement = 0x4a,
		Unknown4b = 0x4b,
		Unknown3d = 0x4d,
	};

	enum class FieldType : u32 {
		BoolFalse = 0x00,
		BoolTrue = 0x01,
		Int = 0x08,
		Byte = 0x09,
		Long = 0x10,
		LongString = 0x20,
		ShortString = 0x21,
		LongArray = 0x31,
		IntArray = 0x88,
		StringArray = 0xA0,
	};

	struct FieldInfo {
		union {
			u32 raw{};

			BitField<0, 24, FieldId> id;
			BitField<24, 8, FieldType> type;
		};
	};

#pragma pack(push, 1)
	// This is FieldType::Bool
	struct BoolField {
		FieldInfo info{};
		bool value{};
	};

	// This is FieldType::Byte
	struct ByteField {
		FieldInfo info{};
		u8 value{};
	};

	// This is FieldType::Int
	struct IntField {
		FieldInfo info{};
		u32 value{};
	};

	// This is FieldType::Int
	struct TimeField {
		FieldInfo info;
		u32 date;
	};

	// This is FieldType::ShortString or FieldType::LongString
	struct TextField {
		FieldInfo info{};
		u16 data_size{};
		std::string text{};
	};

	// This is FieldType::IntArray
	struct IntArrayField {
		FieldInfo info{};
		u16 entries{};
		std::vector<u32> data{};
	};

	// This is FieldType::IntArray
	struct IpAddressField {
		FieldInfo info{};
		u16 entries{};
		std::vector<IpAddress> ip_address{};
	};

	// This is FieldType::LongArray
	struct LongArrayField {
		FieldInfo info{};
		u8 data_size{};
		std::vector<u8> data{};
	};

	// This is FieldType::LongArray
	struct MacAddressField {
		FieldInfo info{};
		u8 data_size{};
		std::vector<MacAddress> mac_address{};
	};

	struct StringArrayEntry {
		u16 data_size{};
		std::string text{};
	};

	// This is FieldType::StringArray
	struct StringArrayField {
		FieldInfo info{};
		u16 entry_count{};
		std::vector<StringArrayEntry> entries{};
	};

	struct RawObjData {
		u16 magic{};
		IntArrayField data_format{};
		std::vector<u8> data{};
	};
#pragma pack(pop)

	// This is type 0x09 data
	struct NotesData {
		IntField object_id;
		IntField parent_id;
		TimeField time_added;
		TextField name;
	};

	// This is type 0x0E data
	struct DeviceTypeData {
		IntArrayField ignored_services;
		IntArrayField allowed_services;
		IntArrayField required_services;
		IntField image_id;
		ByteField image_scale;
		IntField object_id;
		IntField next_id;
		TextField url;
		TextField name;
	};

	// This is type 0x0F data
	struct DeviceData {
		IntArrayField parent_ids;
		IntArrayField notify_ids;
		StringArrayField dns_names;
		IpAddressField ip;
		BoolField secure_mode;
		BoolField router_os;
		BoolField dude_server;
		BoolField notify_use;
		BoolField prove_enabled;
		ByteField lookup;
		ByteField dns_lookup_interval;
		ByteField mac_lookup;
		IntField type_id;
		IntField agent_id;
		IntField snmp_profile_id;
		IntField object_id;
		IntField prove_interval;
		ByteField prove_timeout;
		ByteField prove_down_count;
		TextField custom_field_3;
		TextField custom_field_2;
		TextField custom_field_1;
		TextField password;
		TextField username;
		MacAddressField mac;
		TextField name;
	};

	// This is type 0x29 data
	struct DataSourceData {
		BoolField enabled;
		IntField function_device_id;
		ByteField function_interval;
		ByteField data_source_type;
		IntField object_id;
		ByteField keep_time_raw;
		ByteField keep_time_10min;
		ByteField keep_time_2hour;
		ByteField keep_time_1Day;
		TextField function_code;
		TextField unit;
		TextField name;
	};

	// This is type 0x3A data
	struct SnmpProfileData {
		IntField version;
		IntField port;
		ByteField security;
		ByteField auth_method;
		ByteField crypth_method;
		ByteField try_count;
		IntField try_timeout;
		IntField object_id;
		TextField crypt_password;
		TextField auth_password;
		TextField community;
		TextField name;
	};

	// This is type 0x4A data
	struct NetworkMapElementData {
		BoolField item_use_acked_color;
		BoolField item_use_label;
		BoolField item_use_shapes;
		BoolField item_use_font;
		BoolField item_use_image;
		BoolField item_use_image_scale;
		BoolField item_use_width;
		BoolField item_use_up_color;
		BoolField item_use_down_partial_color;
		BoolField item_use_down_complete_color;
		BoolField item_use_unknown_color;
		IntField item_up_color;
		IntField item_down_partial_color;
		IntField item_down_complete_color;
		IntField item_unknown_color;
		IntField item_acked_color;
		ByteField item_shape;
		IntField item_image;
		ByteField item_image_scale;
		IntField link_from;
		IntField link_to;
		IntField link_id;
		ByteField link_width;
		IntField object_id;
		IntField map_id;
		ByteField type;
		ByteField item_type;
		IntField item_id;
		IntField item_x;
		IntField item_y;
		IntField label_refresh_interval;
		LongArrayField item_font;
		TextField name;
	};

}
