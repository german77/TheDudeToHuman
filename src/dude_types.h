// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

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
		UnknownE = 0x0e,
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

	enum class FieldType : u32 {
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
		Unknown4E = 0x101f4E,

		Unknown51 = 0x101f51,
		Unk1 = 0x101f52,
		Unk2 = 0x101f53,
		Unk3 = 0x101f54,
		Unknown55 = 0x101f55,

		Pid = 0x101f57,
		CustomField1 = 0x101f58,
		CustomField2 = 0x101f59,
		CustomField3 = 0x101f5a,

		ObjectId = 0xfe0001,
		Name = 0xfe0010,
	};

#pragma pack(push, 1)
	template <typename T>
	struct ObjData {
		u64 magic{};
		ObjectType object_type{ ObjectType::None };
		T data{};
	};

	using RawObjData = ObjData<std::vector<u8>>;

	struct TextField {
		FieldType type{ FieldType::None };
		u8 data_size{};
		std::string text{};
	};

	struct ObjectIdField {
		FieldType type{ FieldType::None };
		u32 id;
	};

	struct UnknownDeviceField1 {
		FieldType type{ FieldType::None };
		u8 entries{};
		INSERT_PADDING_BYTES(0x1);
		std::vector<u32> data{};
	};

	struct UnknownDeviceField2 {
		FieldType type{ FieldType::None };
		u8 data_size{};
		std::string data{};
	};

	struct IpAddressField {
		FieldType type{ FieldType::None };
		u16 entries{};
		u16 data_size{};
		std::vector<IpAddress> ip_address{};
	};

	struct MacAddressField {
		FieldType type{ FieldType::None };
		u8 data_size{};
		std::vector<MacAddress> mac_address{};
	};

	struct DnsField {
		FieldType type{ FieldType::None };
		u16 entries{};
		u16 data_size{};
		std::string dns{};
	};

	// This is type 0x0F data
	struct DeviceData {
		INSERT_PADDING_BYTES(0x6);
		UnknownDeviceField1 unk;
		INSERT_PADDING_BYTES(0x1);
		DnsField dns;
		IpAddressField ip;
		INSERT_PADDING_BYTES(0x3F);
		ObjectIdField object_id;
		UnknownDeviceField2 unk1;
		UnknownDeviceField2 unk2;
		UnknownDeviceField2 unk3;
		TextField custom_field_3;
		TextField custom_field_2;
		TextField custom_field_1;
		TextField password;
		TextField username;
		MacAddressField mac;
		TextField name;
	};
#pragma pack(pop)

}
