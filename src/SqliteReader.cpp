// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include "SqliteReader.h"

namespace Database {

		SqliteReader::SqliteReader(std::string db_file) {
			is_open = false;
			db_filename = db_file;
			OpenDatabase();
		}

		int SqliteReader::OpenDatabase() {
			if (is_open) {
				return 0;
			}

			int rc = sqlite3_open_v2(db_filename.c_str(), &db, SQLITE_OPEN_READONLY, 0);

			if (rc) {
				printf("Can't open database: %s\n", sqlite3_errmsg(db));
				return rc;
			}

			printf("Opened database successfully\n");
			is_open = true;
			return 0;
		}

		void SqliteReader::CloseDatabase() {
			if (!is_open) {
				return;
			}

			is_open = false;
			sqlite3_close(db);
		}

}