// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <span>

#include "common/common_types.h"
#include "the_dude_to_human/database/dude_types.h"

namespace Database {
enum class ParserResult : u32 {
    Success,
    Corrupted,
    FieldTypeMismatch,
    FieldIdMismatch,
    InvalidFieldType,
    InvalidFieldArguments,
    InvalidMagic,
    InvalidHeader,
    EndOfFile,
};

class DudeFieldParser {
public:
    DudeFieldParser(std::span<const u8> raw_data);

    bool IsDataValid() const;
    ParserResult GetStatus() const;
    std::string GetErrorMessage() const;
    static std::string GetErrorMessage(ParserResult result);

    u16 GetMagic() const;
    IntArrayField GetFormat() const;
    DataFormat GetMainFormat() const;

    // Resets offset to the first byte
    void Reset();

    // Advances offset to next field location
    ParserResult SkipField();

    // Returns field info of the current location
    ParserResult GetFieldInfo(FieldInfo& info);

    // Read field and advances to next field location. If field type or id is mismatched stays in
    // the same location.
    ParserResult ReadField(BoolField& field, FieldId id);
    ParserResult ReadField(ByteField& field, FieldId id);
    ParserResult ReadField(IntField& field, FieldId id);
    ParserResult ReadField(TimeField& field, FieldId id);
    ParserResult ReadField(LongField& field, FieldId id);
    ParserResult ReadField(LongLongField& field, FieldId id);
    ParserResult ReadField(TextField& field, FieldId id);
    ParserResult ReadField(IntArrayField& field, FieldId id);
    ParserResult ReadField(LongArrayField& field, FieldId id);
    ParserResult ReadField(MacAddressField& field, FieldId id);
    ParserResult ReadField(StringArrayField& field, FieldId id);

private:
    void SaveOffset();
    void RestoreOffset();

    ParserResult ReadData(void* data, std::size_t size);

    ParserResult ReadFieldInfo(FieldInfo& field_info, FieldId id = FieldId::None);
    ParserResult ValidataFieldInfo(const FieldInfo& field_info, FieldId id = FieldId::None);

    ParserResult ReturnWithError(ParserResult result);

    u16 magic{};
    u32 error_count{};
    ParserResult status{ParserResult::Success};
    IntArrayField data_format{};

    std::size_t offset;
    std::size_t previous_offset;
    std::span<const u8> raw_data;
};
} // namespace Database
