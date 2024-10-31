// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <string>
#include "SqliteReader.h"

int main()
{
    std::string db_filename = "../dude.db";
    Database::SqliteReader db{ db_filename };

    db.OpenDatabase();
    db.CloseDatabase();
}