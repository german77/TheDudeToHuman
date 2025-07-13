// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>

#include "the_dude_to_human/database/dude_field_parser.h"

namespace Database {
DudeFieldParser::DudeFieldParser(std::span<const u8> data) : raw_data{data} {
    Reset();
}

void DudeFieldParser::Reset() {
    status = ParserResult::Success;
    error_count = 0;
    offset = 0;

    if (ReadData(&magic, sizeof(u16)) != ParserResult::Success) {
        status = ParserResult::InvalidMagic;
        return;
    }

    if (ReadField(data_format, FieldId::DataFormat) != ParserResult::Success) {
        status = ParserResult::InvalidHeader;
        return;
    }

    if (data_format.entries == 0) {
        status = ParserResult::InvalidHeader;
        return;
    }
}

u16 DudeFieldParser::GetMagic() const {
    if (!IsDataValid()) {
        return {};
    }

    return magic;
}

IntArrayField DudeFieldParser::GetFormat() const {
    if (!IsDataValid()) {
        return {};
    }

    return data_format;
}

DataFormat DudeFieldParser::GetMainFormat() const {
    if (!IsDataValid()) {
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
        return ReturnWithError(result);
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
        return ReturnWithError(ParserResult::InvalidFieldType);
    }
}

ParserResult DudeFieldParser::ReadData(void* data, std::size_t size) {
    if (size == 0) {
        return ParserResult::Success;
    }
    if (data == nullptr) {
        return ReturnWithError(ParserResult::InvalidFieldArguments);
    }
    if (!IsDataValid()) {
        return ParserResult::Corrupted;
    }
    if (raw_data.size() < size + offset) {
        return ReturnWithError(ParserResult::EndOfFile);
    }

    std::memcpy(data, raw_data.data() + offset, size);
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
        return ReturnWithError(ParserResult::InvalidFieldType);
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

    switch (field.info.type) {
    case FieldType::Byte:
        break;
    case FieldType::Int:
        printf("Warning attempted to read byte on int field: %s\n",
               field.info.SerializeJson().c_str());
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    case FieldType::Long:
        printf("Warning attempted to read byte on long field: %s\n",
               field.info.SerializeJson().c_str());
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    default:
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
    case FieldType::Long:
        printf("Warning attempted to read int on long field: %s\n",
               field.info.SerializeJson().c_str());
        return ParserResult::FieldTypeMismatch;
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
        result = ReadData(&field.text_size, sizeof(u8));
        break;
    case FieldType::LongString:
        result = ReadData(&field.text_size, sizeof(u16));
        break;
    default:
        RestoreOffset();
        return ParserResult::FieldTypeMismatch;
    }

    if (result != ParserResult::Success) {
        RestoreOffset();
        return result;
    }

    std::vector<char> raw_text(field.text_size);
    result = ReadData(raw_text.data(), field.text_size);

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
        result = ReadData(&field.entries[i].text_size, sizeof(StringArrayField::entry_count));
        if (result != ParserResult::Success) {
            RestoreOffset();
            return result;
        }

        std::vector<char> raw_text(field.entries[i].text_size);
        result = ReadData(raw_text.data(), field.entries[i].text_size);
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

ParserResult DudeFieldParser::ReturnWithError(ParserResult result) {
    if (result == ParserResult::Success) {
        return ParserResult::Success;
    }

    error_count++;
    status = result;
    return result;
}

bool DudeFieldParser::IsDataValid() const {
    return status == ParserResult::Success;
}

ParserResult DudeFieldParser::GetStatus() const {
    return status;
}

std::string DudeFieldParser::GetErrorMessage() const {
    return GetErrorMessage(status);
}

std::string DudeFieldParser::GetErrorMessage(ParserResult result) {
    switch (result) {
    case ParserResult::Success:
        return "OK";
    case ParserResult::Corrupted:
        return "The field can't be fully parsed";
    case ParserResult::FieldTypeMismatch:
        return "Requested field type mismatch";
    case ParserResult::FieldIdMismatch:
        return "Requested field id mismatch";
    case ParserResult::InvalidFieldType:
        return "Unsupported field type";
    case ParserResult::InvalidFieldArguments:
        return "Arguments given are invalid";
    case ParserResult::InvalidMagic:
        return "Magic bytes can't be read";
    case ParserResult::InvalidHeader:
        return "Header is invalid";
    case ParserResult::EndOfFile:
        return "Reached end of file while parsing data";
    default:
        return "Unexpected error";
    }
}

} // namespace Database
