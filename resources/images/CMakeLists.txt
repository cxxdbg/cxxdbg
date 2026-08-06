

set(images
    break.png
    edit.png
    delete.png
    disabled_break.png
    right_blue.png
    right_yellow.png
    thread.png
    logo-title.svg

    # standard icons replacement
    new.png
    open.png
    save.png
    save-as.png
    exit.png

    cancel.png
    back.png
    cut.png
    copy.png
    insert.png
    xdelete.png
    find.png

    icon.ico
   )

set(svg_images
    play
    resume
    pause
    stop

    step_into
    step_over
    step_out
    
    break_toggle
    break_disable
    
    break_add_source
    break_add_func
    break_add_addr
    
    break_enable_all
    break_disable_all
    break_delete_all
    
    goto
    select
    add
    remove
    edit
    
    goto_val
    goto_def
    goto_type
    
    open)

foreach(svg ${svg_images})
    list(APPEND images "${svg}_16.svg")
endforeach()

set(share_subdir "share/cxxdbg/images")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(dest_dir "${CMAKE_CURRENT_BINARY_DIR}/../../cxxdbg.app/Contents/${share_subdir}")
else()
    set(dest_dir "${CMAKE_CURRENT_BINARY_DIR}/../../${share_subdir}")
endif()

# copy image files to stage directory and add install rules
set(deps)
foreach(image ${images})
    set(input "${CMAKE_CURRENT_SOURCE_DIR}/${image}")
    set(output "${dest_dir}/${image}")

    add_custom_command(OUTPUT "${output}"
                       COMMAND "${CMAKE_COMMAND}" -E copy "${input}" "${output}"
                       DEPENDS "${input}")
    list(APPEND deps "${output}")

    # images will be installed together with cxxdbg bundle on Macos
    if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        install(FILES ${deps} DESTINATION "${share_subdir}" COMPONENT cxxdbg)
    endif()
endforeach()

# copy icon into Resources folder for Macos
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(input "${CMAKE_CURRENT_SOURCE_DIR}/icon.icns")
    set(output "${CMAKE_CURRENT_BINARY_DIR}/../../cxxdbg.app/Contents/Resources/icon.icns")

    add_custom_command(OUTPUT "${output}"
                       COMMAND "${CMAKE_COMMAND}" -E copy "${input}" "${output}"
                       DEPENDS "${input}")
    list(APPEND deps "${output}")
endif()

add_custom_target(images ALL DEPENDS ${deps})

