// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "debug_source_code_widget_delegate.hpp"
#include "cxxdbg/app/signals.hpp"
#include <QMainWindow>


class QAction;
class QLabel;


namespace cxxdbg::gui {

class BreakpointsDockWidget;
class CallStackDockWidget;
class document_list_widget;
class TerminalDockWidget;
class LocalsDockWidget;
class RegistersDockWidget;
class SearchDialog;
class CxxdbgApplication;
class SourceTreeDockWidget;
class ThreadsDockWidget;
class WatchListDockWidget;


class MainWindow: public QMainWindow
{
    Q_OBJECT

    public:
        MainWindow(CxxdbgApplication & app,
                   QWidget *         parent = 0);

        ~MainWindow();

        /// Updates font in code viewer widgets
        void updateCodeViewerFont();

        /// Highlights search string in active source code widget
        void highlightSearchString(const QString & str,
                                   bool            mCase);

        /// Removes highlighting for search string
        void removeSearchStringHighlight();

        // shows goto line number dialog
        void gotoLineNumber();

        /// Shows temporary message in status bar
        void showStatusMessage(const QString & msg,
                               int             timeout = 0);

        /// Clears temporary message in status bar
        void clearStatusMessage();

        /// Sets permanent status message
        void setPermanentStatusMessage(const QString & msg);

    signals:

        /// Emited when canDoSearch property changed
        void canDoSearchChanged();

    protected:

        /// Called when user closes window
        void closeEvent(QCloseEvent * event);

    private:

        /// Saves state of window and all dock panels
        void saveState();

        /// Called when user clicks "Open source" menu
        void onOpenSource();

        /// Called when user clicks "Close source" menu
        void onCloseSource();

        /// Called when user clicks "Load executable" menu
        void onLoadExecutable();

        /// Called when user clicks "Attach to process" manu
        void onAttach();

        /// Called when user clicks "Connect to platform" menu
        void onConnectToPlatform();

        /// Called when user clicks "Disconnect from platform" menu
        void onDisconnectFromPlatform();

        /// Called when user clicks "Close executable" menu
        void onCloseExecutable();

        /// Called when user clicks "Start with options..." menu
        void onStartWithOptions();

        /// Called when user clicks "Continue debugging" menu
        void onContinueDebugging();

        /// Called when user clicks "Interrupt" menu
        void onInterrupt();

        /// Called when user clicks "Stop debugging" menu
        void onStopDebugging();

        /// Called when user clicks "Detach from process" menu
        void onDetachFromProcess();

        /// Updates menu status
        void updateMenuStatus();

        /// Called when application state changed
        void onAppStateChanged();

        /// Called when platform connection changed
        void onPlatformChanged();


        CxxdbgApplication &       app_;              ///< Reference to application object
        document_list_widget *         centralWidget_;    ///< Central widget
        SourceTreeDockWidget *  sourceTree_;       ///< Source tree dock widget
        LocalsDockWidget *      locals_;           ///< Locals dock widget
        WatchListDockWidget *   watch_;            ///< Watch dock widget
        RegistersDockWidget *   registers_;        ///< Registers dock widget
        ThreadsDockWidget *     threads_;          ///< Threads dock widget
        CallStackDockWidget *   callStack_;        ///< Call stack dock widget
        BreakpointsDockWidget * breakpoints_;      ///< Breakpoints dock widget
        TerminalDockWidget * exeTerm_;          ///< Executable terminal dock widget
        QLabel *                statusLabel_;      ///< Label containing status in status bad

        /// Label that displays platform connection status
        QLabel *                platformConnectionLabel_ = nullptr;

        /// Pointer to search dialog (or null if not created);
        SearchDialog *                      searchDlg_;
        QAction *                           findAction_;                ///< Find menu action
        QAction *                           findNextAction_;            ///< Find next action
        QAction *                           findPrevAction_;            ///< Find prev action
        QAction *                           gotoLineNumberAction_;      ///< GoTo line number action
        QAction *                           closeExecAction_;           ///< "Close executable" menu action
        QAction *                           startAction_;               ///< Start debugging menu action
        QAction *                           startWithOptionsAction_;    ///< Start with options menu action
        QAction *                           continueAction_;            ///< Continue debugging menu action
        QAction *                           interruptAction_;           ///< Interrupt menu action
        QAction *                           stopAction_;                ///< Stop debugging menu action
        QAction *                           detachAction_;              ///< Detach from process menu action
        QAction *                           connectToPlatformAction_;   ///< Connect to platform menu action
        QAction *                           disconnectFromPlatformAction_;////< Disconnect from platform menu action
        QAction *                           stepIntoAction_;            ///< Step into action
        QAction *                           stepOverAction_;            ///< Step over action
        QAction *                           stepOutAction_;             ///< Step out action
        QAction *                           stepInstIntoAction_;        ///< Step instruction into action
        QAction *                           stepInstOverAction_;        ///< Step instruction over action
        QAction *                           stepUntilAction_;           ///< Step until action
        cxxdbg::scoped_signal_connection state_con_;                      ///< State signal connection
        cxxdbg::scoped_signal_connection curr_source_con_;                ///< Current source changed signal connection
        cxxdbg::scoped_signal_connection curr_frame_con_;                 ///< Current frame signal connection
        cxxdbg::scoped_signal_connection proc_exit_con_;                  ///< Connection to process exited signal
        cxxdbg::scoped_signal_connection platform_con_;                   ///< Connection to platform change signal
};


}
