// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: MIT

#include <cstdio>
#include <string>

#include "dude_database.h"

int main() {
	std::string db_filename = "../dude.db";
	Database::DudeDatabase db{ db_filename };

	//auto formats = db.ListUsedDataFormats();
	//auto data1 = db.GetDeviceTypeData();
	//auto data2 = db.GetDeviceData();
	//auto data3 = db.GetSnmpProfileData();
	//auto data4 = db.GetNetworkMapElementData();
	//auto data5 = db.GetNotesData();
	//auto data6 = db.GetDataSourceData();
	//auto data7 = db.GetMapData();
	//auto data8 = db.GetPanelElementData();
	//auto data9 = db.GetServiceData();
	auto data10 = db.GetPanelData();
	auto data11 = db.GetToolData();
}
