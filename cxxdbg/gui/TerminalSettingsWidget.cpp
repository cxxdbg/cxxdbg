// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file TerminalSettingsWidget.cpp
/// Contains implementation of the TerminalSettingsWidget class.

#include "TerminalSettingsWidget.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>


namespace cxxdbg::gui {


TerminalSettingsWidget::TerminalSettingsWidget(const cxxdbg::dbg::term_settings & sett,
                                               QWidget * parent):
QWidget(parent) {
    auto formLayout = new FormLayout(this);
    formLayout->setContentsMargins({});

    builtinButton_ = new QRadioButton(tr("Use built-in terminal emulator"));
    formLayout->addRow(builtinButton_);
    builtinButton_->setChecked(sett.tkind() == cxxdbg::dbg::term_settings::term_kind::builtin);
    cxxdbg_connect(builtinButton_, &QRadioButton::toggled, [this] { updateGroups(); });
    cxxdbg_connect(builtinButton_, &QRadioButton::toggled, [this] { changed(); });

    xtermButton_ = new QRadioButton(tr("Use xterm terminal"), this);
    formLayout->addRow(xtermButton_);
    xtermButton_->setChecked(sett.tkind() == cxxdbg::dbg::term_settings::term_kind::xterm);
    cxxdbg_connect(xtermButton_, &QRadioButton::toggled, [this] { updateGroups(); });
    cxxdbg_connect(xtermButton_, &QRadioButton::toggled, [this] { changed(); });

    xtermGroup_ = new QGroupBox(tr("xterm settings"));
    formLayout->addRow(xtermGroup_);
    auto xtermGroupLayout = new FormLayout(xtermGroup_);

    xtermPathEdit_ = new QLineEdit(this);
    xtermGroupLayout->addRow(tr("Path to xterm:"), xtermPathEdit_);
    xtermPathEdit_->setText(QString::fromStdString(sett.xterm_path().string()));
    cxxdbg_connect(xtermPathEdit_, &QLineEdit::textEdited, [this] { changed(); });

    xtermCloseCheck_ = new QCheckBox(tr("Close xterm after process exit"), this);
    xtermGroupLayout->addRow(xtermCloseCheck_);
    xtermCloseCheck_->setChecked(sett.close_xterm());
    cxxdbg_connect(xtermCloseCheck_, &QCheckBox::toggled, [this] { changed(); });

    customButton_ = new QRadioButton(tr("Use custom terminal"), this);
    customButton_->setChecked(sett.tkind() == cxxdbg::dbg::term_settings::term_kind::custom);
    formLayout->addRow(customButton_);
    cxxdbg_connect(customButton_, &QRadioButton::toggled, [this] { updateGroups(); });
    cxxdbg_connect(customButton_, &QRadioButton::toggled, [this] { changed(); });

    customGroup_ = new QGroupBox(tr("Custom terminal settings"));
    formLayout->addRow(customGroup_);
    auto customTermGroupLayout = new FormLayout(customGroup_);

    customCommandEdit_ = new QLineEdit(this);
    customTermGroupLayout->addRow(tr("Custom command:"), customCommandEdit_);
    customCommandEdit_->setText(QString::fromStdString(sett.custom_term_cmd()));
    cxxdbg_connect(xtermPathEdit_, &QLineEdit::textEdited, [this] { changed(); });

    updateGroups();
}


cxxdbg::dbg::term_settings TerminalSettingsWidget::termSettings() const {
    auto tkind = cxxdbg::dbg::term_settings::term_kind::xterm;

    if (builtinButton_->isChecked()) {
        tkind = cxxdbg::dbg::term_settings::term_kind::builtin;
    } else if (customButton_->isChecked()) {
        tkind = cxxdbg::dbg::term_settings::term_kind::custom;
    }

    cxxdbg::dbg::term_settings sett(tkind,
                                  xtermPathEdit_->text().toStdString(),
                                  xtermCloseCheck_->isChecked(),
                                  customCommandEdit_->text().toStdString());
    return sett;
}


void TerminalSettingsWidget::updateGroups() {
    xtermGroup_->setEnabled(xtermButton_->isChecked());
    customGroup_->setEnabled(customButton_->isChecked());
}


}
