// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "common/common_types.h"
#include "sqlite3.h"
#include "the_dude_to_human/sqlite/sqlite_types.h"

namespace Sqlite {

class SqliteReader {
public:
    SqliteReader(const std::string& db_file);

    int OpenDatabase();
    void CloseDatabase();

    int GetTableData(SqlData& data, const std::string& table_name) const;

    const char* GetError() const;

private:
    int ExecStatement(SqlData& data, const std::string& sql) const;
    SqlRow ReadRow(sqlite3_stmt* statement) const;

    bool is_open{};
    std::string db_filename{};
    sqlite3* db{NULL};
};
} // namespace Sqlite
