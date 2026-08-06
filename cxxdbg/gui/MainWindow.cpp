// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "MainWindow.h"
#include "AboutDialog.h"
#include "AttachDialog.h"
#include "BreakpointsDockWidget.h"
#include "CallStackDockWidget.h"
#include "DbgSettingsDialog.h"
#include "DbgTerminalWidget.h"
#include "debug_document_widget_factory.hpp"
#include "document_list_widget.hpp"
#include "EditAddressBreakpointDialog.h"
#include "EditFunctionBreakpointDialog.h"
#include "EditSourcePosBreakpointDialog.h"
#include "FileSystemDockWidget.h"
#include "LaunchExecutableDialog.h"
#include "LoadExecutableDialog.h"
#include "LocalsDockWidget.h"
#include "PlatformConnectDialog.h"
#include "RegistersDockWidget.h"
#include "SearchDialog.h"
#include "CxxdbgApplication.h"
#include "CxxdbgDockWidget.h"
#include "SourceTreeDockWidget.h"
#include "TerminalDockWidget.h"
#include "ThreadsDockWidget.h"
#include "WatchListDockWidget.h"
#include "LineNumberDialog.h"
#include "AddWatchpointDialog.hpp"
#include "FunctionNameDialog.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/async/result.hpp"
#include "cxxdbg/dbg/dbg_terminal.hpp"
#include "cxxdbg/dbg/debug_settings.hpp"
#include "cxxdbg/dbg/debugger.hpp"
#include "cxxdbg/dbg/source_file.hpp"
#include "cxxdbg/dbg/source_position.hpp"
#include "cxxdbg/dbg/stack_frame.hpp"
#include "cxxdbg/dbg/stop_reason.hpp"

#include <filesystem>
#include <QClipboard>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QTabBar>
#include <QToolBar>


namespace fs = std::filesystem;


