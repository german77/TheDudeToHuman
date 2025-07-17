// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <fmt/core.h>

#include "common/bit_field.h"
#include "common/common_funcs.h"
#include "common/common_types.h"
#include "common/string_util.h"
#include "the_dude_to_human/database/dude_field_id.h"

namespace Database {
using IpAddress = std::array<u8, 4>;
using MacAddress = std::array<u8, 6>;

enum class DataFormat : u32 {
    None,
    ServerConfig = 0x03,
    Tool = 0x04,
    File = 0x05,
    Notes = 0x09,
    Map = 0x0a,
    Probe = 0x0d,
    DeviceType = 0x0e,
    Device = 0x0f,
    Network = 0x10,
    Service = 0x11,
    Notification = 0x18,
    Link = 0x1f,
    LinkType = 0x22,
    DataSource = 0x29,
    ObjectList = 0x2a,
    DeviceGroup = 0x31,
    Function = 0x39,
    SnmpProfile = 0x3a,
    Panel = 0x3b,
    SysLogRule = 0x43,
    NetworkMapElement = 0x4a,
    ChartLine = 0x4b,
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

    std::string SerializeJson() const {
        return fmt::format("\"id\":0x{:x}, \"type\":{}", (u32)id.Value(), (u32)type.Value());
    }
};

// This is FieldType::Bool
struct BoolField {
    FieldInfo info{};
    bool value{};

    std::string SerializeJson() const {
        if (value) {
            return "true";
        }
        return "false";
    }
};

// This is FieldType::Byte
struct ByteField {
    FieldInfo info{};
    u8 value{};

    std::string SerializeJson() const {
        return fmt::format("{}", value);
    }
};

// This is FieldType::Int
struct IntField {
    FieldInfo info{};
    u32 value{};

    std::string SerializeJson() const {
        return fmt::format("{}", value);
    }
};

// This is FieldType::Int
struct TimeField {
    FieldInfo info{};
    u32 date{};

    std::string SerializeJson() const {
        return fmt::format("{}", date);
    }
};

// This is FieldType::Long
struct LongField {
    FieldInfo info{};
    u64 value{};

    std::string SerializeJson() const {
        return fmt::format("{}", value);
    }
};

// This is FieldType::LongLong
struct LongLongField {
    FieldInfo info{};
    u128 value{};

    std::string SerializeJson() const {
        return fmt::format("\"0x{:x}{:08x}\"", value[0], value[1]);
    }
};

// This is FieldType::ShortString or FieldType::LongString
struct TextField {
    FieldInfo info{};
    u16 text_size{};
    std::string text{};

    std::string SerializeJson() const {
        return fmt::format("\"{}\"", Common::Sanitize(text, text_size));
    }
};

// This is FieldType::IntArray
struct IntArrayField {
    FieldInfo info{};
    u16 entries{};
    std::vector<u32> data{};

    std::string SerializeJson() const {
        std::string array = "";

        for (u32 entry : data) {
            array += fmt::format("{},", entry);
        }
        if (!data.empty()) {
            array.pop_back();
        }

        return fmt::format("[{}]", array);
    }
};

// This is FieldType::LongArray
struct LongArrayField {
    FieldInfo info{};
    u8 data_size{};
    std::vector<u8> data{};

    std::string SerializeJson() const {
        std::string array = "";

        for (u8 byte : data) {
            array += fmt::format("{},", byte);
        }
        if (!data.empty()) {
            array.pop_back();
        }

        return fmt::format("[{}]", array);
    }
};

// This is FieldType::LongArray
struct MacAddressField {
    FieldInfo info{};
    u8 data_size{};
    std::vector<MacAddress> mac_address{};

    std::string SerializeJson() const {
        std::string array = "";

        for (const MacAddress& mac : mac_address) {
            array += fmt::format("\"{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\",", mac[0], mac[1],
                                 mac[2], mac[3], mac[4], mac[5]);
        }
        if (!mac_address.empty()) {
            array.pop_back();
        }

        return fmt::format("[{}]", array);
    }
};

struct StringArrayEntry {
    u16 text_size{};
    std::string text{};
};

// This is FieldType::StringArray
struct StringArrayField {
    FieldInfo info{};
    u16 entry_count{};
    std::vector<StringArrayEntry> entries{};

