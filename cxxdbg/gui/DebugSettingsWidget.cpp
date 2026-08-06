// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "DebugSettingsWidget.h"

#include "RegularExpressionDialog.h"
#include "ExpressionsListDialog.h"
#include "ListOfListsDialog.h"
#include "cxxdbg_connect.hpp"

#include "cxxdbg/dbg/core/regex_lists.hpp"

#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>

#include <iostream>
#include <string>


namespace cxxdbg::gui {


DebugSettingsWidget::DebugSettingsWidget(const cxxdbg::dbg::debug_settings & settings,
                           QWidget * parent):
    QWidget(parent), debug_settings_(settings)
{
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    
    avoidNodebugCheck_ = new QCheckBox(
            tr( "Don't stop at functions without debug info "
                "when performing step into / step out."),
            this);
    
    avoidNodebugCheck_->setChecked(debug_settings_.is_avoid_nodebug());
    
    cxxdbg_connect(avoidNodebugCheck_, &QCheckBox::toggled, 
            [this](bool checked) {
                debug_settings_.set_avoid_nodebug(checked);
            });
    
    cxxdbg_connect(avoidNodebugCheck_, &QCheckBox::toggled, [this] { changed(); });
    
    layout->addWidget(avoidNodebugCheck_);
    
    {
        QPushButton* button = new QPushButton(tr("Function names to skip when performing step in / step out..."), this);
    
        layout->addWidget(button);
        layout->addStretch(1);

        cxxdbg_connect(button, &QPushButton::clicked, 
                [this] () {
                    cxxdbg::dbg::core::regex_lists data = debug_settings_.regexp_skip_lists();
                    
                    assert(data.size() > 0);
                    
                    ListOfListsDialog dialog(data, this);                    
                    
                    if (QDialog::Accepted == dialog.exec())
                    {
                        // save lists
                        debug_settings_.set_regexp_skip_lists(data);
                    }
                    
                    emit changed();
        });
    }
    
    {
        QPushButton* button = new QPushButton(tr("Function names to step through when performing step in / step out..."), this);
    
        layout->addWidget(button);
        layout->addStretch(1);

        cxxdbg_connect(button, &QPushButton::clicked, 
                [this] () {
                    cxxdbg::dbg::core::regex_lists data = debug_settings_.step_through_lists();
                    
                    assert(data.size() > 0);
                    
                    ListOfListsDialog dialog(data, this);                    
                    
                    if (QDialog::Accepted == dialog.exec())
                    {
                        // save lists
                        debug_settings_.set_step_through_lists(data);
                    }
                    
                    emit changed();
        });
    }

    {
        QPushButton* button = new QPushButton(tr("Function names to group in call stack..."), this);

        layout->addWidget(button);
        layout->addStretch(1);

        cxxdbg_connect(button, &QPushButton::clicked, [this] () {
                    cxxdbg::dbg::core::regex_lists data = debug_settings_.group_functions_lists();

                    assert(data.size() > 0);

                    ListOfListsDialog dialog(data, this);

                    if (QDialog::Accepted == dialog.exec())
                    {
                        // save lists
                        debug_settings_.set_group_functions_lists(data);
                    }

                    emit changed();
                });
    }
}


const cxxdbg::dbg::debug_settings& DebugSettingsWidget::debugSettings() const
{
    return debug_settings_;
}


}
