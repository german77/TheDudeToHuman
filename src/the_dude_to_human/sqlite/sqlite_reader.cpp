// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstring>

#include "the_dude_to_human/gzip/gzip.h"
#include "the_dude_to_human/sqlite/sqlite_reader.h"

namespace Sqlite {
SqliteReader::SqliteReader(const std::string& db_file) {
    is_open = false;
    db_filename = db_file;
}

int SqliteReader::OpenDatabase() {
    if (is_open) {
        return SQLITE_OK;
    }

    Gzip::Gzip gzip{db_filename};

    // Sqlite can't read compressed databases
    int result = SQLITE_OK;
    if (gzip.IsGzipFile()) {
        std::string tmp_db_file = db_filename + ".tmp";
        if (!gzip.Decompress(tmp_db_file)) {
            return SQLITE_CANTOPEN;
        }

        result = sqlite3_open_v2(tmp_db_file.c_str(), &db, SQLITE_OPEN_READONLY, 0);
    } else {
        result = sqlite3_open_v2(db_filename.c_str(), &db, SQLITE_OPEN_READONLY, 0);
    }

    if (result != SQLITE_OK) {
        return result;
    }

    is_open = true;
    return SQLITE_OK;
}

void SqliteReader::CloseDatabase() {
    if (!is_open) {
        return;
    }

    is_open = false;
    sqlite3_close(db);
}

int SqliteReader::GetTableData(SqlData& data, const std::string& table_name) const {
    if (!is_open) {
        return SQLITE_CANTOPEN;
    }

    SqlData row_count{};
    const int rc = ExecStatement(row_count, "SELECT COUNT(*) FROM '" + table_name + "'");

    if (rc != SQLITE_OK) {
        return rc;
    }

    data.reserve(row_count[0].first);
    return ExecStatement(data, "SELECT * FROM '" + table_name + "'");
}

int SqliteReader::ExecStatement(SqlData& data, const std::string& sql) const {
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
        case SQLITE_ROW:
            data.push_back(ReadRow(statement));
            break;
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

SqlRow SqliteReader::ReadRow(sqlite3_stmt* statement) const {
    if (statement == nullptr) {
        return {};
    }

    const int id = sqlite3_column_int(statement, 0);
    const int blob_size = sqlite3_column_bytes(statement, 1);
    const void* blob_data_pointer = sqlite3_column_blob(statement, 1);

    std::vector<u8> blob_data(blob_size);
    std::memcpy(blob_data.data(), blob_data_pointer, blob_size);
    return {id, blob_data};
}

const char* SqliteReader::GetError() const {
    return sqlite3_errmsg(db);
}

} // namespace Sqlite
