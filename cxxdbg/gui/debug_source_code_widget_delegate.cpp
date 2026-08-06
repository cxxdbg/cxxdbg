// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file debug_source_code_widget_delegate.cpp
/// Contains implementation of the debug_source_code_widget_delegate class.

#include "debug_source_code_widget_delegate.hpp"
#include "DebugUI.h"
#include "CxxdbgApplication.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/dbg/breakpoint_utils.hpp"
#include "cxxdbg/dbg/custom_watch_list.hpp"
#include "cxxdbg/dbg/source_position_info.hpp"
#include "cxxdbg/app/text_file_document.hpp"
#include "cxxdbg/dbg/thread_utils.hpp"


namespace cxxdbg::gui {


debug_source_code_widget_delegate::debug_source_code_widget_delegate(cxxdbg::dbg::debugger & dbg,
                                                                     DebugUI & dbgUi):
dbg_{dbg},
dbgUi_{dbgUi} {
    // update mark area when application state changed (thread markers may change)
    state_changed_con_ = dbg_.state_changed().connect([this] {
        mark_area_should_be_updated();
    });

    // update widget when current frame changed (we need redraw position marker)
    current_frame_changed_con_ = dbg_.current_frame_changed().connect([this] {
        mark_area_should_be_updated();

        // notifying clients that list of highlighted lines are changed
        highlighted_lines_changed();
    });

    // update widget when breakpoints changed
    breakpoints_changed_con_ = dbg_.breakpoints().list_changed().connect([this]() {
        mark_area_should_be_updated();
    });

    // update widget when show threads action checked/unchecked
    cxxdbg_connect(dbgUi_.showThreadsInSourceAction(), &QAction::triggered, [this]() {
        mark_area_should_be_updated();
    });
}

std::list<QIcon>
debug_source_code_widget_delegate::calculate_line_icons(const text_document * doc, size_t line) {
    std::list<QIcon> icons;

    auto fdoc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    if (!fdoc) {
        return {};
    }

    auto src = dbg_.src_mdl().source(fdoc->path());

    if (dbgUi_.showThreadsInSourceAction()->isChecked() && isThreadAtLine(src, line)) {
        icons.push_back(dbgUi_.threadIcon());
    }

    auto bps = cxxdbg::dbg::breakpoints_at_line(dbg_.breakpoints(), {src, static_cast<unsigned>(line + 1)});
    if (!bps.empty()) {
        if (std::ranges::distance(bps) == 1) {
            auto bp = bps.front();
            if (bp->enabled()) {
                icons.push_back(dbgUi_.breakpointIcon());
            } else {
                icons.push_back(dbgUi_.disabledBreakpointIcon());
            }
        } else {
            icons.push_back(dbgUi_.breakpointIcon());
        }
    }

    if (isCurrentThreadAtLine(src, line)) {
        icons.push_back(dbgUi_.rightYellowIcon());
    } else {
        if (isCurrentFrameAtLine(src, line)) {
            icons.push_back(dbgUi_.rightBlueIcon());
        }
    }

    return icons;
}


std::list<std::pair<size_t, QColor>>
debug_source_code_widget_delegate::calculate_highlighted_lines(const text_document * doc) {

    // no highlighted lines for non-file documents
    auto fdoc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    if (!fdoc) {
        return {};
    }

    auto src = dbg_.src_mdl().source(fdoc->path());

    // checking if current frame exists
    if (dbg_.current_frame() == nullptr) {
        return {};
    }

    std::list<std::pair<size_t, QColor>> res;

    // checking if current position of current thread is in the document
    if (dbg_.curr_thread_pos().is_valid() && dbg_.curr_thread_pos().file() == src) {
        res.push_back(std::pair{dbg_.curr_thread_pos().line() - 1, QColor{255, 255, 186}});
    }

    // checking if current frame is in the document and is not equal to current position of current thread
    if (dbg_.curr_thread_pos().is_valid() &&
        dbg_.curr_frame_pos().file() == src &&
        dbg_.curr_frame_pos() != dbg_.curr_thread_pos()) {

        res.push_back(std::pair{dbg_.curr_frame_pos().line() - 1, QColor{233, 244, 233}});
    }

    return res;
}


std::string debug_source_code_widget_delegate::get_line_description(const text_document * doc, size_t line) {
    auto * fdoc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    if (!fdoc) {
        return {};
    }

    auto src = dbg_.src_mdl().source(fdoc->path());
    return dbg_.line_description({src, static_cast<unsigned>(line) + 1},
                                 dbgUi_.showThreadsInSourceAction()->isChecked());
}


void debug_source_code_widget_delegate::handle_line_click(const text_document * doc, size_t line) {
    // source code widgets sets current position at clicked line before
    // invoking delegate, so just toggle breakpoint at current line
    auto text_doc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    auto d = dbg_.src_mdl().source(text_doc->path());
    auto l = static_cast<unsigned int>(line) + 1;
    dbg_.breakpoints().toggle_breakpoint(d, l);
}


std::unique_ptr<QMenu>
debug_source_code_widget_delegate::create_marks_area_context_menu(const text_document * doc, size_t line) {
    assert(doc && "document should not be null here");

    auto fdoc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    if (!fdoc) {
        return {};
    }

    auto src = dbg_.src_mdl().source(fdoc->path());

    // showing breakpoint context menu if there is a breakpoint at line
    auto bps = cxxdbg::dbg::breakpoints_at_line(dbg_.breakpoints(),  {src, static_cast<unsigned>(line + 1)});
    if (!bps.empty()) {
        return createMarksAreaBreakpointMenu(*std::begin(bps));
    }

    // showing threads context menu if there is thread at line
    if (dbgUi_.showThreadsInSourceAction()->isChecked()) {
        auto threads = dbg_.line_threads(src, line + 1);
        if (!threads.empty()) {
            return createMarksAreaThreadMenu(*std::begin(threads));
        }
    }

    // displaying common context menu for marks panel

    auto menu = std::make_unique<QMenu>();

    QAction * insertBreakpointAction = menu->addAction(tr("Insert breakpoint"));
    bool enable_bp = dbg_.state() == cxxdbg::dbg::debugger::state_t::initial ||
                     dbg_.state() == cxxdbg::dbg::debugger::state_t::loaded ||
                     dbg_.state() == cxxdbg::dbg::debugger::state_t::stopped;
    insertBreakpointAction->setEnabled(enable_bp);
    cxxdbg_connect(insertBreakpointAction, &QAction::triggered, [this, fdoc, line] {
        dbg_.breakpoints().add_breakpoint(
            cxxdbg::dbg::source_position_info{fdoc->path().filename(), static_cast<unsigned>(line + 1)},
            [](auto && ...){}
        );
    });

    return std::move(menu);
}


void debug_source_code_widget_delegate::add_text_context_menu_actions(QMenu * menu,
                                                                      text_document * doc,
                                                                      size_t line,
                                                                      const std::wstring & selText) {

    auto fdoc = dynamic_cast<const cxxdbg::text_file_document*>(doc);
    if (!fdoc) {
        return;
    }

    QAction * addWatchAction = menu->addAction(tr("Add watch"));
    addWatchAction->setEnabled(!selText.empty());
    cxxdbg_connect(addWatchAction, &QAction::triggered, [this, selText] {
        dbg_.watch().add_watch(selText);
    });

    menu->addSeparator();

    auto file = dbg_.src_mdl().source(fdoc->path());

    createTextWidgetBreakpointMenu(menu, file, line);
    createTextWidgetThreadMenu(menu, file, line);

    menu->addSeparator();

    menu->addMenu(dbgUi_.debugStepIntoFuncMenu());
}


bool debug_source_code_widget_delegate::isThreadAtLine(const cxxdbg::dbg::source_file * file,
                                                   unsigned int line) {
    return cxxdbg::dbg::threads_at_line(dbg_, {file, line + 1}).empty();
}


bool debug_source_code_widget_delegate::isCurrentThreadAtLine(const cxxdbg::dbg::source_file * file,
                                                          unsigned int line) {
    auto pos = dbg_.curr_thread_pos();
    return pos.is_valid() && pos.file() == file && pos.line() == (line + 1);
}


bool debug_source_code_widget_delegate::isCurrentFrameAtLine(const cxxdbg::dbg::source_file * file,
                                                         unsigned int line) {
    auto pos = dbg_.curr_frame_pos();
    return pos.is_valid() && pos.file() == file && pos.line() == (line + 1);
}


std::unique_ptr<QMenu>
debug_source_code_widget_delegate::createMarksAreaBreakpointMenu(const cxxdbg::dbg::code_breakpoint * bp) {
    assert(bp && "breakpoint should no be null here");

    auto menu = std::make_unique<QMenu>();

    QAction * deleteAction = menu->addAction(QIcon::fromTheme("list-delete"), tr("Delete"));
    cxxdbg_connect(deleteAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().remove_breakpoint(bp);
    });

