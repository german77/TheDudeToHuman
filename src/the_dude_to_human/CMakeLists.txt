# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(the_dude_to_human
    database/dude_database.cpp
    database/dude_database.h
    database/dude_field_id.h
    database/dude_field_parser.cpp
    database/dude_field_parser.h
    database/dude_json.cpp
    database/dude_json.h
    database/dude_types.h
    database/dude_validator.cpp
    database/dude_validator.h
    gzip/gzip.cpp
    gzip/gzip.h
    mikrotik/mikrotik_device.cpp
    mikrotik/mikrotik_device.h
    sqlite/sqlite_reader.cpp
    sqlite/sqlite_reader.h
    sqlite/sqlite_types.h
    sqlite/sqlite_writer.cpp
    sqlite/sqlite_writer.h
    TheDudeToHuman.cpp
)

target_link_libraries(the_dude_to_human PRIVATE common sqlite libssh2 zlib fmt::fmt)
if (MSVC)
    target_link_libraries(the_dude_to_human PRIVATE getopt wsock32 ws2_32)
endif()

create_target_directory_groups(the_dude_to_human)
