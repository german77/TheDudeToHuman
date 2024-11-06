# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: MIT

add_executable(the_dude_to_human
    database/dude_field_id.h
    database/dude_field_parser.cpp
    database/dude_field_parser.h
    database/dude_types.h
    dude_database.cpp
    dude_database.h
    sqlite_reader.cpp
    sqlite_reader.h
    TheDudeToHuman.cpp
)

create_target_directory_groups(the_dude_to_human)

target_link_libraries(the_dude_to_human PUBLIC sqlite)
