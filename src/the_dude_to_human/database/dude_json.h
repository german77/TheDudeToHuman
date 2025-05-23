// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Database {
    class DudeDatabase;
    std::string serializeDatabaseJson(DudeDatabase* db);
} // namespace Database