    std::string SerializeJson() const {
        std::string array = "";

        for (const StringArrayEntry& entry : entries) {
            array += fmt::format("\"{}\",", Common::Sanitize(entry.text, entry.text_size));
        }
        if (!entries.empty()) {
            array.pop_back();
        }

        return fmt::format("[{}]", array);
    }
};

struct DudeObj {
    virtual ~DudeObj() {}
    virtual std::string SerializeJson(bool has_credentials) const {
        return "\"object_id\":-1";
    }
};

// This is type 0x03 data
struct ServerConfigData : DudeObj {
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
    IntField agent_id;
    IntField probe_interval;
    IntField probe_timeout;
    IntField probe_down_count;
    IntField syslog_port;
    IntField snmp_trap_port;
    IntField map_background_color;
    IntField map_label_refresh_interval;
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"timeZoneHistory\":{}, \"discoverSkipTypes\":{}, "
            "\"discoverSkipProbes\":{}, \"customColors\":{}, \"chartLineColors\":{}, "
            "\"notifyIds\":{}, \"discoverIdentification\":{}, \"discoverNetworks\":{}, "
            "\"discoverLinks\":{}, \"mapDeviceVisible\":{}, \"discoverLayer2\":{}, "
            "\"firstConnection\":{}, \"discoverPpp\":{}, \"discoverGraphServices\":{}, "
            "\"mapNetworkVisible\":{}, \"discoverGraphLinks\":{}, \"discoverServiceLess\":{}, "
            "\"mapSubmapVisible\":{}, \"probeEnabled\":{}, \"mapStaticVisible\":{}, "
            "\"syslogEnabled\":{}, \"mapLinkVisible\":{}, \"snmpTrapEnabled\":{}, "
            "\"confirmRemove\":{}, \"resolveMacAddressManufacturer\":{}, \"mapDepVisible\":{}, "
            "\"mapAntialiasedGeometry\":{}, \"mapGradients\":{}, \"version\":{}, "
            "\"snmpProfileId\":{}, \"agentId\":{}, \"probeInterval\":{}, \"probeTimeout\":{}, "
            "\"probeDownCount\":{}, \"syslogPort\":{}, \"snmpTrapPort\":{}, "
            "\"mapBackgroundColor\":{}, \"mapLabelRefreshInterval\":{}, \"mapUpColor\":{}, "
            "\"mapDownPartialColor\":{}, \"mapDownCompleteColor\":{}, \"mapUnknownColor\":{}, "
            "\"mapAckedColor\":{}, \"mapNetworkColor\":{}, \"mapSubmapColor\":{}, "
            "\"mapSubmapUpColor\":{}, \"mapSubmapDownPartialColor\":{}, "
            "\"mapSubmapDownCompleteColor\":{}, \"mapSubmapAckedColor\":{}, \"mapStaticColor\":{}, "
            "\"mapLinkColor\":{}, \"mapLinkLabelColor\":{}, \"mapLinkFullColor\":{}, "
            "\"mapDeviceShape\":{}, \"mapNetworkShape\":{}, \"mapSubmapShape\":{}, "
            "\"mapStaticShape\":{}, \"mapLinkThickness\":{}, \"mapDepColor\":{}, "
            "\"mapDepThickness\":{}, \"mapDepStyle\":{}, \"chartValueKeepTimeRaw\":{}, "
            "\"chartValueKeepTime10Min\":{}, \"chartValueKeepTime2Hour\":{}, "
            "\"chartValueKeepTime1Day\":{}, \"chartBackgroundColor\":{}, \"chartGridColor\":{}, "
            "\"chartTextColor\":{}, \"discoverNamePreference\":{}, \"discoverMode\":{}, "
            "\"discoverHops\":{}, \"discoverHopNetworkSizeLimit\":{}, \"discoverSimultaneous\":{}, "
            "\"discoverInterval\":{}, \"discoverItemWidth\":{}, \"discoverItemHeight\":{}, "
            "\"discoverBigRow\":{}, \"discoverBigColumn\":{}, \"discoverWholeRow\":{}, "
            "\"discoverWholeColumn\":{}, \"rosConnInterval\":{}, \"rosConnIntervalAuthFailed\":{}, "
            "\"undoQueueSize\":{}, \"macMappingRefreshInterval\":{}, \"contentsPaneBehavior\":{}, "
            "\"lastChartMaintenanceTime\":{}, \"discoverBlackList\":{}, \"reportFont\":{}, "
            "\"chartFont\":{}, \"mapLinkFont\":{}, \"mapLinkTooltip\":{}, \"mapLinkLabel\":{}, "
            "\"mapStaticFont\":{}, \"mapSubmapFont\":{}, \"mapSubmapTooltip\":{}, "
            "\"mapSubmapLabel\":{}, \"mapNetworkFont\":{}, \"mapNetworkTooltip\":{}, "
            "\"mapNetworkLabel\":{}, \"mapDeviceFont\":{}, \"mapDeviceTooltip\":{}, "
            "\"mapDeviceLabel\":{}, \"uniqueId\":{}",
            object_id.SerializeJson(), name.SerializeJson(), time_zone_history.SerializeJson(),
            discover_skip_types.SerializeJson(), discover_skip_probes.SerializeJson(),
            custom_colors.SerializeJson(), chart_line_colors.SerializeJson(),
            notify_ids.SerializeJson(), discover_identification.SerializeJson(),
            discover_networks.SerializeJson(), discover_links.SerializeJson(),
            map_device_visible.SerializeJson(), discover_layer_2.SerializeJson(),
            first_connection.SerializeJson(), discover_ppp.SerializeJson(),
            discover_graph_services.SerializeJson(), map_network_visible.SerializeJson(),
            discover_graph_links.SerializeJson(), discover_service_less.SerializeJson(),
            map_submap_visible.SerializeJson(), probe_enabled.SerializeJson(),
            map_static_visible.SerializeJson(), syslog_enabled.SerializeJson(),
            map_link_visible.SerializeJson(), snmp_trap_enabled.SerializeJson(),
            confirm_remove.SerializeJson(), resolve_mac_address_manufacturer.SerializeJson(),
            map_dep_visible.SerializeJson(), map_antialiased_geometry.SerializeJson(),
            map_gradients.SerializeJson(), version.SerializeJson(), snmp_profile_id.SerializeJson(),
            agent_id.SerializeJson(), probe_interval.SerializeJson(), probe_timeout.SerializeJson(),
            probe_down_count.SerializeJson(), syslog_port.SerializeJson(),
            snmp_trap_port.SerializeJson(), map_background_color.SerializeJson(),
            map_label_refresh_interval.SerializeJson(), map_up_color.SerializeJson(),
            map_down_partial_color.SerializeJson(), map_down_complete_color.SerializeJson(),
            map_unknown_color.SerializeJson(), map_acked_color.SerializeJson(),
            map_network_color.SerializeJson(), map_submap_color.SerializeJson(),
            map_submap_up_color.SerializeJson(), map_submap_down_partial_color.SerializeJson(),
            map_submap_down_complete_color.SerializeJson(), map_submap_acked_color.SerializeJson(),
            map_static_color.SerializeJson(), map_link_color.SerializeJson(),
            map_link_label_color.SerializeJson(), map_link_full_color.SerializeJson(),
            map_device_shape.SerializeJson(), map_network_shape.SerializeJson(),
            map_submap_shape.SerializeJson(), map_static_shape.SerializeJson(),
            map_link_thickness.SerializeJson(), map_dep_color.SerializeJson(),
            map_dep_thickness.SerializeJson(), map_dep_style.SerializeJson(),
            chart_value_keep_time_raw.SerializeJson(), chart_value_keep_time_10_min.SerializeJson(),
            chart_value_keep_time_2_hour.SerializeJson(),
            chart_value_keep_time_1_day.SerializeJson(), chart_background_color.SerializeJson(),
            chart_grid_color.SerializeJson(), chart_text_color.SerializeJson(),
            discover_name_preference.SerializeJson(), discover_mode.SerializeJson(),
            discover_hops.SerializeJson(), discover_hop_network_size_limit.SerializeJson(),
            discover_simultaneous.SerializeJson(), discover_interval.SerializeJson(),
            discover_item_width.SerializeJson(), discover_item_height.SerializeJson(),
            discover_big_row.SerializeJson(), discover_big_column.SerializeJson(),
            discover_whole_row.SerializeJson(), discover_whole_column.SerializeJson(),
            ros_conn_interval.SerializeJson(), ros_conn_interval_auth_failed.SerializeJson(),
            undo_queue_size.SerializeJson(), mac_mapping_refresh_interval.SerializeJson(),
            contents_pane_behavior.SerializeJson(), last_chart_maintenance_time.SerializeJson(),
            discover_black_list.SerializeJson(), report_font.SerializeJson(),
            chart_font.SerializeJson(), map_link_font.SerializeJson(),
            map_link_tooltip.SerializeJson(), map_link_label.SerializeJson(),
            map_static_font.SerializeJson(), map_submap_font.SerializeJson(),
            map_submap_tooltip.SerializeJson(), map_submap_label.SerializeJson(),
            map_network_font.SerializeJson(), map_network_tooltip.SerializeJson(),
            map_network_label.SerializeJson(), map_device_font.SerializeJson(),
            map_device_tooltip.SerializeJson(), map_device_label.SerializeJson(),
            unique_id.SerializeJson());
    }
};

