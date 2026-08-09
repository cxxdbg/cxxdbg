# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# CPack post-build script (see CPACK_POST_BUILD_SCRIPTS in cpack.cmake):
# signs the generated .dmg with CPACK_CXXDBG_CODESIGN_IDENTITY.

foreach(package_file IN LISTS CPACK_PACKAGE_FILES)
    if("${package_file}" MATCHES "\\.dmg$")
        message(STATUS "Signing ${package_file}")
        execute_process(
            COMMAND "/usr/bin/codesign" "--force" "-s" "${CPACK_CXXDBG_CODESIGN_IDENTITY}" "${package_file}"
            RESULT_VARIABLE codesign_result)
        if(NOT codesign_result EQUAL 0)
            message(FATAL_ERROR "Failed to sign ${package_file}")
        endif()
    endif()
endforeach()
