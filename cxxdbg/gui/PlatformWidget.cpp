// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file PlatformWidget.cpp
/// Contains implementation of the PlatformWidget class.

#include "PlatformWidget.h"
#include "FormLayout.h"
#include "cxxdbg_connect.hpp"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextBlock>


Q_DECLARE_METATYPE(const cxxdbg::dbg::platform*)


namespace cxxdbg::gui {


PlatformWidget::PlatformWidget(const cxxdbg::dbg::platform_list & plist,
                               bool displayHost,
                               bool displayCurrent,
                               QWidget * parent):
QWidget{parent},
platforms_{plist},
displayCurrent_{displayCurrent} {

    auto layout = new FormLayout{this};
    layout->setContentsMargins({});

    auto descText = "Please select platform from the list and enter platform URL. "
                    "For most platforms, URL contains host name and port number in the form of "
                    "<host-name>:<port>.";
    auto descLabel = new QLabel{descText};
    layout->addRow(descLabel);
    descLabel->setWordWrap(true);

    // combo box with list of platforms
    platformCombo_ = new QComboBox;
    layout->addRow("Platform:", platformCombo_);

    if (displayCurrent) {
        QVariant data;
        data.setValue(static_cast<const cxxdbg::dbg::platform*>(nullptr));
        platformCombo_->addItem("Current platform", data);
    }

    for (auto && p : plist.all()) {
        if (!displayHost && p == plist.host()) {
            continue;
        }

        QVariant data;
        data.setValue(p);
        platformCombo_->addItem(QString::fromStdString(p->name()), data);
        platformCombo_->setItemData(platformCombo_->count() - 1, QString::fromStdString(p->desc()), Qt::ToolTipRole);
    }

    if (!displayCurrent_) {
        platformCombo_->setCurrentIndex(-1);
    }

    cxxdbg_connect(platformCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](auto && ...) {
        onSelectedPlatformChanged();
    });

    // URL
    urlLineEdit_ = new QLineEdit;
    urlLineEdit_->setMinimumWidth(200);
    layout->addRow("URL:", urlLineEdit_);
    urlLineEdit_->setEnabled(false);

    // Executable search paths
    {
        searchPaths_ = new QPlainTextEdit;
        searchPaths_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        searchPaths_->setWordWrapMode(QTextOption::NoWrap);
        QFontMetrics fm{searchPaths_->font()};
        auto margins = searchPaths_->contentsMargins();
        searchPaths_->setMaximumHeight(fm.lineSpacing() * 4 + margins.top() + margins.bottom() + 1);
        layout->addRow(new QLabel{"Executable and shared library search paths:"});
        layout->addRow(searchPaths_);
    }

    onSelectedPlatformChanged();
}


const cxxdbg::dbg::platform * PlatformWidget::selectedPlatform() const {
    if (platformCombo_->currentIndex() == -1) {
        return nullptr;
    }

    return platformCombo_->currentData().value<const cxxdbg::dbg::platform*>();
}


QString PlatformWidget::url() const {
    if (selectedPlatform() != nullptr) {
        return urlLineEdit_->text();
    } else {
        return {};
    }
}


std::vector<std::string> PlatformWidget::execSearchPaths() const {
    std::vector<std::string> res;
    auto block = searchPaths_->document()->firstBlock();
    while (block.isValid()) {
        auto txt = block.text();
        if (!txt.isEmpty()) {
            res.push_back(txt.toStdString());
        }
        block = block.next();
    }

    return res;
}


cxxdbg::dbg::platform_connection_options PlatformWidget::platformOpts() const {
    cxxdbg::dbg::platform_connection_options opts;
    opts.plat = selectedPlatform();
    opts.url = url().toStdString();
    opts.exec_search_paths = execSearchPaths();
    return opts;
}


bool PlatformWidget::checkOptions() {
    if (displayCurrent_) {
        // current platform is selected
        return true;
    }

    auto selPlatform = selectedPlatform();

    // checking that platform is selected
    if (selPlatform == nullptr) {
        QMessageBox::critical(this,
                              "Error connecting to platform",
                              "Platform is not selected. Please select one of the platforms from list.");
        return false;
    }

    return true;
}


void PlatformWidget::onSelectedPlatformChanged() {
    // disable URL field for default or host platform
    auto selPlat = selectedPlatform();
    urlLineEdit_->setEnabled((!displayCurrent_ || selPlat != nullptr) &&
                             selPlat != platforms_.host());
    emit selectedPlatformChanged();
}


}