// This is type 0x04 data
struct ToolData : DudeObj {
    BoolField builtin;
    ByteField type;
    IntField device_id;
    IntField object_id;
    TextField command;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"builtin\":{}, \"type\":{}, "
                           "\"deviceId\":{}, \"command\":{}",
                           object_id.SerializeJson(), name.SerializeJson(), builtin.SerializeJson(),
                           type.SerializeJson(), device_id.SerializeJson(),
                           command.SerializeJson());
    }
};

// This is type 0x05 data
struct FileData : DudeObj {
    IntField parent_id;
    IntField object_id;
    TextField file_name;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"parentId\":{}, \"fileName\":{}",
                           object_id.SerializeJson(), name.SerializeJson(),
                           parent_id.SerializeJson(), file_name.SerializeJson());
    }
};

// This is type 0x09 data
struct NotesData : DudeObj {
    IntField object_id;
    IntField parent_id;
    TimeField time_added;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"parentId\":{}, \"timeAdded\":{}",
                           object_id.SerializeJson(), name.SerializeJson(),
                           parent_id.SerializeJson(), time_added.SerializeJson());
    }
};

// This is type 0x0A data
struct MapData : DudeObj {
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"notifyIds\":{}, \"useStaticColor\":{}, "
            "\"useLinkColor\":{}, \"useLinkLabelColor\":{}, \"useLinkFullColor\":{}, "
            "\"useDeviceLabel\":{}, \"useDeviceShape\":{}, \"useDeviceFont\":{}, "
            "\"useNetworkLabel\":{}, \"useNetworkShape\":{}, \"useNetworkFont\":{}, "
            "\"useSubmapLabel\":{}, \"useSubmapShape\":{}, \"useSubmapFont\":{}, "
            "\"useStaticShape\":{}, \"useStaticFont\":{}, \"useLinkLabel\":{}, \"useLinkFont\":{}, "
            "\"useLinkThickness\":{}, \"ordered\":{}, \"proveEnabled\":{}, \"notifyUse\":{}, "
            "\"reportScanning\":{}, \"locked\":{}, \"imageTile\":{}, \"colorVisible\":{}, "
            "\"deviceVisible\":{}, \"networkVisible\":{}, \"submapVisible\":{}, "
            "\"staticVisible\":{}, \"linkVisible\":{}, \"useBackgroundColor\":{}, "
            "\"useUpColor\":{}, \"useDownPartialColor\":{}, \"useDownCompleteColor\":{}, "
            "\"useUnknownColor\":{}, \"useAckedColor\":{}, \"useNetworkColor\":{}, "
            "\"useSubmapColor\":{}, \"useSubmapUpColor\":{}, \"useSubmapDownPartialColor\":{}, "
            "\"useSubmapDownCompleteColor\":{}, \"useSubmapAckedColor\":{}, \"linkThickness\":{}, "
            "\"layoutDensity\":{}, \"layoutQuality\":{}, \"proveInterval\":{}, "
            "\"proveTimeout\":{}, \"proveDownCount\":{}, \"defaultZoom\":{}, \"imageId\":{}, "
            "\"imageScale\":{}, \"labelRefreshInterval\":{}, \"backgroundColor\":{}, "
            "\"upColor\":{}, \"downPartialColor\":{}, \"downCompleteColor\":{}, "
            "\"unknownColor\":{}, \"ackedColor\":{}, \"networkColor\":{}, \"submapColor\":{}, "
            "\"submapUpColor\":{}, \"submapDownPartialColor\":{}, \"submapDownCompleteColor\":{}, "
            "\"submapAckedColor\":{}, \"staticColor\":{}, \"linkColor\":{}, \"linkLabelColor\":{}, "
            "\"linkFullColor\":{}, \"deviceShape\":{}, \"networkShape\":{}, \"submapShape\":{}, "
            "\"staticShape\":{}, \"linkFont\":{}, \"linkLabel\":{}, \"staticFont\":{}, "
            "\"submapFont\":{}, \"submapLabel\":{}, \"networkFont\":{}, \"networkLabel\":{}, "
            "\"deviceFont\":{}, \"deviceLabel\":{}, \"listType\":{}",
            object_id.SerializeJson(), name.SerializeJson(), notify_ids.SerializeJson(),
            use_static_color.SerializeJson(), use_link_color.SerializeJson(),
            use_link_label_color.SerializeJson(), use_link_full_color.SerializeJson(),
            use_device_label.SerializeJson(), use_device_shape.SerializeJson(),
            use_device_font.SerializeJson(), use_network_label.SerializeJson(),
            use_network_shape.SerializeJson(), use_network_font.SerializeJson(),
            use_submap_label.SerializeJson(), use_submap_shape.SerializeJson(),
            use_submap_font.SerializeJson(), use_static_shape.SerializeJson(),
            use_static_font.SerializeJson(), use_link_label.SerializeJson(),
            use_link_font.SerializeJson(), use_link_thickness.SerializeJson(),
            ordered.SerializeJson(), prove_enabled.SerializeJson(), notify_use.SerializeJson(),
            report_scanning.SerializeJson(), locked.SerializeJson(), image_tile.SerializeJson(),
            color_visible.SerializeJson(), device_visible.SerializeJson(),
            network_visible.SerializeJson(), submap_visible.SerializeJson(),
            static_visible.SerializeJson(), link_visible.SerializeJson(),
            use_background_color.SerializeJson(), use_up_color.SerializeJson(),
            use_down_partial_color.SerializeJson(), use_down_complete_color.SerializeJson(),
            use_unknown_color.SerializeJson(), use_acked_color.SerializeJson(),
            use_network_color.SerializeJson(), use_submap_color.SerializeJson(),
            use_submap_up_color.SerializeJson(), use_submap_down_partial_color.SerializeJson(),
            use_submap_down_complete_color.SerializeJson(), use_submap_acked_color.SerializeJson(),
            link_thickness.SerializeJson(), layout_density.SerializeJson(),
            layout_quality.SerializeJson(), prove_interval.SerializeJson(),
            prove_timeout.SerializeJson(), prove_down_count.SerializeJson(),
            default_zoom.SerializeJson(), image_id.SerializeJson(), image_scale.SerializeJson(),
            label_refresh_interval.SerializeJson(), background_color.SerializeJson(),
            up_color.SerializeJson(), down_partial_color.SerializeJson(),
            down_complete_color.SerializeJson(), unknown_color.SerializeJson(),
            acked_color.SerializeJson(), network_color.SerializeJson(),
            submap_color.SerializeJson(), submap_up_color.SerializeJson(),
            submap_down_partial_color.SerializeJson(), submap_down_complete_color.SerializeJson(),
            submap_acked_color.SerializeJson(), static_color.SerializeJson(),
            link_color.SerializeJson(), link_label_color.SerializeJson(),
            link_full_color.SerializeJson(), device_shape.SerializeJson(),
            network_shape.SerializeJson(), submap_shape.SerializeJson(),
            static_shape.SerializeJson(), link_font.SerializeJson(), link_label.SerializeJson(),
            static_font.SerializeJson(), submap_font.SerializeJson(), submap_label.SerializeJson(),
            network_font.SerializeJson(), network_label.SerializeJson(),
            device_font.SerializeJson(), device_label.SerializeJson(), list_type.SerializeJson());
    }
};

