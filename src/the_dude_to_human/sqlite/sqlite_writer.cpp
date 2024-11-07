// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include "the_dude_to_human/sqlite/sqlite_writer.h"

namespace Sqlite {
SqliteWriter::SqliteWriter(const std::string& db_file) {
    is_open = false;
    db_filename = db_file;
}

int SqliteWriter::OpenDatabase() {
    if (is_open) {
        return SQLITE_OK;
    }

    const int rc = sqlite3_open(db_filename.c_str(), &db);

    if (rc != SQLITE_OK) {
        return rc;
    }

    is_open = true;
    return SQLITE_OK;
}

void SqliteWriter::CloseDatabase() {
    if (!is_open) {
        return;
    }

    is_open = false;
    sqlite3_close(db);
}

int SqliteWriter::CreateTable() {
    return 0;
}

int SqliteWriter::ExecStatement(SqlData& data, const std::string& sql) const {
    sqlite3_stmt* statement{nullptr};

    int rc = sqlite3_prepare(db, sql.c_str(), -1, &statement, 0);

    if (rc != SQLITE_OK) {
        sqlite3_finalize(statement);
        printf("Can't create query \"%s\": %s\n", sql.c_str(), sqlite3_errmsg(db));
        return rc;
    }

    bool done = false;

    while (!done) {
        rc = sqlite3_step(statement);
        switch (rc) {
        case SQLITE_DONE:
            done = true;
            break;
        case SQLITE_BUSY:
            break;
        default:
            sqlite3_finalize(statement);
            printf("Can't execute query: %s\n%s\n", sql.c_str(), sqlite3_errmsg(db));
            return rc;
        }
    }

    sqlite3_finalize(statement);
    return SQLITE_OK;
}

const char* SqliteWriter::GetError() const {
    return sqlite3_errmsg(db);
}

} // namespace Sqlite
