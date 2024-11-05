# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: MIT

add_executable(the_dude_to_human
    dude_database.cpp
    dude_database.h
    dude_field_id.h
    dude_types.h
    sqlite_reader.cpp
    sqlite_reader.h
    TheDudeToHuman.cpp
)

create_target_directory_groups(the_dude_to_human)

target_link_libraries(the_dude_to_human PUBLIC sqlite)