// This is type 0x0D data
struct ProbeData : DudeObj {
    IntArrayField logic_probe_ids;
    IntArrayField snmp_value_oid;
    IntArrayField snmp_oid;
    IntArrayField dns_addresses;
    BoolField snmp_avail_if_up;
    BoolField tcp_only_connect;
    BoolField tcp_first_receive;
    ByteField logic_type;
    IntField type_id;
    IntField object_id;
    IntField agent_id;
    IntField default_port;
    ByteField icmp_size;
    ByteField icmp_retry_count;
    IntField icmp_retry_interval;
    ByteField random_probability;
    ByteField icmp_ttl;
    IntField snmp_profile_id;
    ByteField snmp_oid_type;
    ByteField snmp_compare_method;
    ByteField snmp_value_number;
    ByteField snmp_value_ip;
    TextField function_unit;
    TextField funtion_value;
    TextField function_error;
    TextField function_available;
    TextField snmp_value_string;
    LongField snmp_value_big_number;
    TextField dns_name;
    TextField tcp_receive_3;
    TextField tcp_send_3;
    TextField tcp_receive_2;
    TextField tcp_send_2;
    TextField tcp_receive_1;
    TextField tcp_send_1;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"logicProbeIds\":{}, \"snmpValueOid\":{}, "
            "\"snmpOid\":{}, \"dnsAddresses\":{}, \"snmpAvailIfUp\":{}, \"tcpOnlyConnect\":{}, "
            "\"tcpFirstReceive\":{}, \"logicType\":{}, \"typeId\":{}, \"agentId\":{}, "
            "\"defaultPort\":{}, \"icmpSize\":{}, \"icmpRetryCount\":{}, \"icmpRetryInterval\":{}, "
            "\"randomProbability\":{}, \"icmpTtl\":{}, \"snmpProfileId\":{}, \"snmpOidType\":{}, "
            "\"snmpCompareMethod\":{}, \"snmpValueNumber\":{}, \"snmpValueIp\":{}, "
            "\"functionUnit\":{}, \"funtionValue\":{}, \"functionError\":{}, "
            "\"functionAvailable\":{}, \"snmpValueString\":{}, \"snmpValueBigNumber\":{}, "
            "\"dnsName\":{}, \"tcpReceive3\":{}, \"tcpSend3\":{}, \"tcpReceive2\":{}, "
            "\"tcpSend2\":{}, \"tcpReceive1\":{}, \"tcpSend1\":{}",
            object_id.SerializeJson(), name.SerializeJson(), logic_probe_ids.SerializeJson(),
            snmp_value_oid.SerializeJson(), snmp_oid.SerializeJson(), dns_addresses.SerializeJson(),
            snmp_avail_if_up.SerializeJson(), tcp_only_connect.SerializeJson(),
            tcp_first_receive.SerializeJson(), logic_type.SerializeJson(), type_id.SerializeJson(),
            agent_id.SerializeJson(), default_port.SerializeJson(), icmp_size.SerializeJson(),
            icmp_retry_count.SerializeJson(), icmp_retry_interval.SerializeJson(),
            random_probability.SerializeJson(), icmp_ttl.SerializeJson(),
            snmp_profile_id.SerializeJson(), snmp_oid_type.SerializeJson(),
            snmp_compare_method.SerializeJson(), snmp_value_number.SerializeJson(),
            snmp_value_ip.SerializeJson(), function_unit.SerializeJson(),
            funtion_value.SerializeJson(), // Using "funtion" to match struct
            function_error.SerializeJson(), function_available.SerializeJson(),
            snmp_value_string.SerializeJson(), snmp_value_big_number.SerializeJson(),
            dns_name.SerializeJson(), tcp_receive_3.SerializeJson(), tcp_send_3.SerializeJson(),
            tcp_receive_2.SerializeJson(), tcp_send_2.SerializeJson(),
            tcp_receive_1.SerializeJson(), tcp_send_1.SerializeJson());
    }
};

