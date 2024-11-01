// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"

namespace Database {
	using IpAddress = std::array<u8, 4>;
	using MacAddress = std::array<u8, 6>;

	enum class ObjectType : u32 {
		None,
		Unknown1 = 0x01,
		Unknown3 = 0x03,
		Unknown4 = 0x04,
		Unknown5 = 0x05,
		Unknown9 = 0x09,
		UnknownA = 0x0a,
		UnknownD = 0x0d,
		DeviceType = 0x0e,
		Device = 0x0f,
		Unknown10 = 0x10,
		Unknown11 = 0x11,
		Unknown18 = 0x18,
		Unknown22 = 0x22,
		Unknown29 = 0x29,
		Unknown2a = 0x2a,
		Unknown31 = 0x31,
		Unknown39 = 0x39,
		Unknown3a = 0x3a,
		Unknown3b = 0x3b,
		Unknown43 = 0x43,
		Unknown4a = 0x4a,
		Unknown4b = 0x4b,
		Unknown3d = 0x4d,
	};

	enum class FieldId : u32 {
		None,
		IpAddress = 0x101f40,
		DnsNames = 0x101f41,
		Unknown42 = 0x101f42,
		DnsLookupInterval = 0x101f43,
		MacAddress = 0x101f44,
		Unknown45 = 0x101f45,
		Username = 0x101f46,
		Password = 0x101f47,
		Unknown49 = 0x101f49,
		RouterOs = 0x101f4A,
		Unknown4B = 0x101f4B,
		DeviceTypeId = 0x101f4C,
		Unknown4D = 0x101f4D,
		SnmpProfileId = 0x101f4E,

		Unknown51 = 0x101f51,
		Unknown52 = 0x101f52,
		Unknown53 = 0x101f53,
		Unknown54 = 0x101f54,
		Unknown55 = 0x101f55,
		Unknown56 = 0x101f56,
		Pid = 0x101f57,
		CustomField1 = 0x101f58,
		CustomField2 = 0x101f59,
		CustomField3 = 0x101f5a,

		RequiredServices = 0x102710,
		AllowedServices = 0x102711,
		IgnoredServices = 0x102712,
		Image = 0x102713,
		Scale = 0x102714,
		UrlAddress = 0x102715,

		ObjectId = 0xfe0001,
		UnknownFE0005 = 0xfe0005,
		Name = 0xfe0010,

	};

	enum class FieldType : u32 {
		BoolFalse = 0x00,
		BoolTrue = 0x01,
		Int = 0x08,
		Byte = 0x09,
		Long = 0x10,
		ShortString = 0x21,
		LongArray = 0x31,
		IntArray = 0x88,
		StringArray = 0xA0,
	};

#pragma pack(push, 1)
	template <typename T>
	struct ObjData {
		u64 magic{};
		ObjectType object_type{ ObjectType::None };
		T data{};
	};

	using RawObjData = ObjData<std::vector<u8>>;

	struct FieldInfo {
		union {
			u32 raw{};

			BitField<0, 24, FieldId> id;
			BitField<24, 8, FieldType> type;
		};
	};

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

	// This is FieldType::ShortString
	struct TextField {
		FieldInfo info{};
		u8 data_size{};
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

	// This is type 0x0F data
	struct DeviceData {
		IntArrayField unk1;
		IntArrayField unk2;
		StringArrayField dns;
		IpAddressField ip;
		BoolField unk3;
		BoolField router_os;
		BoolField unk5;
		BoolField unk6;
		BoolField unk7;
		ByteField unk8;
		ByteField dns_lookup_interval;
		ByteField unk10;
		IntField device_type_id;
		IntField unk12;
		IntField unk13;
		IntField object_id;
		ByteField unk14;
		ByteField unk15;
		ByteField unk16;
		TextField custom_field_3;
		TextField custom_field_2;
		TextField custom_field_1;
		TextField password;
		TextField username;
		MacAddressField mac;
		TextField name;
	};


	struct DeviceTypeData {

	};
#pragma pack(pop)

}