    QAction * enableAction = menu->addAction(tr("Enable"));
    enableAction->setEnabled(!bp->enabled());
    cxxdbg_connect(enableAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().enable_breakpoint(bp);
    });

    QAction * disableAction = menu->addAction(tr("Disable"));
    disableAction->setEnabled(bp->enabled());
    cxxdbg_connect(disableAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().disable_breakpoint(bp);
    });

    menu->addSeparator();

    QAction * conditionAction = menu->addAction(tr("Condition..."));
    cxxdbg_connect(conditionAction, &QAction::triggered, [this, bp] {
        dbgUi_.changeBreakpointCondition(bp);
    });

    QAction * hitCountAction = menu->addAction(tr("Hit count..."));
    cxxdbg_connect(hitCountAction, &QAction::triggered, [this, bp] {
        dbgUi_.changeBreakpointHitCount(bp);
    });

    return std::move(menu);
}


std::unique_ptr<QMenu>
debug_source_code_widget_delegate::createMarksAreaThreadMenu(const cxxdbg::dbg::thread * thrd) {
    assert(thrd && "thread should not be null here");

    auto menu = std::make_unique<QMenu>();

    QAction * switchToThreadAction = menu->addAction(tr("Switch to thread"));
    cxxdbg_connect(switchToThreadAction, &QAction::triggered, [this, thrd] {
        dbg_.set_current_thread(thrd);
    });

    return std::move(menu);
}


