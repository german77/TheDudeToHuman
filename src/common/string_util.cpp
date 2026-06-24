// SPDX-FileCopyrightText: 2013 Dolphin Emulator Project
// SPDX-FileCopyrightText: 2014 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

#include "common/string_util.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef ANDROID
#include <common/fs/fs_android.h>
#endif

namespace Common {

/// Make a string lowercase
std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return str;
}

/// Make a string uppercase
std::string ToUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return str;
}

std::string StringFromBuffer(std::span<const u8> data) {
    return std::string(data.begin(), std::find(data.begin(), data.end(), '\0'));
}

std::string StringFromBuffer(std::span<const char> data) {
    return std::string(data.begin(), std::find(data.begin(), data.end(), '\0'));
}

std::string HexStringFromBuffer(std::span<const u8> data) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;

    for (uint8_t byte : data) {
        ss << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

// Turns "  hej " into "hej". Also handles tabs.
std::string StripSpaces(const std::string& str) {
    const std::size_t s = str.find_first_not_of(" \t\r\n");

    if (str.npos != s)
        return str.substr(s, str.find_last_not_of(" \t\r\n") - s + 1);
    else
        return "";
}

// "\"hello\"" is turned to "hello"
// This one assumes that the string has already been space stripped in both
// ends, as done by StripSpaces above, for example.
std::string StripQuotes(const std::string& s) {
    if (s.size() && '\"' == s[0] && '\"' == *s.rbegin())
        return s.substr(1, s.size() - 2);
    else
        return s;
}

std::string StringFromBool(bool value) {
    return value ? "True" : "False";
}

bool SplitPath(const std::string& full_path, std::string* _pPath, std::string* _pFilename,
               std::string* _pExtension) {
    if (full_path.empty())
        return false;

#ifdef ANDROID
    if (full_path[0] != '/') {
        *_pPath = Common::FS::Android::GetParentDirectory(full_path);
        *_pFilename = Common::FS::Android::GetFilename(full_path);
        return true;
    }
#endif

    std::size_t dir_end = full_path.find_last_of("/"
// windows needs the : included for something like just "C:" to be considered a directory
#ifdef _WIN32
                                                 "\\:"
#endif
    );
    if (std::string::npos == dir_end)
        dir_end = 0;
    else
        dir_end += 1;

    std::size_t fname_end = full_path.rfind('.');
    if (fname_end < dir_end || std::string::npos == fname_end)
        fname_end = full_path.size();

    if (_pPath)
        *_pPath = full_path.substr(0, dir_end);

    if (_pFilename)
        *_pFilename = full_path.substr(dir_end, fname_end - dir_end);

    if (_pExtension)
        *_pExtension = full_path.substr(fname_end);

    return true;
}

void SplitString(const std::string& str, const char delim, std::vector<std::string>& output) {
    std::istringstream iss(str);
    output.resize(1);

    while (std::getline(iss, *output.rbegin(), delim)) {
        output.emplace_back();
    }

    output.pop_back();
}

std::string TabsToSpaces(int tab_size, std::string in) {
    std::size_t i = 0;

    while ((i = in.find('\t')) != std::string::npos) {
        in.replace(i, 1, tab_size, ' ');
    }

    return in;
}

std::string ReplaceAll(std::string result, const std::string& src, const std::string& dest) {
    std::size_t pos = 0;

    if (src == dest)
        return result;

    while ((pos = result.find(src, pos)) != std::string::npos) {
        result.replace(pos, src.size(), dest);
        pos += dest.length();
    }

    return result;
}

std::string ReplaceChar(std::string src, char c, const std::string& dest, std::size_t length) {
    std::size_t instances = 0;
    for (std::size_t i = 0; i < length; i++) {
        if (src.at(i) == c)
            instances++;
    }

    if (instances == 0) {
        return src;
    }

    std::vector<char> text;
    text.reserve(length + (instances * dest.size()));
    for (std::size_t i = 0; i < length; i++) {
        if (src.at(i) == c) {
            for (std::size_t e = 0; e < dest.size(); e++) {
                text.push_back(dest.at(e));
            }
            continue;
        }
        text.push_back(src.at(i));
    }
    return std::string(text.begin(), text.end());
}

std::string Sanitize(std::string str, std::size_t length) {
    if (str.size() != length) {
        return "Invalid size";
    }

    str = ReplaceChar(str, '\\', "\\\\", length);
    str = ReplaceChar(str, '\0', "\\\\0", length);
    str = ReplaceAll(str, "\x1", "\\\\x01");
    str = ReplaceAll(str, "\x2", "\\\\x02");
    str = ReplaceAll(str, "\x3", "\\\\x03");
    str = ReplaceAll(str, "\x4", "\\\\x04");
    str = ReplaceAll(str, "\x5", "\\\\x05");
    str = ReplaceAll(str, "\x6", "\\\\x06");
    str = ReplaceAll(str, "\x7", "\\\\x07");
    str = ReplaceAll(str, "\b", "\\b");
    str = ReplaceAll(str, "\t", "\\t");
    str = ReplaceAll(str, "\n", "\\n");
    str = ReplaceAll(str, "\v", "\\\\v");
    str = ReplaceAll(str, "\f", "\\f");
    str = ReplaceAll(str, "\r", "\\r");
    str = ReplaceAll(str, "\xe", "\\\\x0e");
    str = ReplaceAll(str, "\xf", "\\\\x0f");
    str = ReplaceAll(str, "\x10", "\\\\x10");
    str = ReplaceAll(str, "\x11", "\\\\x11");
    str = ReplaceAll(str, "\x12", "\\\\x12");
    str = ReplaceAll(str, "\x13", "\\\\x13");
    str = ReplaceAll(str, "\x14", "\\\\x14");
    str = ReplaceAll(str, "\x15", "\\\\x15");
    str = ReplaceAll(str, "\x16", "\\\\x16");
    str = ReplaceAll(str, "\x17", "\\\\x17");
    str = ReplaceAll(str, "\x18", "\\\\x18");
    str = ReplaceAll(str, "\x19", "\\\\x19");
    str = ReplaceAll(str, "\x1a", "\\\\x1a");
    str = ReplaceAll(str, "\x1b", "\\\\x1b");
    str = ReplaceAll(str, "\x1c", "\\\\x1c");
    str = ReplaceAll(str, "\x1d", "\\\\x1d");
    str = ReplaceAll(str, "\x1e", "\\\\x1e");
    str = ReplaceAll(str, "\x1f", "\\\\x1f");
    str = ReplaceAll(str, "\xa8", "\\\\xa8");
    str = ReplaceAll(str, "\"", "\\\"");
    return str;
}

std::string UTF16ToUTF8(std::u16string_view input) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(input.data(), input.data() + input.size());
}

