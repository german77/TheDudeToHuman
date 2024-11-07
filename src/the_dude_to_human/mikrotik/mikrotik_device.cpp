// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mikrotik_device.h"

namespace Mikrotik {
MikrotikDevice::MikrotikDevice(std::string address_, u16 port_) : address{address_}, port{port_} {}

bool MikrotikDevice::Connect(std::string user, std::string password) {
    if (is_connected) {
        return true;
    }
    is_connected = true;
    return true;
}

bool MikrotikDevice::Disconnect() {
    if (!is_connected) {
        return true;
    }
    is_connected = false;
    return true;
}

void MikrotikDevice::DownloadDatabase() {
    if (!is_connected) {
        return;
    }
}

void MikrotikDevice::UploadDatabase() {
    if (!is_connected) {
        return;
    }
}

} // namespace Mikrotik
