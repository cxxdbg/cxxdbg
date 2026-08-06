// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DbgSettingsDialog.cpp

#include "DbgSettingsDialog.h"
#include "FontSelectWidget.h"
#include "DebugSettingsWidget.h"
#include "CxxdbgApplication.h"
#include "TerminalSettingsWidget.h"
#include "Utils.h"
#include "cxxdbg_connect.hpp"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>


namespace cxxdbg::gui {


DbgSettingsDialog::DbgSettingsDialog(CxxdbgApplication & app, QWidget * parent):
SettingsDialog{false, parent},
app_(app) {
    this->setWindowTitle(tr("Settings"));

    // Code viewer settings
    {
        QWidget * codeViewSettings = new QWidget;
        addPage(tr("Code viewer"), codeViewSettings);

        QVBoxLayout * codeViewSetLayout = new QVBoxLayout(codeViewSettings);
        codeViewSetLayout->setContentsMargins({});

        // font settings
        QGroupBox * fontSettingsGroup = new QGroupBox(tr("Font settings"), codeViewSettings);

        codeViewSetLayout->addWidget(fontSettingsGroup);

        QVBoxLayout * fontSettingsLayout = new QVBoxLayout(fontSettingsGroup);

        fontSelect_ = new FontSelectWidget(app_.codeViewerFont(), defaultCodeViewerFont(), fontSettingsGroup);

        fontSettingsLayout->addWidget(fontSelect_);
        cxxdbg_connect(fontSelect_, &FontSelectWidget::changed, [this] { onChanged(); });
    }

    // Terminal settings
    {
        termSettings_ = new TerminalSettingsWidget(app_.term_sett());
        addPage(tr("Terminal"), termSettings_);
        cxxdbg_connect(termSettings_, &TerminalSettingsWidget::changed, [this] { onChanged(); });
    }

    // Debug settings
    {
        debugSettings_ = new DebugSettingsWidget(app_.debug_sett());
        addPage(tr("Debug"), debugSettings_);
        cxxdbg_connect(debugSettings_, &DebugSettingsWidget::changed, [this] { onChanged(); });
    }

    cxxdbg_connect(this, &SettingsDialog::applyClicked, [this] { onApplyClicked(); });
    onChanged();
}


void DbgSettingsDialog::onApplyClicked() {
    app_.setCodeViewerFont(fontSelect_->selectedFont());
    app_.set_term_sett(termSettings_->termSettings());
    app_.set_debug_setttings(debugSettings_->debugSettings());
    onChanged();
}


void DbgSettingsDialog::onChanged() {
    // enable Apply button if something was changed
    bool wasChanged = false;

    wasChanged |= fontSelect_->selectedFont() != app_.codeViewerFont();
    wasChanged |= termSettings_->termSettings() != app_.term_sett();
    wasChanged |= debugSettings_->debugSettings() != app_.debug_sett();

    setApplyEnabled(wasChanged);
}


}
