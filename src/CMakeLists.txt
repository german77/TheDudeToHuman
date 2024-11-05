# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: MIT

# Enable modules to include each

add_executable(the_dude_to_human
    dude_database.cpp
    dude_database.h
    dude_field_id.h
    dude_types.h
    sqlite_reader.cpp
    sqlite_reader.h
    TheDudeToHuman.cpp
    common/bit_field.h
    common/common_funcs.h
    common/common_types.h
    common/swap.h
    sqlite/sqlite3.c
    sqlite/sqlite3.h
)

#add_subdirectory(common)
#add_subdirectory(dude_to_human)
