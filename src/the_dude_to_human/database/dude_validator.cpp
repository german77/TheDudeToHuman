// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cstddef>
#include <cstdio>

#include "the_dude_to_human/database/dude_validator.h"

namespace Database {
DudeValidator::DudeValidator(DudeDatabase* database) : db{database} {}

} // namespace Database
