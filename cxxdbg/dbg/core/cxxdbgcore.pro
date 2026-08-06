#-------------------------------------------------
#
# Project created by QtCreator 2014-04-22T23:44:14
#
#-------------------------------------------------

QT       -= core gui

TARGET = cxxdbgcore
TEMPLATE = lib
CONFIG += staticlib

DEFINES += CXXDBGCORE_LIBRARY

SOURCES += \
    debugger.cpp \
    async_debugger.cpp \
    event.cpp \
    target.cpp \
    application.cpp \
    source_tree.cpp \
    command_line.cpp \
    thread.cpp \
    source_position.cpp \
    source_file.cpp \
    stack_frame.cpp \
    source_position_info.cpp \
    stack_frame_info.cpp \
    thread_info.cpp \
    change_state_event.cpp \
    source_model.cpp \
    thread_list_info.cpp \
    thread_list.cpp \
    async_execution_queue.cpp \
    quit_event.cpp \
    async_command_event.cpp \
    breakpoint.cpp \
    source_position_breakpoint.cpp \
    breakpoint_location.cpp \
    breakpoint_info.cpp \
    source_position_breakpoint_info.cpp \
    breakpoint_location_info.cpp \
    breakpoint_event.cpp \
    function_breakpoint_info.cpp \
    function_breakpoint.cpp \
    breakpoint_location_event.cpp

HEADERS += \
    command_interpreter.hpp \
    debugger.hpp \
    async_debugger.hpp \
    async_event_handler.hpp \
    event.hpp \
    version.hpp \
    target_base.hpp \
    application.hpp \
    source_tree.hpp \
    command_line.hpp \
    thread.hpp \
    source_position.hpp \
    source_file.hpp \
    stack_frame.hpp \
    source_position_info.hpp \
    stack_frame_info.hpp \
    thread_info.hpp \
    change_state_event.hpp \
    source_model.hpp \
    thread_list_info.hpp \
    thread_list.hpp \
    async_execution_queue.hpp \
    quit_event.hpp \
    async_command_event.hpp \
    breakpoint.hpp \
    source_position_breakpoint.hpp \
    breakpoint_location.hpp \
    breakpoint_info.hpp \
    source_position_breakpoint_info.hpp \
    breakpoint_location_info.hpp \
    breakpoint_event.hpp \
    function_breakpoint_info.hpp \
    function_breakpoint.hpp \
    breakpoint_location_event.hpp

QMAKE_CXXFLAGS += -std=c++0x -Wno-unused-parameter

unix {
    target.path = /usr/lib
    INSTALLS += target
}

