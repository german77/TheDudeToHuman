// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "common_funcs.h"
#include "common_types.h"

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

	enum class FieldType : u8 {
		None,
		Unknown = 0x09,
		Text = 0x21,
		Data = 0x31,
	};

#pragma pack(push, 1)
	template <typename T>
	struct ObjData {
		u64 magic{};
		ObjectType object_type{ ObjectType::None };
		T data{};
	};

	using RawObjData = ObjData<std::vector<u8>>;

	template <typename T>
	struct CommonFieldData {
		u8 data_size{};
		std::vector<T> data{};
	};

	template <typename T>
	struct DataField {
		u8 data_id{};
		u16 unknown{};
		FieldType type{ FieldType::None };
		CommonFieldData<T> data{};
	};

	struct UnknownDeviceField {
		u8 entries{};
		INSERT_PADDING_BYTES(0x1);
		std::vector<u32> data{};
	};

	struct DnsField {
		u8 has_dns{};
		INSERT_PADDING_BYTES(0x1);
		u16 data_size{};
		std::vector<char> data{};
	};

	// This is type 0x0F data
	struct DeviceData {
		INSERT_PADDING_BYTES(0xA);
		UnknownDeviceField unk;
		INSERT_PADDING_BYTES(0x5);
		DnsField dns;
		INSERT_PADDING_BYTES(0x6);
		IpAddress ip;
		INSERT_PADDING_BYTES(0x43);
		DataField<char> unk1;
		DataField<char> unk2;
		DataField<char> unk3;
		DataField<char> custom_field_3;
		DataField<char> custom_field_2;
		DataField<char> custom_field_1;
		DataField<char> password;
		DataField<char> user;
		DataField<MacAddress> mac;
		DataField<char> name;
	};
#pragma pack(pop)

}