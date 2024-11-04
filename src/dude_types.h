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
		ServerConfiguration = 0x03,
		Tool = 0x04,
		File = 0x05,
		Notes = 0x09,
		Map = 0x0a,
		UnknownD = 0x0d,
		DeviceType = 0x0e,
		Device = 0x0f,
		Unknown10 = 0x10,
		Service = 0x11,
		Notification = 0x18,
		Link = 0x1f,
		LinkType = 0x22,
		DataSource = 0x29,
		Unknown2a = 0x2a,
		Unknown31 = 0x31,
		Function = 0x39,
		SnmpProfile = 0x3a,
		Panel = 0x3b,
		Unknown43 = 0x43,
		NetworkMapElement = 0x4a,
		Unknown4b = 0x4b,
		PanelElement = 0x4d,
	};

	enum class FieldType : u32 {
		BoolFalse = 0x00,
		BoolTrue = 0x01,
		Int = 0x08,
		Byte = 0x09,
		Long = 0x10,
		LongLong = 0x18,
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

	// This is FieldType::Long
	struct LongField {
		FieldInfo info;
		u64 value;
	};

	// This is FieldType::LongLong
	struct LongLongField {
		FieldInfo info;
		u128 value;
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

	// This is type 0x03 data
	struct ServerConfigurationData {
		IntArrayField time_zone_history;
		IntArrayField discover_skip_types;
		IntArrayField discover_skip_probes;
		IntArrayField custom_colors;
		IntArrayField chart_line_colors;
		IntArrayField notify_ids;
		BoolField discover_identification;
		BoolField discover_networks;
		BoolField discover_links;
		BoolField map_device_visible;
		BoolField discover_layer_2;
		BoolField first_connection;
		BoolField discover_ppp;
		BoolField discover_graph_services;
		BoolField map_network_visible;
		BoolField discover_graph_links;
		BoolField discover_service_less;
		BoolField map_submap_visible;
		BoolField probe_enabled;
		BoolField map_static_visible;
		BoolField syslog_enabled;
		BoolField map_link_visible;
		BoolField snmp_trap_enabled;
		BoolField confirm_remove;
		BoolField resolve_mac_address_manufacturer;
		BoolField map_dep_visible;
		BoolField map_antialiased_geometry;
		BoolField map_gradients;
		IntField object_id;
		ByteField version;
		IntField snmp_profile_id;
		ByteField agent_id;
		ByteField probe_interval;
		ByteField probe_timeout;
		ByteField probe_down_count;
		IntField syslog_port;
		ByteField snmp_trap_port;
		IntField map_background_color;
		ByteField map_label_refresh_interval;
		IntField map_up_color;
		IntField map_down_partial_color;
		IntField map_down_complete_color;
		IntField map_unknown_color;
		IntField map_acked_color;
		IntField map_network_color;
		IntField map_submap_color;
		IntField map_submap_up_color;
		IntField map_submap_down_partial_color;
		IntField map_submap_down_complete_color;
		IntField map_submap_acked_color;
		IntField map_static_color;
		IntField map_link_color;
		IntField map_link_label_color;
		IntField map_link_full_color;
		ByteField map_device_shape;
		ByteField map_network_shape;
		ByteField map_submap_shape;
		ByteField map_static_shape;
		ByteField map_link_thickness;
		IntField map_dep_color;
		ByteField map_dep_thickness;
		ByteField map_dep_style;
		IntField chart_value_keep_time_raw;
		IntField chart_value_keep_time_10_min;
		IntField chart_value_keep_time_2_hour;
		IntField chart_value_keep_time_1_day;
		IntField chart_background_color;
		IntField chart_grid_color;
		IntField chart_text_color;
		ByteField discover_name_preference;
		ByteField discover_mode;
		ByteField discover_hops;
		ByteField discover_hop_network_size_limit;
		ByteField discover_simultaneous;
		IntField discover_interval;
		ByteField discover_item_width;
		ByteField discover_item_height;
		ByteField discover_big_row;
		ByteField discover_big_column;
		ByteField discover_whole_row;
		ByteField discover_whole_column;
		IntField ros_conn_interval;
		IntField ros_conn_interval_auth_failed;
		ByteField undo_queue_size;
		IntField mac_mapping_refresh_interval;
		ByteField contents_pane_behavior;
		IntField last_chart_maintenance_time;
		TextField discover_black_list;
		LongArrayField report_font;
		LongArrayField chart_font;
		LongArrayField map_link_font;
		TextField map_link_tooltip;
		TextField map_link_label;
		LongArrayField map_static_font;
		LongArrayField map_submap_font;
		TextField map_submap_tooltip;
		TextField map_submap_label;
		LongArrayField map_network_font;
		TextField map_network_tooltip;
		TextField map_network_label;
		LongArrayField map_device_font;
		TextField map_device_tooltip;
		TextField map_device_label;
		LongArrayField unique_id;
		TextField name;
	};

	// This is type 0x04 data
	struct ToolData {
		BoolField builtin;
		ByteField type;
		IntField device_id;
		IntField object_id;
		TextField command;
		TextField name;
	};

	// This is type 0x05 data
	struct FileData {
		IntField parent_id;
		IntField object_id;
		TextField file_name;
		TextField name;
	};

	// This is type 0x09 data
	struct NotesData {
		IntField object_id;
		IntField parent_id;
		TimeField time_added;
		TextField name;
	};

	// This is type 0x0A data
	struct MapData {
		IntArrayField notify_ids;
		BoolField use_static_color;
		BoolField use_link_color;
		BoolField use_link_label_color;
		BoolField use_link_full_color;
		BoolField use_device_label;
		BoolField use_device_shape;
		BoolField use_device_font;
		BoolField use_network_label;
		BoolField use_network_shape;
		BoolField use_network_font;
		BoolField use_submap_label;
		BoolField use_submap_shape;
		BoolField use_submap_font;
		BoolField use_static_shape;
		BoolField use_static_font;
		BoolField use_link_label;
		BoolField use_link_font;
		BoolField use_link_thickness;
		BoolField ordered;
		BoolField prove_enabled;
		BoolField notify_use;
		BoolField report_scanning;
		BoolField locked;
		BoolField image_tile;
		BoolField color_visible;
		BoolField device_visible;
		BoolField network_visible;
		BoolField submap_visible;
		BoolField static_visible;
		BoolField link_visible;
		BoolField use_background_color;
		BoolField use_up_color;
		BoolField use_down_partial_color;
		BoolField use_down_complete_color;
		BoolField use_unknown_color;
		BoolField use_acked_color;
		BoolField use_network_color;
		BoolField use_submap_color;
		BoolField use_submap_up_color;
		BoolField use_submap_down_partial_color;
		BoolField use_submap_down_complete_color;
		BoolField use_submap_acked_color;
		IntField link_thickness;
		IntField layout_density;
		IntField layout_quality;
		IntField prove_interval;
		IntField prove_timeout;
		IntField prove_down_count;
		IntField object_id;
		IntField default_zoom;
		IntField image_id;
		IntField image_scale;
		IntField label_refresh_interval;
		IntField background_color;
		IntField up_color;
		IntField down_partial_color;
		IntField down_complete_color;
		IntField unknown_color;
		IntField acked_color;
		IntField network_color;
		IntField submap_color;
		IntField submap_up_color;
		IntField submap_down_partial_color;
		IntField submap_down_complete_color;
		IntField submap_acked_color;
		IntField static_color;
		IntField link_color;
		IntField link_label_color;
		IntField link_full_color;
		IntField device_shape;
		IntField network_shape;
		IntField submap_shape;
		IntField static_shape;
		LongArrayField link_font;
		TextField link_label;
		LongArrayField static_font;
		LongArrayField submap_font;
		TextField submap_label;
		LongArrayField network_font;
		TextField network_label;
		LongArrayField device_font;
		TextField device_label;
		TextField list_type;
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

	// This is type 0x11 data
	struct ServiceData {
		IntArrayField notify_ids;
		BoolField enabled;
		BoolField history;
		BoolField notify_use;
		BoolField acked;
		IntField probe_port;
		ByteField probe_interval;
		ByteField probe_timeout;
		ByteField probe_down_count;
		IntField data_source_id;
		ByteField status;
		IntField time_since_changed;
		IntField time_since_last_up;
		IntField time_since_last_down;
		IntField time_previous_up;
		IntField time_previous_down;
		ByteField proves_down;
		IntField object_id;
		IntField device_id;
		IntField agent_id;
		IntField prove_id;
		LongField value;
		TextField name;
	};

	// This is type 0x18 data
	struct NotificationData {
		IntArrayField status_list;
		IntArrayField group_notify_ids;
		StringArrayField mail_cc;
		IntArrayField activity;
		BoolField log_use_color;
		BoolField enabled;
		ByteField mail_tls_mode;
		ByteField sys_log_server;
		IntField sys_log_port;
		IntField sound_file_id;
		IntField log_color;
		ByteField speak_rate;
		ByteField speak_volume;
		ByteField delay_interval;
		ByteField repeat_interval;
		ByteField repeat_count;
		IntField object_id;
		IntField rype_id;
		IntField mail_server;
		IntField mail_port;
		TextField log_prefix;
		TextField mail_subject;
		TextField mail_to;
		TextField mail_from;
		TextField mail_password;
		TextField mail_user;
		TextField mail_server_dns;
		LongLongField mail_server6;
		TextField text_template;
		TextField name;
	};

	// This is type 0x1c data
	struct LinkData {
		BoolField history;
		ByteField mastering_type;
		IntField master_device;
		IntField master_interface;
		IntField net_map_id;
		IntField net_map_element_id;
		IntField type_id;
		IntField tx_data_source_id;
		IntField object_id;
		IntField rx_data_source_id;
		LongField speed;
		TextField name;
	};

	// This is type 0x22 data
	struct LinkTypeData {
		IntField object_id;
		ByteField style;
		ByteField thickness;
		IntField snmp_type;
		IntField next_id;
		LongField snmp_speed;
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

	// This is type 0x39 data
	struct FunctionData {
		StringArrayField argument_descriptors;
		BoolField builtin;
		ByteField min_arguments;
		ByteField max_arguments;
		IntField object_id;
		TextField description;
		TextField code;
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

	// This is type 0x3B data
	struct PanelData {
		BoolField ordered;
		BoolField locked;
		BoolField title_bars;
		IntField object_id;
		IntField top_element_id;
		TextField admin;
		TextField type;
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

	// This is type 0x4D data
	struct PanelElementData {
		BoolField split;
		IntField panel_id;
		ByteField split_type;
		IntField split_share;
		IntField first_id;
		IntField second_id;
		IntField obj_id;
		IntField object_id;
		LongArrayField obj_meta;
		TextField name;
	};
}
