// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <string>

#include "SqliteReader.h"

int main() {
	std::string db_filename = "../dude.db";
	Database::SqliteReader db{ db_filename };

	int rc = db.OpenDatabase();
	if (rc != 0) {
		printf("Can't open database: %s\n", db_filename);
		return 0;
	}
	printf("Opened database successfully\n");

	Database::SqlData data{};
	db.GetTableData(data, "objs");

	db.CloseDatabase();
}