void debug_source_code_widget_delegate::createTextWidgetBreakpointMenu(QMenu * parent,
                                                                   const cxxdbg::dbg::source_file * file,
                                                                   unsigned int line) {
    // getting list of breakpoints at current line
    auto bps = cxxdbg::dbg::breakpoints_at_line(dbg_.breakpoints(), {file, line + 1});
    auto bp = bps.empty() ? nullptr : *std::ranges::begin(bps);
    bool canChangeBp = dbg_.state() == cxxdbg::dbg::application::state_t::initial ||
                       dbg_.state() == cxxdbg::dbg::application::state_t::loaded ||
                       dbg_.state() == cxxdbg::dbg::application::state_t::stopped;

    auto menu = parent->addMenu(tr("Breakpoint"));

    QAction * insertBreakpointAction = menu->addAction(tr("Insert"));
    insertBreakpointAction->setEnabled(canChangeBp);
    cxxdbg_connect(insertBreakpointAction, &QAction::triggered, [this, file, line] {
        dbg_.breakpoints().add_breakpoint(
            cxxdbg::dbg::source_position_info{file->path().filename(), line + 1},
            [](auto && ... x) {}
        );
    });

    menu->addSeparator();

    QAction * deleteBreakpointAction = menu->addAction(tr("Delete"));
    deleteBreakpointAction->setEnabled(canChangeBp && bp != nullptr);
    cxxdbg_connect(deleteBreakpointAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().remove_breakpoint(bp);
    });

    QAction * enableBreakpointAction = menu->addAction(tr("Enable"));
    enableBreakpointAction->setEnabled(canChangeBp && bp != nullptr && !bp->enabled());
    cxxdbg_connect(enableBreakpointAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().enable_breakpoint(bp);
    });

    QAction * disableBreakpointAction = menu->addAction(tr("Disable"));
    disableBreakpointAction->setEnabled(canChangeBp && bp != nullptr && bp->enabled());
    cxxdbg_connect(disableBreakpointAction, &QAction::triggered, [this, bp] {
        dbg_.breakpoints().disable_breakpoint(bp);
    });

    menu->addSeparator();

    QAction * breakpointConditionAction = menu->addAction(tr("Condition..."));
    breakpointConditionAction->setEnabled(canChangeBp && bp != nullptr);
    cxxdbg_connect(breakpointConditionAction, &QAction::triggered, [this, bp] {
        dbgUi_.changeBreakpointCondition(bp);
    });

    QAction * breakpointHitCountAction = menu->addAction(tr("Hit count..."));
    breakpointHitCountAction->setEnabled(canChangeBp && bp != nullptr);
    cxxdbg_connect(breakpointHitCountAction, &QAction::triggered, [this, bp] {
        dbgUi_.changeBreakpointHitCount(bp);
    });
}


void debug_source_code_widget_delegate::createTextWidgetThreadMenu(QMenu * parent,
                                                               const cxxdbg::dbg::source_file * file,
                                                               unsigned int line) {
    auto threads = dbg_.line_threads(file, line + 1);
    auto thread = threads.empty() ? nullptr : *std::ranges::begin(threads);

    auto menu = parent->addMenu(tr("Thread"));
    QAction * switchToThreadAction = menu->addAction(tr("Switch to thread"));
    switchToThreadAction->setEnabled(thread != nullptr &&
                                     dbg_.state() == cxxdbg::dbg::debugger::state_t::stopped);
    cxxdbg_connect(switchToThreadAction, &QAction::triggered, [this, thread] {
        dbg_.set_current_thread(thread);
    });
}


}
