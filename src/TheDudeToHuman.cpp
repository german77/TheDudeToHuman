// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <string>

#include "DudeDatabase.h"

int main() {
	std::string db_filename = "../dude.db";
	Database::DudeDatabase db{ db_filename };

	auto data = db.GetDeviceData();
}