// This is type 0x0E data
struct DeviceTypeData : DudeObj {
    IntArrayField ignored_services;
    IntArrayField allowed_services;
    IntArrayField required_services;
    IntField image_id;
    ByteField image_scale;
    IntField object_id;
    IntField next_id;
    TextField url;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"ignoredServices\":{}, \"allowedServices\":{}, "
            "\"requiredServices\":{}, \"imageId\":{}, \"imageScale\":{}, \"nextId\":{}, \"url\":{}",
            object_id.SerializeJson(), name.SerializeJson(), ignored_services.SerializeJson(),
            allowed_services.SerializeJson(), required_services.SerializeJson(),
            image_id.SerializeJson(), image_scale.SerializeJson(), next_id.SerializeJson(),
            url.SerializeJson());
    }
};

// This is type 0x0F data
struct DeviceData : DudeObj {
    IntArrayField parent_ids;
    IntArrayField notify_ids;
    StringArrayField dns_names;
    IntArrayField ip;
    BoolField secure_mode;
    BoolField router_os;
    BoolField dude_server;
    BoolField notify_use;
    BoolField prove_enabled;
    ByteField lookup;
    IntField dns_lookup_interval;
    ByteField mac_lookup;
    IntField type_id;
    IntField agent_id;
    IntField snmp_profile_id;
    IntField object_id;
    IntField prove_interval;
    IntField prove_timeout;
    IntField prove_down_count;
    TextField custom_field_3;
    TextField custom_field_2;
    TextField custom_field_1;
    TextField password;
    TextField username;
    MacAddressField mac;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"parentIds\":{}, \"notifyIds\":{}, \"dnsNames\":{}, "
            "\"ip\":{}, \"secureMode\":{}, \"routerOs\":{}, \"dudeServer\":{}, \"notifyUse\":{}, "
            "\"proveEnabled\":{}, \"lookup\":{}, \"dnsLookupInterval\":{}, \"macLookup\":{}, "
            "\"typeId\":{}, \"agentId\":{}, \"snmpProfileId\":{}, \"proveInterval\":{}, "
            "\"proveTimeout\":{}, \"proveDownCount\":{}, \"customField3\":{}, \"customField2\":{}, "
            "\"customField1\":{}, \"password\":{}, \"username\":{}, \"mac\":{}",
            object_id.SerializeJson(), name.SerializeJson(), parent_ids.SerializeJson(),
            notify_ids.SerializeJson(), dns_names.SerializeJson(), ip.SerializeJson(),
            secure_mode.SerializeJson(), router_os.SerializeJson(), dude_server.SerializeJson(),
            notify_use.SerializeJson(), prove_enabled.SerializeJson(), lookup.SerializeJson(),
            dns_lookup_interval.SerializeJson(), mac_lookup.SerializeJson(),
            type_id.SerializeJson(), agent_id.SerializeJson(), snmp_profile_id.SerializeJson(),
            prove_interval.SerializeJson(), prove_timeout.SerializeJson(),
            prove_down_count.SerializeJson(), custom_field_3.SerializeJson(),
            custom_field_2.SerializeJson(), custom_field_1.SerializeJson(),
            has_credentials ? password.SerializeJson() : "\"*****\"",
            has_credentials ? username.SerializeJson() : "\"*****\"", mac.SerializeJson());
    }
};

