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
		Link = 0x29,
		Unknown2a = 0x2a,
		Unknown31 = 0x31,
		Unknown39 = 0x39,
		SnmpProfile = 0x3a,
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
		RouterOs = 0x101f4a,
		Unknown4B = 0x101f4b,
		DeviceTypeId = 0x101f4c,
		Unknown4D = 0x101f4d,
		SnmpProfileId = 0x101f4e,

		Unknown51 = 0x101f51,
		Unknown52 = 0x101f52,
		Unknown53 = 0x101f53,
		Unknown54 = 0x101f54,
		Unknown55 = 0x101f55,
		Unknown56 = 0x101f56,
		Unknown57 = 0x101f57,
		CustomField1 = 0x101f58,
		CustomField2 = 0x101f59,
		CustomField3 = 0x101f5a,

		RequiredServices = 0x102710,
		AllowedServices = 0x102711,
		IgnoredServices = 0x102712,
		ImageId = 0x102713,
		Scale = 0x102714,
		UrlAddress = 0x102715,

		Unknown5DCC = 0x105dcc,
		Unknown5DCE = 0x105dce,
		Unknown5DCD = 0x105dcd,
		Unknown5DCF = 0x105dcf,
		Unknown5DD0 = 0x105dd0,
		Unknown5DD1 = 0x105dd1,
		Unknown5DDE = 0x105dde,
		Unknown5DC8 = 0x105dc8,
		Unknown5DC9 = 0x105dc9,
		Unknown5DCA = 0x105dca,
		Unknown5DCB = 0x105dcb,
		Unknown5DD2 = 0x105dd2,
		Unknown5DD3 = 0x105dd3,
		Unknown5DD4 = 0x105dd4,
		Unknown5DD5 = 0x105dd5,
		Unknown5DD6 = 0x105dd6,
		Unknown5DD7 = 0x105dd7,
		Unknown5DD9 = 0x105dd9,
		Unknown5DDA = 0x105dda,
		Unknown5DDB = 0x105ddb,
		Unknown5DDC = 0x105ddc,
		Unknown5DDD = 0x105ddd,
		Unknown5DDF = 0x105ddf,
		Unknown5DC0 = 0x105dc0,
		Unknown5DC2 = 0x105dc2,
		Unknown5DC3 = 0x105dc3,
		Unknown5DC4 = 0x105dc4,
		Unknown5DC5 = 0x105dc5,
		Unknown5DD8 = 0x105dd8,
		Unknown5DC6 = 0x105dc6,
		Unknown5DC7 = 0x105dc7,

		SnmpVersion = 0x113c68,
		Community = 0x113c69,
		Port = 0x113c6a,
		Unknown6B = 0x113c6b,
		Unknown6C = 0x113c6c,
		CryptPassword = 0x113c6d,
		Unknown6E = 0x113c6e,
		AuthPassword = 0x113c6f,
		Tries = 0x113c71,
		TryTimeout = 0x113c72,

		ObjectId = 0xfe0001,
		SecondaryObjectId = 0xfe0005,
		Name = 0xfe0010,
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

	// This is type 0x0E data
	struct DeviceTypeData {
		IntArrayField ignored_services;
		IntArrayField allowed_services;
		IntArrayField required_services;
		IntField image_id;
		ByteField scale;
		IntField object_id;
		IntField secondary_object_id;
		TextField url;
		TextField name;
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
		IntField snmp_profile_id;
		IntField object_id;
		IntField unk14;
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

	// This is type 0x29 data
	struct LinkData {
		BoolField unk1;
		IntField unk2;
		ByteField unk3;
		ByteField unk4;
		IntField object_id;
		ByteField unk6;
		ByteField unk7;
		ByteField unk8;
		ByteField unk9;
		TextField unk10;
		TextField unit;
		TextField name;
	};

	// This is type 0x3A data
	struct SnmpProfileData {
		IntField version;
		IntField port;
		ByteField unk3; // Security
		ByteField unk4; // AuthMethod
		ByteField unk5; // CryptMethod
		ByteField tries;
		IntField try_timeout;
		IntField object_id;
		TextField auth_password;
		TextField crypt_password;
		TextField community;
		TextField name;
	};

	// This is type 0x4A data
	struct Unknown4aData {
		BoolField unk1;
		BoolField unk2;
		BoolField unk3;
		BoolField unk4;
		BoolField unk5;
		BoolField unk6;
		BoolField unk7;
		BoolField unk8;
		BoolField unk9;
		BoolField unk10;
		BoolField unk11;
		IntField unk12;
		IntField unk13;
		IntField unk14;
		IntField unk15;
		IntField unk16;
		ByteField unk17;
		IntField unk18;
		ByteField unk19;
		IntField unk20;
		IntField unk21;
		IntField unk22;
		ByteField unk23;
		IntField object_id;
		IntField unk25;
		ByteField unk26;
		ByteField unk27;
		IntField unk28;
		IntField unk29;
		IntField unk30;
		IntField unk31;
		LongArrayField unk32;
		TextField name;
	};
#pragma pack(pop)

}
