# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

include("${CMAKE_CURRENT_SOURCE_DIR}/../app/version.cmake")

# searching for Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if("${CXXDBG_USE_QT6}")
    set(qt_name "Qt6")
else()
    set(qt_name "Qt5")
endif()

find_package(${qt_name} COMPONENTS "Widgets" "DBus" "Svg")

# Detecting if system Qt is used
set(CXXDBG_QT_IS_SYSTEM FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    get_target_property(_qmake_executable ${qt_name}::qmake IMPORTED_LOCATION)
    if(NOT _qmake_executable)
        message(FATAL_ERROR "Can't detect path to qmake executable")
    endif()

    execute_process(COMMAND "${_qmake_executable}" -query QT_INSTALL_PREFIX
                     OUTPUT_VARIABLE _qt_install_prefix
                     OUTPUT_STRIP_TRAILING_WHITESPACE)
    if("${_qt_install_prefix}" STREQUAL "/usr")
        set(CXXDBG_QT_IS_SYSTEM TRUE)
    endif()
    message(STATUS "Detected ${qt_name} install prefix: ${_qt_install_prefix} (system Qt: ${CXXDBG_QT_IS_SYSTEM})")
endif()

# Detecting if static or shared Qt is used
get_target_property(_qt_core_type ${qt_name}::Core TYPE)
if("${_qt_core_type}" STREQUAL "STATIC_LIBRARY")
    set(CXXDBG_QT_IS_STATIC TRUE)
else()
    set(CXXDBG_QT_IS_STATIC FALSE)
endif()
message(STATUS "Detected ${qt_name} library type: ${_qt_core_type} (static Qt: ${CXXDBG_QT_IS_STATIC})")

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_definitions(-fPIC)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})


# Common GUI code

set(cxxdbg_gui_sources
            pch_gui.hpp
            cxxdbg_connect.cpp
            cxxdbg_connect.hpp
            AboutDialog.cpp
            AboutDialog.h
            AddWatchDialog.cpp
            AddWatchDialog.h
            AddWatchpointDialog.cpp
            AddWatchpointDialog.hpp
            AddWatchpointDialog.ui
            AppItemModel.cpp
            AppItemModel.h
            AutoCompletionWidget.hpp
            AutoCompletionWidget.cpp
            BreakpointConditionDialog.cpp
            BreakpointConditionDialog.h
            BreakpointHitCountDialog.cpp
            BreakpointHitCountDialog.h
            BreakpointsDockWidget.cpp
            BreakpointsDockWidget.h
            CallStackDockWidget.cpp
            CallStackDockWidget.h
            CxxSyntaxHighlighter.cpp
            CxxSyntaxHighlighter.h
            DbgTerminalWidget.cpp
            DbgTerminalWidget.h
            debug_document_widget_factory.cpp
            debug_document_widget_factory.hpp
            debug_source_code_widget_delegate.cpp
            debug_source_code_widget_delegate.hpp
            DebugUI.cpp
            DebugUI.h
            DocumentListUi.cpp
            DocumentListUi.h
            document_list_widget.cpp
            document_list_widget.hpp
            document_widget.hpp
            EditAddressBreakpointDialog.cpp
            EditAddressBreakpointDialog.h
            EditFunctionBreakpointDialog.cpp
            EditFunctionBreakpointDialog.h
            EditSourcePosBreakpointDialog.cpp
            EditSourcePosBreakpointDialog.h
            EventQueueApplication.cpp
            EventQueueApplication.h
            FileSelectWidget.cpp
            FileSelectWidget.h
            FileSystemDockWidget.cpp
            FileSystemDockWidget.h
            FontSelectWidget.cpp
            FontSelectWidget.h
            FormLayout.cpp
            FormLayout.h
            FunctionNameDialog.cpp
            FunctionNameDialog.h
            FunctionNameDialog.ui
            HexValidator.cpp
            HexValidator.h
            LineNumberDialog.h
            LineNumberDialog.cpp
            LineNumbersWidget.cpp
            LineNumbersWidget.h
            LocalsDockWidget.cpp
            LocalsDockWidget.h
            LogDockWidget.cpp
            LogDockWidget.h
            OutputTextWidget.cpp
            OutputTextWidget.h
            PlatformWidget.cpp
            PlatformWidget.h
            QtSettingsStorage.cpp
            QtSettingsStorage.h
            RecentFilesMenu.cpp
            RecentFilesMenu.h
            RegistersDockWidget.cpp
            RegistersDockWidget.h
            SaveChangesDialog.cpp
            SaveChangesDialog.h
            SearchDialog.cpp
            SearchDialog.h
            SettingsDialog.cpp
            SettingsDialog.h
            CxxdbgDockWidget.cpp
            CxxdbgDockWidget.h
            CxxdbgTreeView.cpp
            CxxdbgTreeView.h
            CxxdbgTreeWidget.cpp
            CxxdbgTreeWidget.h
            StepUntilDialog.cpp
            StepUntilDialog.h
            TerminalDockWidget.cpp
            TerminalDockWidget.h
            TerminalWidget.cpp
            TerminalWidget.h
            text_document_widget.ipp
            text_document_widget.hpp
            ThreadsDockWidget.cpp
            ThreadsDockWidget.h
            WatchListDockWidget.cpp
            WatchListDockWidget.h
            WatchListTreeView.cpp
            WatchListTreeView.h
            Utils.cpp
            Utils.h)


