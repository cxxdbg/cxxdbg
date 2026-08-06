// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "CxxdbgApplication.h"
#include "AttachDialog.h"
#include "BreakpointConditionDialog.h"
#include "BreakpointHitCountDialog.h"
#include "LaunchExecutableDialog.h"
#include "MainWindow.h"
#include "CxxdbgProgressDialog.h"
#include "StepUntilDialog.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/text_file_document.hpp"
#include "cxxdbg/dbg/appcore/appcore.hpp"
#include "cxxdbg/dbg/application.hpp"
#include "cxxdbg/dbg/code_breakpoint.hpp"
#include "cxxdbg/dbg/source_tree.hpp"
#include <QAction>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QStyleFactory>


namespace cxxdbg::gui {


CxxdbgApplication::CxxdbgApplication(int & argc, char *argv[]):
EventQueueApplication(argc, argv),
cxxdbg::dbg::application(*this, *this, cxxdbg::dbg::appcore::make_debugger_impl(*this), *this),
dbgUi_(dbg(), documents(), this, &trees().sorted_sources()),
mainWindow_(nullptr),
codeViewerFont_(defaultCodeViewerFont()) {

    // use fusion theme by default on macos and windows
#if defined(__APPLE__) || defined(_WIN32)
    setStyle(QStyleFactory::create("fusion"));
#endif

    initIcons();
    initActions();

    // loading code viewer font
    {
        QSettings settings;
        QVariant val = settings.value("ui/codeview/font");
        if (!val.isNull()) {
            QFont fnt;
            if (fnt.fromString(val.toString())) {
                codeViewerFont_ = fnt;
            }
        }
    }

    // updating actions status when application state changed
    app_state_changed_con_ = connect_state_changed([this]() {
        updateActionsStatus();
    });

    // updating actions status when current source changed
    curr_source_changed_con_ = documents().current_changed.connect([this]() {
        updateActionsStatus();
    });
}


CxxdbgApplication::~CxxdbgApplication() {
}


void CxxdbgApplication::setMainWindow(MainWindow * wnd) {
    dbgUi().setMainWindow(wnd);
    mainWindow_ = wnd;
}


void CxxdbgApplication::processCommandLine(const cxxdbg::dbg::command_line & cmd_line) {
    process_command_line(cmd_line);
}


void CxxdbgApplication::saveSettings() {
    dbgUi_.saveSettings();
}


QAction * CxxdbgApplication::closeCurrentSourceAction() {
    return closeCurrentSourceAction_;
}


QAction * CxxdbgApplication::debugStartAction() {
    return debugStartAction_;
}


QAction * CxxdbgApplication::debugStartWithOptionsAction() {
    return debugStartWithOptionsAction_;
}


QAction * CxxdbgApplication::debugStopAction() {
    return debugStopAction_;
}


QFont CxxdbgApplication::codeViewerFont() const {
    return codeViewerFont_;
}


void CxxdbgApplication::setCodeViewerFont(const QFont & font) {
    codeViewerFont_ = font;
    QSettings().setValue("ui/codeview/font", font.toString());
    mainWindow_->updateCodeViewerFont();
}

void CxxdbgApplication::show_status_message(const std::string & msg) {
    mainWindow_->showStatusMessage(QString::fromStdString(msg));
}


void CxxdbgApplication::clear_status_message() {
    mainWindow_->clearStatusMessage();
}


std::shared_ptr<cxxdbg::dbg::progress_dialog>
CxxdbgApplication::make_loading_progress(const std::filesystem::path & name) {
    std::shared_ptr<cxxdbg::dbg::progress_dialog> dlg { new CxxdbgProgressDialog {
        tr("Loading executable"),
        tr("Loading executable '%1'...").arg(QString::fromStdWString(name.wstring())),
        mainWindow_
    } };
    return dlg;
}


bool CxxdbgApplication::ask_step_until_line_number(unsigned int & line_number) {
    return dbgUi().askStepUntilLineNumber(line_number, mainWindow_);
}


bool CxxdbgApplication::ask_terminate_confirmation() {
    QString targetName = QString::fromStdString(exe_path());
    QMessageBox msg(QMessageBox::Question,
                    tr("Closing executable"),
                    tr("Executable '%1' is running. Are you sure you want kill it?").arg(targetName),
                    QMessageBox::Yes | QMessageBox::No,
                    mainWindow_);

    return msg.exec() == QMessageBox::Yes;
}


bool CxxdbgApplication::ask_close_confirmation() {
    QString targetName = QString::fromStdString(exe_path());
    QMessageBox msg(QMessageBox::Question,
                    tr("Close opened executable"),
                    tr("Executable '%1' already opened. "
                       "Are you sure you want close it and open another executable?").arg(targetName),
                    QMessageBox::Yes | QMessageBox::No,
                    mainWindow_);
    return msg.exec() == QMessageBox::Yes;
}


QWidget * CxxdbgApplication::getMainWindow() const {
    return mainWindow_;
}


std::shared_ptr<cxxdbg::dbg::progress_dialog>
CxxdbgApplication::make_progress_dialog(const std::string & title, const std::string & status, int min, int max) {
    return std::make_shared<CxxdbgProgressDialog>(QString::fromStdString(title),
                                                QString::fromStdString(status),
                                                min,
                                                max,
                                                mainWindow_);
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_launching_progress() {
    QString msg = tr("Launching executable '%1'...").arg(QString::fromStdString(exe_path()));
    return makeProgressDialog(tr("Launching executable"), msg);
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_stopping_progress() {
    return makeProgressDialog(tr("Interrupting process"), tr("Interrupting process..."));
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_terminating_progress(bool detach) {
    return makeProgressDialog(tr("Terminating process"), tr("Terminating process..."));
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_detaching_progress() {
    return makeProgressDialog(tr("Detaching from process"), tr("Detaching from process..."));
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_closing_progress() {
    return makeProgressDialog(tr("Closing executable"), tr("Closing executable..."));
}


std::shared_ptr<cxxdbg::dbg::progress_dialog> CxxdbgApplication::make_attaching_progress(const std::string & targ) {
    QString msg = tr("Attaching to process %1...").arg(QString::fromStdString(targ));
    return makeProgressDialog(tr("Attaching to process"), msg);
}


void CxxdbgApplication::initActions() {
    // close current source action
    closeCurrentSourceAction_ = new QAction(tr("Close source"), this);

    // Start action
    debugStartAction_ = new QAction(tr("Start"), this);
#ifdef __APPLE__
    debugStartAction_->setShortcut(QKeySequence(tr("CTRL+R")));
#else
    debugStartAction_->setShortcut(QKeySequence(tr("F5")));
#endif
    debugStartAction_->setIcon(debugStartIcon_);
    cxxdbg_connect(debugStartAction_, &QAction::triggered, [this](auto && ...) {
        launch(def_launch_opts(), false);
    });

    // Start with options action
    debugStartWithOptionsAction_ = new QAction(tr("Start with options..."), this);
    cxxdbg_connect(debugStartWithOptionsAction_, &QAction::triggered, [this] {
        // displaying launch dialog
        LaunchExecutableDialog launchDlg(dbg(), def_launch_opts(), mainWindow_);
        if (launchDlg.exec() != QDialog::Accepted)
            return;

        launch(launchDlg.launch_opts(), launchDlg.saveOpts());
    });

    // Stop action
    debugStopAction_ = new QAction(tr("Stop"), this);
    debugStopAction_->setShortcut(QKeySequence(tr("SHIFT+F5")));
    debugStopAction_->setIcon(debugStopIcon_);
    cxxdbg_connect(debugStopAction_, &QAction::triggered, [this] { terminate(); });

    updateActionsStatus();
}


void CxxdbgApplication::updateActionsStatus() {

    dbgUi().updateActionsStatus();

    cxxdbg::dbg::application::state_t st = state();

    // enable "start" / "start with options" action only in loaded state
    debugStartAction_->setEnabled(st == cxxdbg::dbg::application::state_t::loaded);
    debugStartWithOptionsAction_->setEnabled(st == cxxdbg::dbg::application::state_t::loaded);

    // enable "stop debugging" action only in running / stopped state
    debugStopAction_->setEnabled(st == cxxdbg::dbg::application::state_t::running ||
                                 st == cxxdbg::dbg::application::state_t::stopped);
}


void CxxdbgApplication::initIcons() {
    QString imagesDir = this->applicationDirPath() + "/../share/cxxdbg/images";

    debugStartIcon_ = makeSvgIcon(imagesDir, "play");
    debugStopIcon_ = makeSvgIcon(imagesDir, "stop");

    appIcon_ = QIcon(imagesDir + "/icon.ico");
}


std::shared_ptr<cxxdbg::dbg::progress_dialog>
CxxdbgApplication::makeProgressDialog(const QString & title, const QString & text) {
    std::shared_ptr<cxxdbg::dbg::progress_dialog> dlg { new CxxdbgProgressDialog {
        title, text, mainWindow_
    } };
    return dlg;
}


void CxxdbgApplication::show_info_message(const std::wstring & msg) {
    QMessageBox::information(mainWindow_, tr("Information"), QString::fromStdWString(msg));
}


unsigned long CxxdbgApplication::ask_pid_and_attach() {
    AttachDialog dlg{*this, mainWindow_};
    if (dlg.exec() == QDialog::Accepted) {
        return dlg.pid();
    }

    return 0;
}


}
