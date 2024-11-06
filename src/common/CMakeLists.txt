# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: MIT

add_library(common STATIC 
    bit_field.h
    common_funcs.h
    common_types.h
    string_util.cpp
    string_util.h
    swap.h
)

if (MSVC)
  target_compile_definitions(common PRIVATE
    # The standard library doesn't provide any replacement for codecvt yet
    # so we can disable this deprecation warning for the time being.
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
  )
  target_compile_options(common PRIVATE
    /we4242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
    /we4254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
    /we4800 # Implicit conversion from 'type' to bool. Possible information loss
  )
endif()

create_target_directory_groups(common)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(common PUBLIC the_dude_to_human)