# Debugger

set(cxxdbg_additional_sources)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(cxxdbg-icon OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/images/icon.rc")
endif()

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"
   AND NOT "${CXXDBG_QT_IS_STATIC}"
   AND NOT "${CXXDBG_QT_IS_SYSTEM}")
    list(APPEND cxxdbg_additional_sources "qt_conf.qrc")
endif()

add_executable(cxxdbg MACOSX_BUNDLE
               pch_cxxdbg.hpp
               AttachDialog.cpp
               AttachDialog.h
               DbgSettingsDialog.cpp
               DbgSettingsDialog.h
               DebugSettingsWidget.cpp
               DebugSettingsWidget.h
               ExpressionsListDialog.cpp
               ExpressionsListDialog.h
               LaunchExecutableDialog.cpp
               LaunchExecutableDialog.h
               LaunchOptionsWidget.cpp
               LaunchOptionsWidget.h
               ListOfListsDialog.cpp
               ListOfListsDialog.h
               ListOfListsDialog.ui
               LoadExecutableDialog.cpp
               LoadExecutableDialog.h
               main.cpp
               MainWindow.cpp
               MainWindow.h
               PlatformConnectDialog.cpp
               PlatformConnectDialog.h
               RegularExpressionDialog.cpp
               RegularExpressionDialog.h
               RegularExpressionDialog.ui
               CxxdbgApplication.cpp
               CxxdbgApplication.h
               CxxdbgProgressDialog.cpp
               CxxdbgProgressDialog.h
               SourceTreeDockWidget.cpp
               SourceTreeDockWidget.h
               TerminalSettingsWidget.cpp
               TerminalSettingsWidget.h
               WatchListColorConverter.h
               ${cxxdbg_gui_sources}
               ${cxxdbg_additional_sources})

target_precompile_headers(cxxdbg PRIVATE pch_cxxdbg.hpp)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_sources(cxxdbg PRIVATE $<TARGET_OBJECTS:cxxdbg-icon>)
endif()

target_link_libraries(cxxdbg PRIVATE tmvc-qt cxxdbg-app cxxdbg-dbg-appcore)
target_link_libraries(cxxdbg PRIVATE ${qt_name}::Widgets)
target_link_libraries(cxxdbg PRIVATE ${qt_name}::Svg)

if(TARGET ${qt_name}::QXcbIntegrationPlugin)
    target_link_libraries(cxxdbg PRIVATE ${qt_name}::QXcbIntegrationPlugin)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND "${CXXDBG_DISABLE_WINDOWS_CONSOLE}")
    target_link_options(cxxdbg PRIVATE -Wl,-subsystem,windows)
endif()

