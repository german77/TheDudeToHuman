// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "common/common_types.h"

namespace Sqlite {

using SqlRow = std::pair<u32, std::vector<u8>>;
using SqlData = std::vector<SqlRow>;

} // namespace Sqlite