// This is type 0x10 data
struct NetworkData : DudeObj {
    IntArrayField subnets;
    IntField object_id;
    IntField net_map_id;
    IntField net_map_element;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"subnets\":{}, \"netMapId\":{}, \"netMapElement\":{}",
            object_id.SerializeJson(), name.SerializeJson(), subnets.SerializeJson(),
            net_map_id.SerializeJson(), net_map_element.SerializeJson());
    }
};

// This is type 0x11 data
struct ServiceData : DudeObj {
    IntArrayField notify_ids;
    BoolField enabled;
    BoolField history;
    BoolField notify_use;
    BoolField acked;
    IntField probe_port;
    IntField probe_interval;
    IntField probe_timeout;
    IntField probe_down_count;
    IntField data_source_id;
    ByteField status;
    IntField time_since_changed;
    IntField time_since_last_up;
    IntField time_since_last_down;
    IntField time_previous_up;
    IntField time_previous_down;
    IntField proves_down;
    IntField object_id;
    IntField device_id;
    IntField agent_id;
    IntField prove_id;
    LongField value;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"notifyIds\":{}, \"enabled\":{}, \"history\":{}, "
            "\"notifyUse\":{}, \"acked\":{}, \"probePort\":{}, \"probeInterval\":{}, "
            "\"probeTimeout\":{}, \"probeDownCount\":{}, \"dataSourceId\":{}, \"status\":{}, "
            "\"timeSinceChanged\":{}, \"timeSinceLastUp\":{}, \"timeSinceLastDown\":{}, "
            "\"timePreviousUp\":{}, \"timePreviousDown\":{}, \"provesDown\":{}, \"deviceId\":{}, "
            "\"agentId\":{}, \"proveId\":{}, \"value\":{}",
            object_id.SerializeJson(), name.SerializeJson(), notify_ids.SerializeJson(),
            enabled.SerializeJson(), history.SerializeJson(), notify_use.SerializeJson(),
            acked.SerializeJson(), probe_port.SerializeJson(), probe_interval.SerializeJson(),
            probe_timeout.SerializeJson(), probe_down_count.SerializeJson(),
            data_source_id.SerializeJson(), status.SerializeJson(),
            time_since_changed.SerializeJson(), time_since_last_up.SerializeJson(),
            time_since_last_down.SerializeJson(), time_previous_up.SerializeJson(),
            time_previous_down.SerializeJson(), proves_down.SerializeJson(),
            device_id.SerializeJson(), agent_id.SerializeJson(), prove_id.SerializeJson(),
            value.SerializeJson());
    }
};

// This is type 0x18 data
struct NotificationData : DudeObj {
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
    IntField delay_interval;
    IntField repeat_interval;
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"statusList\":{}, \"groupNotifyIds\":{}, "
            "\"mailCc\":{}, \"activity\":{}, \"logUseColor\":{}, \"enabled\":{}, "
            "\"mailTlsMode\":{}, \"sysLogServer\":{}, \"sysLogPort\":{}, \"soundFileId\":{}, "
            "\"logColor\":{}, \"speakRate\":{}, \"speakVolume\":{}, \"delayInterval\":{}, "
            "\"repeatInterval\":{}, \"repeatCount\":{}, \"rypeId\":{}, \"mailServer\":{}, "
            "\"mailPort\":{}, \"logPrefix\":{}, \"mailSubject\":{}, \"mailTo\":{}, "
            "\"mailFrom\":{}, \"mailPassword\":{}, \"mailUser\":{}, \"mailServerDns\":{}, "
            "\"mailServer6\":{}, \"textTemplate\":{}",
            object_id.SerializeJson(), name.SerializeJson(), status_list.SerializeJson(),
            group_notify_ids.SerializeJson(), mail_cc.SerializeJson(), activity.SerializeJson(),
            log_use_color.SerializeJson(), enabled.SerializeJson(), mail_tls_mode.SerializeJson(),
            sys_log_server.SerializeJson(), sys_log_port.SerializeJson(),
            sound_file_id.SerializeJson(), log_color.SerializeJson(), speak_rate.SerializeJson(),
            speak_volume.SerializeJson(), delay_interval.SerializeJson(),
            repeat_interval.SerializeJson(), repeat_count.SerializeJson(), rype_id.SerializeJson(),
            mail_server.SerializeJson(), mail_port.SerializeJson(), log_prefix.SerializeJson(),
            mail_subject.SerializeJson(), mail_to.SerializeJson(), mail_from.SerializeJson(),
            has_credentials ? mail_password.SerializeJson() : "\"*****\"",
            has_credentials ? mail_user.SerializeJson() : "\"*****\"",
            mail_server_dns.SerializeJson(), mail_server6.SerializeJson(),
            text_template.SerializeJson());
    }
};

