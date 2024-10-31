// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "sqlite3.h"

namespace Database {

	class SqliteReader{
	public:
		SqliteReader(std::string db_file);

		int OpenDatabase();

		void CloseDatabase();

	private:
		bool is_open;
		std::string db_filename;
		sqlite3* db;
	};


}