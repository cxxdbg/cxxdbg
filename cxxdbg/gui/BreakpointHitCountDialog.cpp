// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file BreakpointHitCountDialog.cpp
/// Contains implementation of BreakpointHitCountDialog class.

#include "BreakpointHitCountDialog.h"
#include "cxxdbg_connect.hpp"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>


namespace cxxdbg::gui {


BreakpointHitCountDialog::BreakpointHitCountDialog(QWidget * parent):
QDialog(parent) {
    setWindowTitle(tr("Breakpoint hit count"));

    QVBoxLayout * layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr(
        "Breakpoint hit count is the number of times breakpoint location\n"
        "has been reached and breakpoint condition has been met.")));

    // enable check box
    enableCheckBox_ = new QCheckBox(tr("Break only if hit count is greater than or equal to:"));
    layout->addWidget(enableCheckBox_);
    cxxdbg_connect(enableCheckBox_, &QCheckBox::stateChanged, [this](auto && ...) { onEnableStateChanged(); });

    // hit count line edit
    hitCountLineEdit_ = new QLineEdit;
    hitCountLineEdit_->setValidator(new QIntValidator(0, 2000000000));
    layout->addWidget(hitCountLineEdit_);

    // current hit count
    currHitCountLabel_ = new QLabel(tr("Current hit count: 0"));
    layout->addWidget(currHitCountLabel_);

    // reset current hit count
    resetCurrHitCount_ = new QCheckBox(tr("Reset current hit count to 0"));
    layout->addWidget(resetCurrHitCount_);

    // dialog buttons
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { onAccepted(); });
    cxxdbg_connect(buttons, &QDialogButtonBox::rejected, [this] { reject(); });

    onEnableStateChanged();
}


bool BreakpointHitCountDialog::hitCountEnabled() const {
    return enableCheckBox_->isChecked();
}


void BreakpointHitCountDialog::setHitCountEnabled(bool val) {
    enableCheckBox_->setChecked(val);
}


unsigned int BreakpointHitCountDialog::hitCount() const {
    return hitCountLineEdit_->text().toUInt();
}


void BreakpointHitCountDialog::setHitCount(unsigned val) {
    hitCountLineEdit_->setText(QString::number(val));
}


void BreakpointHitCountDialog::setCurrHitCount(unsigned val) {
    currHitCountLabel_->setText(tr("Current hit count: %1").arg(QString::number(val)));
}


bool BreakpointHitCountDialog::resetCurrHitCount() const {
    return resetCurrHitCount_->isChecked();
}


void BreakpointHitCountDialog::onEnableStateChanged() {
    hitCountLineEdit_->setEnabled(enableCheckBox_->isChecked());
}


void BreakpointHitCountDialog::onAccepted() {
    accept();
}


}
