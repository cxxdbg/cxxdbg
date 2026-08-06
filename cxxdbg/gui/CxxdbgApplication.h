// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#ifndef Q_MOC_RUN
#include "DebugUI.h"
#include "DocumentListUi.h"
#include "EventQueueApplication.h"
#include "QtSettingsStorage.h"
#include "cxxdbg/dbg/application.hpp"
#include "cxxdbg/dbg/forward.hpp"
#endif

#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QtWidgets/QMenu>


class QAction;


namespace cxxdbg::gui {

class MainWindow;


class CxxdbgApplication:
        public EventQueueApplication,
        public QtSettingsStorage,
        public DocumentListUi,
        public cxxdbg::dbg::application
{
    Q_OBJECT
public:
    /// Application state
    typedef cxxdbg::dbg::application::state_t state_t;

    /// Constructor, makes cxxdbg application
    explicit CxxdbgApplication(int & argc, char *argv[]);

    /// Destructor, destroys application
    virtual ~CxxdbgApplication() override;

    /// Sets main window
    void setMainWindow(MainWindow * wnd);

    /// Gets main window
    MainWindow * getMainWindow() { return mainWindow_; }

    /// Processes command line options
    void processCommandLine(const cxxdbg::dbg::command_line & cmd_line);

    /// Saves application settings
    void saveSettings();

    /// Returns pointer to close current source action
    QAction * closeCurrentSourceAction();

    /// Returns pointer to Start action
    QAction * debugStartAction();

    /// Returns pointer to Start with options action
    QAction * debugStartWithOptionsAction();

    /// Returns pointer to Stop action
    QAction * debugStopAction();

    /// Returns code viewer font
    QFont codeViewerFont() const;

    /// Sets code viewer font
    void setCodeViewerFont(const QFont & font);

    /// Shows information msgbox
    void show_info_message(const std::wstring & msg) override;

    /// Returns reference to common debug UI object
    DebugUI & dbgUi() { return dbgUi_; }

    /// Displays dialog for attaching to process and attaches it. Returns ID of attached process.
    unsigned long ask_pid_and_attach() override;

    /// Returns application icon
    QIcon appIcon() { return appIcon_; }

signals:
    /// Emitted when application state changed
    void stateChanged();

private:
    /// Shows temporary status message
    void show_status_message(const std::string & msg) override;

    /// Clears temporary status message
    void clear_status_message() override;

    /// Shows loading target progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog>
    make_loading_progress(const std::filesystem::path & name) override;

    /// Asks user for enter step until line number
    bool ask_step_until_line_number(unsigned int & line_number) override;

    /// Asks user about current running process termination. Returns true if user
    /// clicks Yes
    bool ask_terminate_confirmation() override;

public:
    bool ask_close_confirmation() override;

private:
    /// Returns pointer to main window
    QWidget * getMainWindow() const override;

    /// Makes progress dialog with specified title, satus text and range of values
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_progress_dialog(const std::string & title,
                                                                     const std::string & status,
                                                                     int min = 0,
                                                                     int max = 0) override;

    /// Makes launching progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_launching_progress() override;

    /// Makes stopping progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_stopping_progress() override;

    /// Makes terminating progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_terminating_progress(bool do_detach) override;

    /// Makes detaching progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_detaching_progress() override;

    /// Makes closing executable progress dialog
    std::shared_ptr<cxxdbg::dbg::progress_dialog> make_closing_progress() override;

    /// Shows attaching progress dialog until application state state becomes
    /// not 'initial' nor 'loading'.
    virtual std::shared_ptr<cxxdbg::dbg::progress_dialog> make_attaching_progress(const std::string & targ) override;

//    /// Initializes settings
//    void initSettings();

    /// Initializes common application actions
    void initActions();

    /// Updates actions status
    void updateActionsStatus();

    /// Initializes icons
    void initIcons();

    /// Makes progress dialog with specified title and text
    std::shared_ptr<cxxdbg::dbg::progress_dialog> makeProgressDialog(const QString & title,
                                                                   const QString & text);

    DebugUI dbgUi_;
    MainWindow * mainWindow_;                       ///< Pointer to main window
    QFont codeViewerFont_;                          ///< Font used in code viewer

    // common application actions

    QAction * closeCurrentSourceAction_;        ///< Close current source action

    QAction * debugStartAction_;                ///< Start action
    QAction * debugStartWithOptionsAction_;     ///< Start with options action
    QAction * debugStopAction_;                 ///< Stop action

    // application icons

    QIcon debugStartIcon_;                      ///< Icon for start debugging action
    QIcon debugStopIcon_;                       ///< Icon for stop debugging action
    QIcon appIcon_;                             ///< Application icon

    /// Connection to application state changed signal
    cxxdbg::scoped_signal_connection app_state_changed_con_;

    /// Connection to current source changed signal
    cxxdbg::scoped_signal_connection curr_source_changed_con_;
};


}
