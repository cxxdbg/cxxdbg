// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>

#include "cxxdbg/dbg/core/regex_lists.hpp"


namespace cxxdbg::gui {


class ListOfListsDialog: public QDialog
{
    Q_OBJECT
    public:
        ListOfListsDialog(cxxdbg::dbg::core::regex_lists& data, QWidget* parent = 0);
        
    public slots:
        /// add button clicked event handler
        void onAddButtonClicked();
        
        /// edit button clicked event handler
        void onEditButtonClicked();
        
        /// delete button clicked event handler
        void onRemoveButtonClicked();

        /// updates states of dialog items
        void updateState();
        
        /// handles item change (check state)
        void onListItemChanged(QListWidgetItem* item);
        
    private:
        cxxdbg::dbg::core::regex_lists&     data_;              ///< list of lists
        
        QListWidget*                itemList_;          ///< list widget
        QPushButton*                addButton_;         ///< add item button
        QPushButton*                editButton_;        ///< edit item button
        QPushButton*                removeButton_;      ///< remove item button
        QPushButton*                okButton_;          ///< ok button
};


}
