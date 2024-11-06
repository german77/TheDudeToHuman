# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(sqlite STATIC 
    sqlite3.c
    sqlite3.h
)

create_target_directory_groups(sqlite)

target_link_libraries(sqlite PUBLIC the_dude_to_human)