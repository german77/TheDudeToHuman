// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include "dude_field_parser.h"

namespace Database {
DudeFieldParser::DudeFieldParser(std::span<const u8> data) : raw_data{data} {
    Reset();
}

void DudeFieldParser::Reset() {
    is_data_valid = true;
    offset = 0;

    if (ReadData(&magic, sizeof(u16)) != ParserResult::Success) {
        is_data_valid = false;
        return;
    }

    if (ReadField(data_format, FieldId::DataFormat) != ParserResult::Success) {
        is_data_valid = false;
        return;
    }

    if (data_format.entries == 0) {
        is_data_valid = false;
        return;
    }
}

u16 DudeFieldParser::GetMagic() const {
    if (!is_data_valid) {
        return {};
    }

    return magic;
}
IntArrayField DudeFieldParser::GetFormat() const {
    if (!is_data_valid) {
        return {};
    }

    return data_format;
}

DataFormat DudeFieldParser::GetMainFormat() const {
    if (!is_data_valid) {
        return {};
    }

    if (data_format.entries == 0) {
        return {};
    }

    return static_cast<DataFormat>(data_format.data[0]);
}

ParserResult DudeFieldParser::GetFieldInfo(FieldInfo& info) {
    const auto result = ReadFieldInfo(info);
    RestoreOffset();
    return result;
}

ParserResult DudeFieldParser::SkipField() {
    FieldInfo info{};
    ParserResult result = GetFieldInfo(info);

    if (result != ParserResult::Success) {
        return result;
    }

    BoolField bool_field;
    IntField int_field;
    LongField long_field;
    LongLongField long_long_field;
    TextField text_field;
    LongArrayField long_array_field;
    IntArrayField int_array_field;
    StringArrayField string_array_field;

    RestoreOffset();
    switch (info.type) {
    case FieldType::BoolFalse:
    case FieldType::BoolTrue:
        return ReadField(bool_field, FieldId::None);
    case FieldType::Int:
    case FieldType::Byte:
        return ReadField(int_field, FieldId::None);
    case FieldType::Long:
        return ReadField(long_field, FieldId::None);
    case FieldType::LongLong:
        return ReadField(long_long_field, FieldId::None);
    case FieldType::LongString:
    case FieldType::ShortString:
        return ReadField(text_field, FieldId::None);
    case FieldType::LongArray:
        return ReadField(long_array_field, FieldId::None);
    case FieldType::IntArray:
        return ReadField(int_array_field, FieldId::None);
    case FieldType::StringArray:
        return ReadField(string_array_field, FieldId::None);
    default:
        return ParserResult::InvalidFieldType;
    }
}

ParserResult DudeFieldParser::ReadData(void* data, std::size_t size) {
    if (!is_data_valid) {
        return ParserResult::Corrupted;
    }

    if (raw_data.size() < size + offset) {
        return ParserResult::EndOfFile;
    }

    memcpy(data, raw_data.data() + offset, size);
    offset += size;

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ValidataFieldInfo(const FieldInfo& field_info, FieldId id) {
    switch (field_info.type) {
    case FieldType::BoolFalse:
    case FieldType::BoolTrue:
    case FieldType::Int:
    case FieldType::Byte:
    case FieldType::Long:
    case FieldType::LongLong:
    case FieldType::LongString:
    case FieldType::ShortString:
    case FieldType::LongArray:
    case FieldType::IntArray:
    case FieldType::StringArray:
        break;
    default:
        return ParserResult::InvalidFieldType;
    }

    // Allow all id if none is specified
    if (field_info.id.Value() == FieldId::None) {
        return ParserResult::Success;
    }

    if (field_info.id.Value() != id) {
        return ParserResult::FieldIdMismatch;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadFieldInfo(FieldInfo& field_info, FieldId id) {
    SaveOffset();
    auto result = ReadData(&field_info, sizeof(FieldInfo));
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    result = ValidataFieldInfo(field_info, id);
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(BoolField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    switch (field.info.type) {
    case FieldType::BoolFalse:
        field.value = false;
        break;
    case FieldType::BoolTrue:
        field.value = true;
        break;
    default:
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(ByteField& field, FieldId id) {
    const ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::Byte) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    return ReadData(&field.value, sizeof(ByteField::value));
}

ParserResult DudeFieldParser::ReadField(IntField& field, FieldId id) {
    const ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    switch (field.info.type) {
    case FieldType::Int:
    case FieldType::Byte:
        break;
    default:
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    if (field.info.type.Value() == FieldType::Byte) {
        return ReadData(&field.value, sizeof(ByteField::value));
    }

    return ReadData(&field.value, sizeof(IntField::value));
}

ParserResult DudeFieldParser::ReadField(TimeField& field, FieldId id) {
    const ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::Int) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    return ReadData(&field.date, sizeof(TimeField::date));
}

ParserResult DudeFieldParser::ReadField(LongField& field, FieldId id) {
    const ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::Long) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    return ReadData(&field.value, sizeof(LongField::value));
}

ParserResult DudeFieldParser::ReadField(LongLongField& field, FieldId id) {
    const ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::LongLong) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    return ReadData(&field.value, sizeof(LongLongField::value));
}

ParserResult DudeFieldParser::ReadField(TextField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    switch (field.info.type.Value()) {
    case FieldType::ShortString:
        result = ReadData(&field.data_size, sizeof(u8));
        break;
    case FieldType::LongString:
        result = ReadData(&field.data_size, sizeof(u16));
        break;
    default:
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    std::vector<char> raw_text(field.data_size);
    result = ReadData(raw_text.data(), field.data_size);

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    field.text = std::string(raw_text.begin(), raw_text.end());
    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(IntArrayField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::IntArray) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    result = ReadData(&field.entries, sizeof(u16));
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    field.data.resize(field.entries);
    result = ReadData(field.data.data(), field.entries * sizeof(u32));

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(LongArrayField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::LongArray) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    result = ReadData(&field.data_size, sizeof(LongArrayField::data_size));
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    field.data.resize(field.data_size);
    result = ReadData(field.data.data(), field.data_size);

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(MacAddressField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::LongArray) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    result = ReadData(&field.data_size, sizeof(MacAddressField::data_size));
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    field.mac_address.resize(field.data_size / sizeof(MacAddress));
    result = ReadData(field.mac_address.data(), field.data_size);

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    return ParserResult::Success;
}

ParserResult DudeFieldParser::ReadField(StringArrayField& field, FieldId id) {
    ParserResult result = ReadFieldInfo(field.info, id);
    if (result != ParserResult::Success) {
        return result;
    }

    if (field.info.type.Value() != FieldType::StringArray) {
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    result = ReadData(&field.entry_count, sizeof(StringArrayField::entry_count));
    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    field.entries.resize(field.entry_count);
    for (std::size_t i = 0; i < field.entry_count; ++i) {
        result = ReadData(&field.entries[i].data_size, sizeof(StringArrayField::entry_count));
        if (result != ParserResult::Success) {
            RestoreOffset();
            return result;
        }

        std::vector<char> raw_text(field.entries[i].data_size);
        result = ReadData(raw_text.data(), field.entries[i].data_size);
        if (result != ParserResult::Success) {
            RestoreOffset();
            return result;
        }

        field.entries[i].text = std::string(raw_text.begin(), raw_text.end());
    }

    return ParserResult::Success;
}

void DudeFieldParser::SaveOffset() {
    previous_offset = offset;
}

void DudeFieldParser::RestoreOffset() {
    offset = previous_offset;
}

} // namespace Database