set_property(TARGET cxxdbg PROPERTY MACOSX_BUNDLE_BUNDLE_NAME "CxxDbg")
set_property(TARGET cxxdbg PROPERTY MACOSX_BUNDLE_BUNDLE_VERSION "${CXXDBG_VERSION_NUMBER}")
set_property(TARGET cxxdbg PROPERTY MACOSX_BUNDLE_SHORT_VERSION_STRING "${CXXDBG_VERSION_NUMBER}")
set_property(TARGET cxxdbg PROPERTY MACOSX_BUNDLE_ICON_FILE "icon.icns")
set_property(TARGET cxxdbg PROPERTY MACOSX_BUNDLE_GUI_IDENTIFIER "org.cxxdbg.cxxdbg")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg PROPERTY OUTPUT_NAME "CxxDbg")
    set_property(TARGET cxxdbg PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../..")
endif()

get_target_property(_qmake_executable ${qt_name}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

set(sign_cmd)
if(NOT "${LLDB_CODESIGN_IDENTITY}" STREQUAL "")
    set(debugserver_entitlements "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/tools/debugserver/resources/debugserver-macosx-entitlements.plist")
    set(exec_stub_entitlements "${CMAKE_CURRENT_SOURCE_DIR}/../exec_stub/entitlements.plist")
    set(sign_cmd
        COMMAND "/usr/bin/codesign" "--options=runtime" "-s" "${LLDB_CODESIGN_IDENTITY}" "--deep" "--force" "$<TARGET_FILE_DIR:cxxdbg>/../../Contents/bin/cxxdbg-dap-server"
        COMMAND "/usr/bin/codesign" "--options=runtime" "-s" "${LLDB_CODESIGN_IDENTITY}" "--deep" "--force" "--entitlements=${exec_stub_entitlements}" "$<TARGET_FILE_DIR:cxxdbg>/../../Contents/bin/cxxdbg-exec-stub"
        COMMAND "/usr/bin/codesign" "--options=runtime" "-s" "${LLDB_CODESIGN_IDENTITY}" "--deep" "--force" "--entitlements=${debugserver_entitlements}" "$<TARGET_FILE_DIR:cxxdbg>/../../Contents/bin/debugserver"
        COMMAND "/usr/bin/codesign" "--options=runtime" "-s" "${LLDB_CODESIGN_IDENTITY}" "--deep" "--force" "$<TARGET_FILE_DIR:cxxdbg>/../..")
else()
    # No codesign identity configured: remove the signature from all libraries
    # in the bundle instead.
    set(sign_cmd
        COMMAND "/usr/bin/codesign" "--force" "--deep" "-s" "-" "$<TARGET_FILE_DIR:cxxdbg>/../..")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    # Building all utilities before the cxxdbg GUI to make sure it's in the bundle dir
    # before signing the bundle
    add_dependencies(cxxdbg cxxdbg-dap-server cxxdbg-exec-stub lldb-server-stage)
    if("${CXXDBG_ENABLE_CLI}")
        add_dependencies(cxxdbg cxxdbg-cli)
    endif()

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set(no_strip_opt "-no-strip")
    else()
        set(no_strip_opt)
    endif()
    add_custom_command(TARGET cxxdbg POST_BUILD
                       COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE_DIR:cxxdbg>/../.."
                               "-always-overwrite"
                               ${no_strip_opt}
                       COMMAND "${CMAKE_COMMAND}" -E create_symlink
                               "../MacOS/CxxDbg" "$<TARGET_FILE_DIR:cxxdbg>/../../Contents/bin/cxxdbg"
                       ${sign_cmd}
                       COMMENT "Running macdeployqt and codesign...")
endif()

# Desktop file for linux
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/cxxdbg.desktop"
            DESTINATION "share/applications"
            COMPONENT cxxdbg)
endif()

# linking with static libgcc and static libstdc++
#if(NOT CXXDBG_ENABLE_SHARED AND NOT CXXDBG_LLVM_ENABLE_SHARED)
#    # on Windows, Qt build depends on shared mingw libraries
#    if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
#        if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
#            target_link_libraries(cxxdbg PRIVATE -static-libgcc)
#        endif()
#        target_link_libraries(cxxdbg PRIVATE -static-libstdc++)
#    endif()
#endif()


############################################################
# INSTAL

# install anything inside CxxDbg.app bundle on macos
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(inst_prefix "CxxDbg.app/Contents/")
else()
    set(inst_prefix)
endif()

set(lib_dir "lib/cxxdbg")

# set relative rpath for executables
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set_property(TARGET cxxdbg PROPERTY INSTALL_RPATH "@executable_path/../Frameworks")
else()
    set_property(TARGET cxxdbg PROPERTY INSTALL_RPATH "$ORIGIN/../${lib_dir}/lib")
endif()


# install executables
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(inst_args BUNDLE DESTINATION ".")
else()
    set(inst_args RUNTIME DESTINATION "bin")
endif()
install(TARGETS cxxdbg
        ${inst_args}
        COMPONENT cxxdbg)


# Installing Qt libraries
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"
   AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"
   AND NOT "${CXXDBG_QT_IS_STATIC}"
   AND NOT "${CXXDBG_QT_IS_SYSTEM}")
    set(qt_libs "Widgets" "Gui" "Core" "DBus" "Svg")
    set(qt_additional_libs)

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(lib_fname_prefix "")
    else()
        set(lib_fname_prefix "lib")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(qt_additional_libs "icui18n" "icuuc" "icudata" "${qt_name}XcbQpa")
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(main_qt_lib_dir "bin")
    else()
        set(main_qt_lib_dir "${lib_dir}/lib")
    endif()

    # detecting Qt libraries and installing them
    foreach(qt_lib ${qt_libs})
        # getting library location
        get_property(qt_lib_path TARGET "${qt_name}::${qt_lib}" PROPERTY IMPORTED_LOCATION_RELEASE)
        if("${qt_lib_path}" STREQUAL "")
            message(FATAL_ERROR "Can't detect path to '${qt_lib}' Qt library")
        endif()

        set(rename_flags)
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
            # getting library so name on Linux becuse it may have .version suffix
            get_property(qt_lib_name TARGET "${qt_name}::${qt_lib}" PROPERTY IMPORTED_SONAME_RELEASE)
            if("${qt_lib_name}" STREQUAL "")
                message(FATAL_ERROR "Cant' detect soname for '${qt_lib}' Qt library")
            endif()

            set(rename_flags RENAME "${qt_lib_name}")
        endif()
        
        # adding install target
        install(FILES "${qt_lib_path}" DESTINATION "${main_qt_lib_dir}" ${rename_flags} COMPONENT cxxdbg)
    endforeach()

    # installing additional Qt libraries
    get_property(qt_widgets_path TARGET "${qt_name}::Widgets" PROPERTY IMPORTED_LOCATION_RELEASE)
    if("${qt_widgets_path}" STREQUAL "")
        message(FATAL_ERROR "Can't detect path to '${qt_name}::Widgets' Qt library")
    endif()
    get_filename_component(qt_libs_dir "${qt_widgets_path}" PATH)

    foreach(lib ${qt_additional_libs})
        file(GLOB lib_files "${qt_libs_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}.?*")
        if("${lib_files}" STREQUAL "")
            message(FATAL_ERROR "Can't find additional Qt library '${lib}' in '${qt_libs_dir}'")
        endif()

        foreach(lib_file ${lib_files})
            install(FILES "${lib_file}" DESTINATION "${lib_dir}/lib" COMPONENT cxxdbg)
        endforeach()
    endforeach()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        # installing qxcb platform plugin
        install(FILES "${qt_libs_dir}/../plugins/platforms/${CMAKE_SHARED_LIBRARY_PREFIX}qxcb${CMAKE_SHARED_LIBRARY_SUFFIX}"
                DESTINATION "${lib_dir}/plugins/platforms"
                COMPONENT cxxdbg)
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        # installing windows platform plugin
        install(FILES "${qt_libs_dir}/../plugins/platforms/qwindows.dll"
                DESTINATION "${lib_dir}/plugins/platforms"
                COMPONENT cxxdbg)
    endif()

    # installing platform themes
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        set(theme_libname)
        if(EXISTS "${qt_libs_dir}/../plugins/platformthemes/libqgtk3.so")
            set(theme_libname "qgtk3")
        elseif(EXISTS "${qt_libs_dir}/../plugins/platformthemes/libqgtk2.so")
            set(theme_libname "qgtk2")
        endif()

        if(NOT "${theme_libname}" STREQUAL "")
            install(FILES "${qt_libs_dir}/../plugins/platformthemes/lib${theme_libname}.so"
                    DESTINATION "${lib_dir}/plugins/platformthemes"
                    COMPONENT cxxdbg)
        endif()
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        install(FILES "${qt_libs_dir}/../plugins/styles/qwindowsvistastyle.dll"
                DESTINATION "${lib_dir}/plugins/styles"
                COMPONENT cxxdbg)
    endif()

    # installing SVG image support
    install(FILES "${qt_libs_dir}/../plugins/imageformats/${lib_fname_prefix}qsvg${CMAKE_SHARED_LIBRARY_SUFFIX}"
            DESTINATION "${lib_dir}/plugins/imageformats"
            COMPONENT cxxdbg)

    # installing ico image support
    install(FILES "${qt_libs_dir}/../plugins/imageformats/${lib_fname_prefix}qico${CMAKE_SHARED_LIBRARY_SUFFIX}"
    DESTINATION "${lib_dir}/plugins/imageformats"
    COMPONENT cxxdbg)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    install(CODE "
        execute_process(COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                                 \"\${CMAKE_INSTALL_PREFIX}/bin/$<TARGET_FILE_NAME:cxxdbg>\"
                                 --libdir \"\${CMAKE_INSTALL_PREFIX}/bin\"
                                 --plugindir \"\${CMAKE_INSTALL_PREFIX}/${lib_dir}/plugins\"
                                 --no-translations)
    " COMPONENT cxxdbg)

    get_filename_component(mingw_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    install(FILES "${mingw_bin_dir}/libgcc_s_seh-1.dll"
                  "${mingw_bin_dir}/libstdc++-6.dll"
                  "${mingw_bin_dir}/libwinpthread-1.dll"
            DESTINATION "bin"
            COMPONENT cxxdbg)
endif()

add_dependencies(install-cxxdbg cxxdbg)
