// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <span>
#include <string>
#include <vector>

#include "common/bit_field.h"
#include "common/common_types.h"

namespace Gzip {

// Compress/Decompress gzip files
class Gzip {
public:
    Gzip(const std::string& file);

    bool IsGzipFile();

    bool Decompress(const std::string& out_file);
    bool Compress(const std::string& out_file);

private:
    std::string filename{};
};
} // namespace Gzip
