# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: MIT

add_library(common STATIC 
    bit_field.h
    common_funcs.h
    common_types.h
    swap.h
)

create_target_directory_groups(common)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(common PUBLIC the_dude_to_human)