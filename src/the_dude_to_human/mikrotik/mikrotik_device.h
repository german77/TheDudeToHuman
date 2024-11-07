// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "common/common_types.h"

namespace Mikrotik {

// Connects to a mikrotik device using ssh
class MikrotikDevice {
public:
    MikrotikDevice(std::string address_, u16 port_);

    bool Connect(std::string user, std::string password);
    bool Disconnect();

    void DownloadDatabase();
    void UploadDatabase();

private:
    bool is_connected{};

    std::string address{};
    u16 port{};
};

} // namespace Mikrotik
