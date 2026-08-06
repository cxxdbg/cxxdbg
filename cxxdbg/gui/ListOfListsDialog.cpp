// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "ListOfListsDialog.h"
#include "cxxdbg_connect.hpp"

#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include "ExpressionsListDialog.h"
#include "ui_ListOfListsDialog.h"


namespace cxxdbg::gui {


ListOfListsDialog::ListOfListsDialog(cxxdbg::dbg::core::regex_lists& data, QWidget* parent):
QDialog(parent), data_(data)
{
    Ui::ListOfListsDialog ui;
    ui.setupUi(this);
    
    itemList_       = findChild<QListWidget*>("itemList");
    addButton_      = findChild<QPushButton*>("addButton");
    editButton_     = findChild<QPushButton*>("editButton");
    removeButton_   = findChild<QPushButton*>("removeButton");
    okButton_       = ui.buttonBox->button(QDialogButtonBox::Ok);

    assert(itemList_ != 0);
    assert(addButton_ != 0);
    assert(editButton_ != 0);
    assert(removeButton_ != 0);
    assert(okButton_ != 0);

    QDialogButtonBox * buttonBox = findChild<QDialogButtonBox *>();

    assert(buttonBox != 0);
    
    // fill the list 
    for (auto& item : data)
    {
        QString name = QString::fromStdString(item.name());
        bool is_enabled = item.enabled();
        
        QListWidgetItem* listItem = new QListWidgetItem(name, itemList_);
        listItem->setCheckState(is_enabled ? Qt::Checked : Qt::Unchecked);
        
        itemList_->addItem(listItem);
    }
    
    cxxdbg_connect(addButton_, &QPushButton::clicked, [this] { onAddButtonClicked(); });
    cxxdbg_connect(editButton_, &QPushButton::clicked, [this] { onEditButtonClicked(); });
    cxxdbg_connect(removeButton_, &QPushButton::clicked, [this] { onRemoveButtonClicked(); });
    
    cxxdbg_connect(itemList_, &QListWidget::currentRowChanged, [this] { updateState(); });
    
    cxxdbg_connect(itemList_, &QListWidget::itemChanged, [this](QListWidgetItem* item) { onListItemChanged(item); });
    
    updateState();
}


void ListOfListsDialog::onAddButtonClicked()
{
    // TODO: add generated name
    cxxdbg::dbg::core::regex_list_item item("");
    
    name_validator add_validator = make_add_validator(data_);
    
    ExpressionsListDialog dialog(item, add_validator, this);
    if (QDialog::Accepted == dialog.exec())
    {
        data_.add(item);
        QString qName = QString::fromStdString(item.name());
        //qDebug() << qName;
        
        QListWidgetItem* listItem = new QListWidgetItem(qName, itemList_);
        bool is_enabled = false;
        listItem->setCheckState(is_enabled ? Qt::Checked : Qt::Unchecked);
        itemList_->addItem(listItem);
    }
    
    updateState();
}


void ListOfListsDialog::onEditButtonClicked()
{
    int currentRowIndex = itemList_->currentRow();
    if (currentRowIndex < 0)
        return;
    
    QListWidgetItem* listItem = itemList_->item(currentRowIndex);
    
    name_validator validator = make_edit_validator(std::ref(data_), currentRowIndex);
    
    cxxdbg::dbg::core::regex_list_item item = data_[currentRowIndex];  // clone
    
    ExpressionsListDialog dialog(item, validator);
    if (QDialog::Accepted == dialog.exec())
    {
        data_[currentRowIndex] = item;
        
        QString qName = QString::fromStdString(item.name());
        listItem->setText(qName);
    }
    
    updateState();
}


void ListOfListsDialog::onRemoveButtonClicked()
{
    int currentRowIndex = itemList_->currentRow();
    if (currentRowIndex < 0)
        return;
    
    QListWidgetItem* listItem = itemList_->item(currentRowIndex);
    
    const std::string& name = data_[currentRowIndex].name();
    
    data_.remove(name);
    
    itemList_->removeItemWidget(listItem);
    delete listItem;
    
    
    updateState();
}


void ListOfListsDialog::updateState()
{
    int currentRowIndex = itemList_->currentRow();
    
    bool edition_disabled = currentRowIndex < 0;
    
    editButton_->setDisabled(edition_disabled);
    removeButton_->setDisabled(edition_disabled);
}


void ListOfListsDialog::onListItemChanged(QListWidgetItem* item)
{
    int row = itemList_->row(item);
    bool checked = item->checkState() == Qt::Checked;
    
    data_[row].enable(checked);
}


}