// This is type 0x1c data
struct LinkData : DudeObj {
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"history\":{}, \"masteringType\":{}, "
            "\"masterDevice\":{}, \"masterInterface\":{}, \"netMapId\":{}, \"netMapElementId\":{}, "
            "\"typeId\":{}, \"txDataSourceId\":{}, \"rxDataSourceId\":{}, \"speed\":{}",
            object_id.SerializeJson(), name.SerializeJson(), history.SerializeJson(),
            mastering_type.SerializeJson(), master_device.SerializeJson(),
            master_interface.SerializeJson(), net_map_id.SerializeJson(),
            net_map_element_id.SerializeJson(), type_id.SerializeJson(),
            tx_data_source_id.SerializeJson(), rx_data_source_id.SerializeJson(),
            speed.SerializeJson());
    }
};

// This is type 0x22 data
struct LinkTypeData : DudeObj {
    IntField object_id;
    ByteField style;
    ByteField thickness;
    IntField snmp_type;
    IntField next_id;
    LongField snmp_speed;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"style\":{}, \"thickness\":{}, "
                           "\"snmpType\":{}, \"nextId\":{}, \"snmpSpeed\":{}",
                           object_id.SerializeJson(), name.SerializeJson(), style.SerializeJson(),
                           thickness.SerializeJson(), snmp_type.SerializeJson(),
                           next_id.SerializeJson(), snmp_speed.SerializeJson());
    }
};

// This is type 0x29 data
struct DataSourceData : DudeObj {
    BoolField enabled;
    IntField function_device_id;
    IntField function_interval;
    ByteField data_source_type;
    IntField object_id;
    IntField keep_time_raw;
    IntField keep_time_10min;
    IntField keep_time_2hour;
    IntField keep_time_1Day;
    TextField function_code;
    TextField unit;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"enabled\":{}, \"functionDeviceId\":{}, "
                           "\"functionInterval\":{}, \"dataSourceType\":{}, \"keepTimeRaw\":{}, "
                           "\"keepTime10min\":{}, \"keepTime2hour\":{}, \"keepTime1Day\":{}, "
                           "\"functionCode\":{}, \"unit\":{}",
                           object_id.SerializeJson(), name.SerializeJson(), enabled.SerializeJson(),
                           function_device_id.SerializeJson(), function_interval.SerializeJson(),
                           data_source_type.SerializeJson(), keep_time_raw.SerializeJson(),
                           keep_time_10min.SerializeJson(), keep_time_2hour.SerializeJson(),
                           keep_time_1Day.SerializeJson(), function_code.SerializeJson(),
                           unit.SerializeJson());
    }
};

// This is type 0x2a data
struct ObjectListData : DudeObj {
    BoolField ordered;
    IntField object_id;
    TextField type;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"ordered\":{}, \"type\":{}",
                           object_id.SerializeJson(), name.SerializeJson(), ordered.SerializeJson(),
                           type.SerializeJson());
    }
};

// This is type 0x31 data
struct DeviceGroupData : DudeObj {
    IntArrayField device_ids;
    IntField object_id;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"deviceIds\":{}",
                           object_id.SerializeJson(), name.SerializeJson(),
                           device_ids.SerializeJson());
    }
};

// This is type 0x39 data
struct FunctionData : DudeObj {
    StringArrayField argument_descriptors;
    BoolField builtin;
    ByteField min_arguments;
    ByteField max_arguments;
    IntField object_id;
    TextField description;
    TextField code;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"argumentDescriptors\":{}, \"builtin\":{}, "
            "\"minArguments\":{}, \"maxArguments\":{}, \"description\":{}, \"code\":{}",
            object_id.SerializeJson(), name.SerializeJson(), argument_descriptors.SerializeJson(),
            builtin.SerializeJson(), min_arguments.SerializeJson(), max_arguments.SerializeJson(),
            description.SerializeJson(), code.SerializeJson());
    }
};

// This is type 0x3A data
struct SnmpProfileData : DudeObj {
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"version\":{}, \"port\":{}, \"security\":{}, "
            "\"authMethod\":{}, \"crypthMethod\":{}, \"tryCount\":{}, \"tryTimeout\":{}, "
            "\"cryptPassword\":{}, \"authPassword\":{}, \"community\":{}",
            object_id.SerializeJson(), name.SerializeJson(), version.SerializeJson(),
            port.SerializeJson(), security.SerializeJson(), auth_method.SerializeJson(),
            crypth_method.SerializeJson(), try_count.SerializeJson(), try_timeout.SerializeJson(),
            crypt_password.SerializeJson(), auth_password.SerializeJson(),
            community.SerializeJson());
    }
};

// This is type 0x3B data
struct PanelData : DudeObj {
    BoolField ordered;
    BoolField locked;
    BoolField title_bars;
    IntField object_id;
    IntField top_element_id;
    TextField admin;
    TextField type;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"ordered\":{}, \"locked\":{}, "
                           "\"titleBars\":{}, \"topElementId\":{}, \"admin\":{}, \"type\":{}",
                           object_id.SerializeJson(), name.SerializeJson(), ordered.SerializeJson(),
                           locked.SerializeJson(), title_bars.SerializeJson(),
                           top_element_id.SerializeJson(), admin.SerializeJson(),
                           type.SerializeJson());
    }
};

