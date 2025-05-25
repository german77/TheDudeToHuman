// SPDX-FileCopyrightText: Copyright 2025 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Database {
    class DudeDatabase;
    int SerializeDatabaseJson(DudeDatabase* db, const std::string& db_file);
} // namespace Database
