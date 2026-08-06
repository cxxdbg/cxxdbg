// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>

#include <functional>
#include <tuple>

#include "cxxdbg/dbg/core/regex_lists.hpp"


namespace cxxdbg::gui {


/// validation result type consists of two values
/// correctness sign and message hint
typedef std::tuple<bool, std::string> validation_result;
/// name validator signature
typedef std::function<validation_result(std::string const&)> name_validator;


/// validator for adding new item
name_validator make_add_validator(cxxdbg::dbg::core::regex_lists& data);

/// validator for editing existing item
name_validator make_edit_validator(cxxdbg::dbg::core::regex_lists& data, int index);


class ExpressionsListDialog: public QDialog
{
    // Q_OBJECT
    public:
        ExpressionsListDialog(
                cxxdbg::dbg::core::regex_list_item& item,
                name_validator validator,
                QWidget* parent = 0);
        
    public slots:
        /// add button clicked event handler
        void onAddButtonClicked();
        
        /// edit button clicked event handler
        void onEditButtonClicked();
        
        /// delete button clicked event handler
        void onRemoveButtonClicked();

        /// updates states of dialog items
        void updateState();
        
    private:
        /// setup dialog
        void setupDialog();
        
        cxxdbg::dbg::core::regex_list_item& item_;              ///< regexp list
        name_validator              validator_;         ///< item name validator
        QLineEdit*                  listNameEdit_;      ///< list name editor
        QListWidget*                itemList_;          ///< list widget
        QLabel*                     correctnessLabel_;  ///< ok/bad icon
        QLabel*                     statusBarLabel_;    ///< status bar label
        QPushButton*                addButton_;         ///< add item button
        QPushButton*                editButton_;        ///< edit item button
        QPushButton*                removeButton_;      ///< remove item button
        QPushButton*                okButton_;          ///< ok button
        QPixmap                     rightPixmap_;       ///< "right" icon
        QPixmap                     wrongPixmap_;       ///< "wrong" icon
};


}
