// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <fstream>

#include "the_dude_to_human/gzip/gzip.h"
#include "zlib.h"

#define BUFLEN 16384

static bool DecompressFiles(gzFile in, FILE* out) {
    char buf[BUFLEN];
    int len;
    bool is_first_batch = true;

    for (;;) {
        len = gzread(in, buf, sizeof(buf));
        if (len < 0)
            return false;
        if (len == 0)
            break;
        if (is_first_batch) {
            // Dude db files have a header. Skip it
            if ((int)fwrite(buf + 0x200, 1, (unsigned)len - 0x200, out) != len - 0x200) {
                return false;
            }
            is_first_batch = false;
            continue;
        }
        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
            return false;
        }
    }
    if (fclose(out))
        return false;

    return gzclose(in) == Z_OK;
}

namespace Gzip {
Gzip::Gzip(const std::string& file) : filename{file} {}

bool Gzip::IsGzipFile() {
    constexpr std::array<u8, 2> gzip_signature{0x1F, 0x8B};

    std::fstream file_data(filename);

    if (!file_data.is_open()) {
        return false;
    }

    std::array<char, 2> signature{};
    file_data.read(signature.data(), signature.size());
    file_data.close();

    return static_cast<u8>(signature[0]) == gzip_signature[0] && static_cast<u8>(signature[1]) == gzip_signature[1];
}

bool Gzip::Decompress(const std::string& out_file) {
    gzFile in = gzopen(filename.c_str(), "rb");
    FILE* out = nullptr;

#ifdef _WIN32
    if (fopen_s(&out, out_file.c_str(), "wb") != 0) {
        return false;
    }
#else
    out = fopen(out_file.c_str(),"wb");
    if(!out){
        return false;
    }
#endif

    return DecompressFiles(in, out);
}

bool Gzip::Compress(const std::string& out_file) {
    return true;
}

} // namespace Gzip
