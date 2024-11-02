// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <string>

#include "dude_database.h"

int main() {
	std::string db_filename = "../dude.db";
	Database::DudeDatabase db{ db_filename };

	auto data1 = db.GetDeviceTypeData();
	auto data2 = db.GetDeviceData();
	auto data3 = db.GetSnmpProfileData();
	auto data4 = db.GetUnknown4aData();
}
