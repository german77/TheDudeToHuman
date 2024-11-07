// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common/common_types.h"
#include "the_dude_to_human/database/dude_types.h"

namespace Database {
class DudeDatabase;

class DudeValidator {
public:
    DudeValidator();

private:
    DudeDatabase* db;
};
} // namespace Database