std::u16string UTF8ToUTF16(std::string_view input) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.from_bytes(input.data(), input.data() + input.size());
}

std::u32string UTF8ToUTF32(std::string_view input) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
    return convert.from_bytes(input.data(), input.data() + input.size());
}

#ifdef _WIN32
static std::wstring CPToUTF16(u32 code_page, std::string_view input) {
    const auto size =
        MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);

    if (size == 0) {
        return {};
    }

    std::wstring output(size, L'\0');

    if (size != MultiByteToWideChar(code_page, 0, input.data(), static_cast<int>(input.size()),
                                    &output[0], static_cast<int>(output.size()))) {
        output.clear();
    }

    return output;
}

std::string UTF16ToUTF8(std::wstring_view input) {
    const auto size = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                                          nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        return {};
    }

    std::string output(size, '\0');

    if (size != WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                                    &output[0], static_cast<int>(output.size()), nullptr,
                                    nullptr)) {
        output.clear();
    }

    return output;
}

std::wstring UTF8ToUTF16W(std::string_view input) {
    return CPToUTF16(CP_UTF8, input);
}

#endif

std::u16string U16StringFromBuffer(const u16* input, std::size_t length) {
    return std::u16string(reinterpret_cast<const char16_t*>(input), length);
}

std::string StringFromFixedZeroTerminatedBuffer(std::string_view buffer, std::size_t max_len) {
    std::size_t len = 0;
    while (len < buffer.length() && len < max_len && buffer[len] != '\0') {
        ++len;
    }
    return std::string(buffer.begin(), buffer.begin() + len);
}

std::u16string UTF16StringFromFixedZeroTerminatedBuffer(std::u16string_view buffer,
                                                        std::size_t max_len) {
    std::size_t len = 0;
    while (len < buffer.length() && len < max_len && buffer[len] != '\0') {
        ++len;
    }
    return std::u16string(buffer.begin(), buffer.begin() + len);
}

} // namespace Common