namespace cxxdbg::gui {


MainWindow::MainWindow(CxxdbgApplication & app, QWidget *parent):
QMainWindow(parent),
app_(app),
searchDlg_(nullptr)
{
    setMinimumSize(800, 600);
    setWindowState(Qt::WindowMaximized);
    setWindowIcon(app_.appIcon());
    setWindowTitle("CxxDbg");
    app_.setMainWindow(this);

    setDockNestingEnabled(true);
    //setDocumentMode(true);

    //////////////////////////////////////////////////////////////////////
    // Dock widgets
    // TODO: split into functions

    // source tree dock widget
    sourceTree_ = new SourceTreeDockWidget(app_);
    this->addDockWidget(Qt::LeftDockWidgetArea, sourceTree_);

    // file system tree dock widget
    FileSystemDockWidget * fileSysDock = new FileSystemDockWidget(this);
    this->addDockWidget(Qt::LeftDockWidgetArea, fileSysDock);
    cxxdbg_connect(fileSysDock, &FileSystemDockWidget::openSelected, [this](const QString & path) {
        app_.documents().open_text<true>(fs::path(path.toStdWString()), false);
    });

    // make left tabs
    tabifyDockWidget(sourceTree_, fileSysDock);


    // locals dock widget
    locals_ = new LocalsDockWidget(app_.dbg(), app_);
    addDockWidget(Qt::BottomDockWidgetArea, locals_);

    // watch dock widget
    watch_ = new WatchListDockWidget{app_.dbg(), app_, app_.dbgUi()};
    addDockWidget(Qt::BottomDockWidgetArea, watch_);

    // registers dock widget
    registers_ = new RegistersDockWidget(app_.dbg());
    addDockWidget(Qt::BottomDockWidgetArea, registers_);

    // threads dock widget
    threads_ = new ThreadsDockWidget(app_.dbg(), app_, app_.dbgUi());
    addDockWidget(Qt::BottomDockWidgetArea, threads_);

    // make tabls for left bottom docks
    tabifyDockWidget(locals_, watch_);
    tabifyDockWidget(watch_, registers_);
    tabifyDockWidget(registers_, threads_);

    // breakpoints dock widget
    breakpoints_ = new BreakpointsDockWidget(app_.dbg(), app_.dbgUi());
    addDockWidget(Qt::BottomDockWidgetArea, breakpoints_);
    cxxdbg_connect(breakpoints_, &BreakpointsDockWidget::goToSourceClicked, [this](auto && pos) {
        app_.show_pos(pos);
    });

    // call stack dock widget
    callStack_ = new CallStackDockWidget(app_.dbg(), app_, app_.dbgUi());
    addDockWidget(Qt::BottomDockWidgetArea, callStack_);

    // debugger terminal dock widget
    CxxdbgDockWidget *termDock = new CxxdbgDockWidget(tr("Debugger terminal"), this);
    termDock->setObjectName("debugger_terminal");
    termDock->setWidget(new DbgTerminalWidget(app_.dbg(), app_.dbg_term(), app_.interp(), this));
    addDockWidget(Qt::BottomDockWidgetArea, termDock);

    // executable terminal dock widget
    exeTerm_ = new TerminalDockWidget{app_.exe_term()};
    exeTerm_->setObjectName("executable_terminal");
    addDockWidget(Qt::BottomDockWidgetArea, exeTerm_);

    // make tabs for breakpoints, call stack, and terminal
    tabifyDockWidget(breakpoints_, callStack_);
    tabifyDockWidget(callStack_, termDock);
    tabifyDockWidget(termDock, exeTerm_);


    // making first tabs active in all tabified dock widgets
    QList<QTabBar*> tabs = findChildren<QTabBar*>();
    for (auto it = tabs.begin(), end = tabs.end(); it != end; ++it) {
        QTabBar * tabBar = *it;
        if (tabBar->count() > 0) {
            tabBar->setCurrentIndex(0);
        }
    }


    //////////////////////////////////////////////////////////////////////
    // Menu
    // TODO: split into functions

    QMenuBar * menu = new QMenuBar;
    this->setMenuBar(menu);

    // File menu
    {
        QMenu * fileMenu = new QMenu(tr("File"));

        QAction * openExecAction = fileMenu->addAction(tr("Open..."));
        openExecAction->setShortcut(QKeySequence("CTRL+O"));
        openExecAction->setIcon(app_.dbgUi().makeSvgIcon("open"));
        cxxdbg_connect(openExecAction, &QAction::triggered, [this] { onLoadExecutable(); });

        closeExecAction_ = fileMenu->addAction(tr("Close"));
        cxxdbg_connect(closeExecAction_, &QAction::triggered, [this] { onCloseExecutable(); });

        QAction * attachAction = fileMenu->addAction(tr("Attach..."));
        cxxdbg_connect(attachAction, &QAction::triggered, [this] { onAttach(); });

        detachAction_ = fileMenu->addAction(tr("Detach"));
        cxxdbg_connect(detachAction_, &QAction::triggered, [this] { onDetachFromProcess(); });

        fileMenu->addSeparator();

        connectToPlatformAction_ = fileMenu->addAction(tr("Connect to platform..."));
        cxxdbg_connect(connectToPlatformAction_, &QAction::triggered, [this] { onConnectToPlatform(); });

        disconnectFromPlatformAction_ = fileMenu->addAction(tr("Disconnect from platform"));
        cxxdbg_connect(disconnectFromPlatformAction_, &QAction::triggered, [this] { onDisconnectFromPlatform(); });

        fileMenu->addSeparator();

        auto openSourceAction = fileMenu->addAction(tr("Open source..."));
        openSourceAction->setIcon(app_.dbgUi().makeSvgIcon("open"));
        cxxdbg_connect(openSourceAction, &QAction::triggered, [this] { onOpenSource(); });

        fileMenu->addAction(app_.closeCurrentSourceAction());
        cxxdbg_connect(app_.closeCurrentSourceAction(), &QAction::triggered, [this] { onCloseSource(); });

        fileMenu->addSeparator();

        QAction * exitAction = fileMenu->addAction(tr("Exit"));
        exitAction->setShortcut(QKeySequence("ALT+F4"));
        cxxdbg_connect(exitAction, &QAction::triggered, [this] { close(); });

        menu->addMenu(fileMenu);
    }

    // Edit menu
    {
        QMenu * editMenu = new QMenu(tr("Edit"));
        menu->addMenu(editMenu);

        // copy action
        auto copy = editMenu->addAction(tr("Copy"));
        copy->setIcon(QIcon::fromTheme("edit-copy"));
        copy->setShortcut(QKeySequence{QKeySequence::Copy});
        cxxdbg_connect(copy, &QAction::triggered, [this] {
            assert(app_.documents().can_copy() && "can't copy text");
            QGuiApplication::clipboard()->setText(QString::fromStdWString(app_.documents().copy()));
        });
        auto update_copy_state = [this, copy] {
            copy->setEnabled(app_.documents().can_copy());
        };
        update_copy_state();
        app_.documents().current_edit_options_changed.connect(update_copy_state);

        editMenu->addSeparator();

        // select all action
        auto selectAll = editMenu->addAction("Select all");
        selectAll->setIcon(QIcon::fromTheme("edit-select-all"));
        selectAll->setShortcut(QKeySequence{QKeySequence::SelectAll});
        cxxdbg_connect(selectAll, &QAction::triggered, [this] {
            app_.documents().select_all();
        });
        auto update_select_all_state = [this, selectAll] {
            selectAll->setEnabled(app_.documents().can_select_all());
        };
        update_select_all_state();
        app_.documents().current_edit_options_changed.connect(update_select_all_state);

        editMenu->addSeparator();

        // find action
        findAction_ = editMenu->addAction(tr("Find..."));
        findAction_->setShortcut(QKeySequence("CTRL+F"));
        findAction_->setIcon(QIcon::fromTheme("edit-find"));
        cxxdbg_connect(findAction_, &QAction::triggered, [this] {
            if (searchDlg_ == nullptr) {
                searchDlg_ = new SearchDialog(centralWidget_, this);
            }

            searchDlg_->show();
            searchDlg_->raise();
            searchDlg_->activateWindow();
        });

        // find next action
        findNextAction_ = editMenu->addAction(tr("Find next"));
        findNextAction_->setShortcut(QKeySequence("F3"));
        findNextAction_->setEnabled(false);
        cxxdbg_connect(findNextAction_, &QAction::triggered, [this] {
            centralWidget_->searchNext();
        });

        // find prev action
        findPrevAction_ = editMenu->addAction(tr("Find previous"));
        findPrevAction_->setShortcut(QKeySequence("SHIFT+F3"));
        findPrevAction_->setEnabled(false);
        cxxdbg_connect(findPrevAction_, &QAction::triggered, [this] {
            centralWidget_->searchPrev();
        });

        // goto line number action
        gotoLineNumberAction_ = editMenu->addAction(tr("Go to line..."));
        gotoLineNumberAction_->setShortcut(QKeySequence("CTRL+L"));
        gotoLineNumberAction_->setEnabled(false);
        // connecting not directly to central widget, because it is
        // not initialized yet
        cxxdbg_connect(gotoLineNumberAction_, &QAction::triggered, [this] { gotoLineNumber(); });
    }

    // Debug menu
    {
        QMenu * debugMenu = new QMenu(tr("Debug"));
        menu->addMenu(debugMenu);

        debugMenu->addAction(app_.debugStartAction());
        debugMenu->addAction(app_.debugStartWithOptionsAction());
        debugMenu->addAction(app_.dbgUi().debugContinueAction());
        debugMenu->addAction(app_.dbgUi().debugInterruptAction());
        debugMenu->addAction(app_.debugStopAction());

        debugMenu->addSeparator();

        debugMenu->addAction(app_.dbgUi().debugStepIntoAction());
        debugMenu->addAction(app_.dbgUi().debugStepOverAction());
        debugMenu->addAction(app_.dbgUi().debugStepOutAction());
        debugMenu->addAction(app_.dbgUi().debugStepInstIntoAction());
        debugMenu->addAction(app_.dbgUi().debugStepInstOverAction());
        debugMenu->addAction(app_.dbgUi().debugStepUntilAction());

        debugMenu->addSeparator();
        debugMenu->addMenu(app_.dbgUi().debugStepIntoFuncMenu());
        
        // toolbar
        QToolBar* toolBar = addToolBar(tr("Debug"));
        toolBar->setObjectName("debugToolBar");
        toolBar->setIconSize(QSize(16,16));
        toolBar->addAction(app_.debugStartAction());
        toolBar->addAction(app_.dbgUi().debugContinueAction());
        toolBar->addAction(app_.dbgUi().debugInterruptAction());
        toolBar->addAction(app_.debugStopAction());
        toolBar->addSeparator();
        
        toolBar->addAction(app_.dbgUi().debugStepIntoAction());
        toolBar->addAction(app_.dbgUi().debugStepOverAction());
        toolBar->addAction(app_.dbgUi().debugStepOutAction());
    }

    // Breakpoint menu
    {
        QMenu * debugMenu = new QMenu(tr("Breakpoint"));
        menu->addMenu(debugMenu);

        debugMenu->addAction(app_.dbgUi().toggleBreakpointAction());

        debugMenu->addSeparator();

        debugMenu->addAction(app_.dbgUi().addSourcePosBreakpointAction());
        debugMenu->addAction(app_.dbgUi().addFunctionBreakpointAction());
        debugMenu->addAction(app_.dbgUi().addAddressBreakpointAction());

        debugMenu->addSeparator();

        debugMenu->addAction(app_.dbgUi().addExceptionThrawnBreakpointAction());
        debugMenu->addAction(app_.dbgUi().addExceptionCaughtBreakpointAction());

        debugMenu->addSeparator();

        debugMenu->addAction(app_.dbgUi().addWatchpointAction());

        debugMenu->addSeparator();

        debugMenu->addAction(app_.dbgUi().deleteAllBreakpointsAction());
        debugMenu->addAction(app_.dbgUi().enableAllBreakpointsAction());
        debugMenu->addAction(app_.dbgUi().disableAllBreakpointsAction());
        
        // toolbar
        QToolBar * toolBar = addToolBar(tr("Breakpoint"));
        toolBar->setObjectName("breakpointToolBar");
        toolBar->setIconSize(QSize(16,16));
        toolBar->addAction(app_.dbgUi().toggleBreakpointAction());
        toolBar->addSeparator();
        toolBar->addAction(app_.dbgUi().addSourcePosBreakpointAction());
        toolBar->addAction(app_.dbgUi().addFunctionBreakpointAction());
        toolBar->addAction(app_.dbgUi().addAddressBreakpointAction());
        toolBar->addAction(app_.dbgUi().deleteAllBreakpointsAction());
        toolBar->addAction(app_.dbgUi().enableAllBreakpointsAction());
        toolBar->addAction(app_.dbgUi().disableAllBreakpointsAction());
    }

    // Tools menu
    {
        QMenu * toolsMenu = new QMenu(tr("Tools"));
        menu->addMenu(toolsMenu);

        QAction * settingsAction = toolsMenu->addAction(QIcon::fromTheme("preferences-other"),
                                                        tr("Settings..."));
        cxxdbg_connect(settingsAction, &QAction::triggered, [this] {
            DbgSettingsDialog settingsDlg(app_, this);
            // cxxdbg_connect(&settingsDlg, &DbgSettingsDialog::applyClicked, [] {

            // });
            settingsDlg.exec();
        });
    }

    // View menu
    {
        QMenu * viewMenu = createPopupMenu();
        viewMenu->setTitle(tr("View"));
        menu->addMenu(viewMenu);
    }

    // Help menu
    {
        QMenu * helpMenu = new QMenu(tr("Help"));
        menu->addMenu(helpMenu);
        QAction * aboutAction = helpMenu->addAction(tr("About..."));
        cxxdbg_connect(aboutAction, &QAction::triggered, []() {
            AboutDialog dlg{tr("CxxDbg")};
            dlg.exec();
        });
    }

    // status bar
    statusLabel_ = new QLabel;
    statusBar()->addWidget(statusLabel_);
    platformConnectionLabel_ = new QLabel;
    statusBar()->addPermanentWidget(platformConnectionLabel_);
    platform_con_ = app_.dbg().connected_platform_changed.connect([this] {
        onPlatformChanged();
    });
    onPlatformChanged();

    // central widget
    centralWidget_ = new document_list_widget(app_.documents(),
                                              std::make_shared<debug_document_widget_factory>(app_.dbg(), app_.dbgUi()));
    setCentralWidget(centralWidget_);
    centralWidget_->setTextFont(app_.codeViewerFont());
    cxxdbg_connect(centralWidget_, &document_list_widget::openSelected, [this] { onOpenSource(); });
    cxxdbg_connect(centralWidget_, &document_list_widget::canDoSearchChanged, [this] {
        bool canSearch = centralWidget_->canDoSearch();
        findNextAction_->setEnabled(canSearch);
        findPrevAction_->setEnabled(canSearch);

        emit canDoSearchChanged();
    });

    setUnifiedTitleAndToolBarOnMac(true);

    // update menu status when current source file changed
    curr_source_con_ = app_.documents().current_changed.connect([this] {
        updateMenuStatus();
    });

    // handle state changing
    state_con_ = app_.connect_state_changed([this]() { onAppStateChanged(); });

    // update menu
    updateMenuStatus();

    // restoring size
    QVariant geom = QSettings().value("main_window_geom");
    if (geom.isValid()) {
        restoreGeometry(geom.toByteArray());
    }

    // restoring panel states
    QVariant state = QSettings().value("main_window_state");
    if (state.isValid()) {
        restoreState(state.toByteArray());
    }

    // update window state
    onAppStateChanged();

    // display temporary status message on process exit
    proc_exit_con_ = app_.dbg().process_exited().connect([this](int ecode) {
        QString ecodeStr = QString::number(ecode);
        statusBar()->showMessage(tr("Process exited with exit code %1").arg(ecodeStr), 5000);
    });
}


MainWindow::~MainWindow() {

}


void MainWindow::updateCodeViewerFont() {
    centralWidget_->setTextFont(app_.codeViewerFont());
}


void MainWindow::highlightSearchString(const QString & str, bool mCase) {
    centralWidget_->highlightSearchString(str, mCase);
}


void MainWindow::removeSearchStringHighlight() {
    centralWidget_->removeSearchStringHighlight();
}


void MainWindow::gotoLineNumber() {
    centralWidget_->gotoLineNumber();
}


void MainWindow::showStatusMessage(const QString & msg, int timeout) {
    statusBar()->showMessage(msg, timeout);
}


void MainWindow::clearStatusMessage() {
    statusBar()->clearMessage();
}


void MainWindow::setPermanentStatusMessage(const QString & msg) {
    statusLabel_->setText(msg);
}


void MainWindow::closeEvent(QCloseEvent * event) {
    // closing executable (and stopping if started)
    if (app_.state() != cxxdbg::dbg::application::state_t::initial) {
        if (!app_.close()) {
            event->ignore();
            return;
        }
    }

    // saving window state
    saveState();

    event->accept();
}


void MainWindow::saveState() {
    // saving application settings
    app_.saveSettings();;

    // saving panels states
    threads_->saveState();
    callStack_->saveState();
    breakpoints_->saveState();
    locals_->saveState();
    watch_->saveState();

    // saving panels layout
    QByteArray state = QMainWindow::saveState();
    QSettings().setValue("main_window_state", QVariant(state));

    QByteArray geom = saveGeometry();
    QSettings().setValue("main_window_geom", QVariant(geom));
}


void MainWindow::onOpenSource() {
    // show select file dialog
    QString fileName = QFileDialog::getOpenFileName(this, tr("Please select source file for opening"));
    if (fileName.isEmpty())
        return;

    // open selected file
    app_.documents().open_text<true>(fs::path(fileName.toStdWString()), true);
}


void MainWindow::onCloseSource() {
    // close current file
    app_.documents().close(app_.documents().current());
}


void MainWindow::onLoadExecutable() {
    // displaying load executable dialog
    LoadExecutableDialog dlg{app_, this};
    dlg.exec();
}


void MainWindow::onAttach() {
    app_.ask_pid_and_attach();
}


void MainWindow::onConnectToPlatform() {
    // displaying platform connection dialog
    PlatformConnectDialog dlg{app_, this};
    dlg.exec();
}


void MainWindow::onDisconnectFromPlatform() {
    app_.ask_and_disconnect();
}


void MainWindow::onCloseExecutable() {
    app_.close();
}


void MainWindow::onStartWithOptions() {

    // displaying launch dialog
    LaunchExecutableDialog launchDlg(app_.dbg(), app_.def_launch_opts(), this);
    if (launchDlg.exec() != QDialog::Accepted)
        return;

    app_.launch(launchDlg.launch_opts(), launchDlg.saveOpts());
}


void MainWindow::onContinueDebugging() {
    app_.run();
}


void MainWindow::onInterrupt() {
    app_.stop();
}


void MainWindow::onStopDebugging() {
    app_.terminate();
}


void MainWindow::onDetachFromProcess() {
    app_.detach();
}


void MainWindow::updateMenuStatus() {
    // disable or enable "Close source" menu action
    app_.closeCurrentSourceAction()->setEnabled(app_.documents().current() != nullptr);

    cxxdbg::dbg::application::state_t st = app_.state();

    // disable close executable action for initial state
    closeExecAction_->setEnabled(st != cxxdbg::dbg::application::state_t::initial);

    // enable "detach from process" action only in running / stopped state
    detachAction_->setEnabled(st == cxxdbg::dbg::application::state_t::running ||
                              st == cxxdbg::dbg::application::state_t::stopped);

    if (app_.state() == cxxdbg::dbg::application::state_t::connecting) {
        connectToPlatformAction_->setEnabled(false);
        disconnectFromPlatformAction_->setEnabled(false);
    } else {
        connectToPlatformAction_->setEnabled(true);
        auto enableDisconnect = app_.dbg().connected_platform() != app_.dbg().platforms().host();
        disconnectFromPlatformAction_->setEnabled(enableDisconnect);
    }
    
    //gotoLineNumberAction_->setEnabled(centralWidget_->tabsCount());
    gotoLineNumberAction_->setEnabled(app_.documents().current() != nullptr);
}


void MainWindow::onAppStateChanged() {

    // activate main window if stopped
    if (app_.state() == cxxdbg::dbg::application::state_t::stopped) {
        activateWindow();
    }

    // updating status bar text
    statusLabel_->setText(QString::fromStdWString(app_.status_text()));

    // update menu status
    updateMenuStatus();

    // updating platform connection status
    onPlatformChanged();
}


void MainWindow::onPlatformChanged() {
    QString txt;

    if (app_.state() == cxxdbg::dbg::application::state_t::connecting) {
        txt = "Connecting to platform ";
        txt += QString::fromStdString(app_.dbg().connected_platform()->name());
        txt += "...";
    } else {
        txt = "Connected to platform: ";
        txt += QString::fromStdString(app_.dbg().connected_platform()->name());
    }

    platformConnectionLabel_->setText(txt);

    updateMenuStatus();
}


}