// This is type 0x43 data
struct SysLogRuleData : DudeObj {
    BoolField regexp_not;
    BoolField source_set;
    BoolField regexp_set;
    BoolField enabled;
    BoolField source_not;
    IntField source_first;
    IntField source_second;
    ByteField action;
    IntField notify_id;
    IntField object_id;
    IntField next_id;
    TextField regexp;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"regexpNot\":{}, \"sourceSet\":{}, \"regexpSet\":{}, "
            "\"enabled\":{}, \"sourceNot\":{}, \"sourceFirst\":{}, \"sourceSecond\":{}, "
            "\"action\":{}, \"notifyId\":{}, \"nextId\":{}, \"regexp\":{}",
            object_id.SerializeJson(), name.SerializeJson(), regexp_not.SerializeJson(),
            source_set.SerializeJson(), regexp_set.SerializeJson(), enabled.SerializeJson(),
            source_not.SerializeJson(), source_first.SerializeJson(), source_second.SerializeJson(),
            action.SerializeJson(), notify_id.SerializeJson(), next_id.SerializeJson(),
            regexp.SerializeJson());
    }
};

// This is type 0x4A data
struct NetworkMapElementData : DudeObj {
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"itemUseAckedColor\":{}, \"itemUseLabel\":{}, "
            "\"itemUseShapes\":{}, \"itemUseFont\":{}, \"itemUseImage\":{}, "
            "\"itemUseImageScale\":{}, \"itemUseWidth\":{}, \"itemUseUpColor\":{}, "
            "\"itemUse_down_partialColor\":{}, \"itemUseDown_complete_color\":{}, "
            "\"itemUseUnknownColor\":{}, \"itemUpColor\":{}, \"itemDownPartialColor\":{}, "
            "\"itemDownCompleteColor\":{}, \"itemUnknownColor\":{}, \"itemAckedColor\":{}, "
            "\"itemShape\":{}, \"linkFrom\":{}, \"linkTo\":{}, \"linkId\":{}, \"linkWidth\":{}, "
            "\"mapId\":{}, \"type\":{}, \"itemType\":{}, \"itemId\":{}, \"itemX\":{}, "
            "\"itemY\":{}, \"labelRefreshInterval\":{}, \"itemFont\":{}",
            object_id.SerializeJson(), name.SerializeJson(), item_use_acked_color.SerializeJson(),
            item_use_label.SerializeJson(), item_use_shapes.SerializeJson(),
            item_use_font.SerializeJson(), item_use_image.SerializeJson(),
            item_use_image_scale.SerializeJson(), item_use_width.SerializeJson(),
            item_use_up_color.SerializeJson(), item_use_down_partial_color.SerializeJson(),
            item_use_down_complete_color.SerializeJson(), item_use_unknown_color.SerializeJson(),
            item_up_color.SerializeJson(), item_down_partial_color.SerializeJson(),
            item_down_complete_color.SerializeJson(), item_unknown_color.SerializeJson(),
            item_acked_color.SerializeJson(), item_shape.SerializeJson(), link_from.SerializeJson(),
            link_to.SerializeJson(), link_id.SerializeJson(), link_width.SerializeJson(),
            map_id.SerializeJson(), type.SerializeJson(), item_type.SerializeJson(),
            item_id.SerializeJson(), item_x.SerializeJson(), item_y.SerializeJson(),
            label_refresh_interval.SerializeJson(), item_font.SerializeJson());
    }
};

// This is type 0x4B data
struct ChartLineData : DudeObj {
    IntField chart_id;
    IntField source_id;
    ByteField line_style;
    IntField line_color;
    ByteField line_opacity;
    IntField fill_color;
    ByteField fill_opacity;
    IntField object_id;
    IntField next_id;
    TextField name;

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format("\"objectId\":{}, \"name\":{}, \"chartId\":{}, \"sourceId\":{}, "
                           "\"lineStyle\":{}, \"lineColor\":{}, \"lineOpacity\":{}, "
                           "\"fillColor\":{}, \"fillOpacity\":{}, \"nextId\":{}",
                           object_id.SerializeJson(), name.SerializeJson(),
                           chart_id.SerializeJson(), source_id.SerializeJson(),
                           line_style.SerializeJson(), line_color.SerializeJson(),
                           line_opacity.SerializeJson(), fill_color.SerializeJson(),
                           fill_opacity.SerializeJson(), next_id.SerializeJson());
    }
};

// This is type 0x4D data
struct PanelElementData : DudeObj {
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

    std::string SerializeJson(bool has_credentials) const override {
        return fmt::format(
            "\"objectId\":{}, \"name\":{}, \"split\":{}, \"panelId\":{}, \"splitType\":{}, "
            "\"splitShare\":{}, \"firstId\":{}, \"secondId\":{}, \"objId\":{}, \"objMeta\":{}",
            object_id.SerializeJson(), name.SerializeJson(), split.SerializeJson(),
            panel_id.SerializeJson(), split_type.SerializeJson(), split_share.SerializeJson(),
            first_id.SerializeJson(), second_id.SerializeJson(), obj_id.SerializeJson(),
            obj_meta.SerializeJson());
    }
};
} // namespace Database
