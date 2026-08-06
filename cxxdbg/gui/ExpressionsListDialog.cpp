// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

#include "ExpressionsListDialog.h"
#include "RegularExpressionDialog.h"
#include "cxxdbg_connect.hpp"

/// generated ui file
#include "ui_ExpressionsListDialog.h"


namespace cxxdbg::gui {


#ifndef UI_EXPRESSIONSLISTDIALOG_H

// stub for IDE

namespace Ui
{
    class ExpressionsListDialog
    {
    public:
        template<class T>
        void setupUi(T * t)
        {
        }
    };
}

#endif


name_validator make_add_validator(cxxdbg::dbg::core::regex_lists& data)
{
    auto data_ref = std::ref(data);
    
    name_validator validator = [data_ref](std::string const& name)->validation_result {
        if (name.empty()) {
            return validation_result(false, "Name must not be empty");
        }
        
        if (data_ref.get().has_name(name)) {
            return validation_result(false, "Duplicate list item name");
        }
        
        return validation_result(true, "Ok");
    };
    
    return validator;
}


name_validator make_edit_validator(cxxdbg::dbg::core::regex_lists& data, int index)
{
    auto data_ref = std::ref(data);
    
    name_validator validator = [data_ref, index](std::string const& name)->validation_result {
        if (name.empty()) {
            return validation_result(false, "Name must not be empty");
        }
        
        if (data_ref.get().has_name(name, index)) {
            return validation_result(false, "Duplicate list item name");
        }
        
        return validation_result(true, "Ok");
    };
    
    return validator;
}


ExpressionsListDialog::ExpressionsListDialog(
                cxxdbg::dbg::core::regex_list_item& item,
                name_validator validator,
                QWidget* parent): 
        QDialog(parent),
        item_(item)
{
    validator_ = validator;
    
    Ui::ExpressionsListDialog ui;
    
    ui.setupUi(this);
    
    rightPixmap_ = QIcon::fromTheme("dialog-information").pixmap(16, 16);
    wrongPixmap_ = QIcon::fromTheme("dialog-error").pixmap(16, 16);
    
    listNameEdit_ = findChild<QLineEdit*>("listNameEdit");
    itemList_ = findChild<QListWidget*>("itemList");
    correctnessLabel_ = findChild<QLabel *>("correctnessLabel");
    statusBarLabel_   = findChild<QLabel *>("statusBarLabel");
    addButton_ = findChild<QPushButton *>("addButton");
    editButton_ = findChild<QPushButton *>("editButton");
    removeButton_ = findChild<QPushButton *>("removeButton");
    

    assert(listNameEdit_ != 0);
    assert(itemList_ != 0);
    assert(correctnessLabel_ != 0);
    assert(statusBarLabel_ != 0);
    assert(addButton_ != 0);
    assert(editButton_ != 0);
    assert(removeButton_ != 0);

    QDialogButtonBox * buttonBox = findChild<QDialogButtonBox *>();

    assert(buttonBox != 0);

    okButton_ = buttonBox -> button(QDialogButtonBox::Ok);
    
    // set list name
    listNameEdit_->setText(QString::fromStdString(item_.name()));
    
    // fill list content
    for (int i = 0; i < item_.size(); ++i) {
        QString textItem = QString::fromStdString(item_[i]);
        itemList_->addItem(textItem);
    }
    
    cxxdbg_connect(addButton_, &QPushButton::clicked, [this] { onAddButtonClicked(); });
    
    cxxdbg_connect(editButton_, &QPushButton::clicked, [this] { onEditButtonClicked(); });
    
    cxxdbg_connect(removeButton_, &QPushButton::clicked, [this] { onRemoveButtonClicked(); });
    
    cxxdbg_connect(listNameEdit_, &QLineEdit::textChanged,
            [this] (const QString&){ updateState(); });
            
    cxxdbg_connect(itemList_, &QListWidget::currentRowChanged,
            [this](int index) { updateState(); });
    
    updateState();
}


void ExpressionsListDialog::onAddButtonClicked()
{
    RegularExpressionDialog dialog({}, this);
    
    int dialogResult = dialog.exec();
    
    if (dialogResult == QDialog::Accepted) {
        const QString& expression = dialog.expression();
        itemList_->addItem(expression);
        item_.add(expression.toStdString());
    }
    
    updateState();
}


void ExpressionsListDialog::onEditButtonClicked()
{
    int currentRowIndex = itemList_->currentRow();
    if (currentRowIndex < 0) {
        return;
    }
    
    std::string& expression = item_[currentRowIndex];
    
    RegularExpressionDialog dialog(QString::fromStdString(expression), this);
    
    if (QDialog::Accepted == dialog.exec()) {
        const QString newExpression = dialog.expression();
        expression = newExpression.toStdString();
        itemList_->item(currentRowIndex)->setText(newExpression);
    }
    
    updateState();
}


void ExpressionsListDialog::onRemoveButtonClicked()
{
    int currentRowIndex = itemList_->currentRow();
    if (currentRowIndex < 0) {
        return;
    }
    
    QListWidgetItem* widget = itemList_->item(currentRowIndex);
    
    itemList_->removeItemWidget(widget);
    delete widget;
    item_.remove(currentRowIndex);
    
    updateState();
}


void ExpressionsListDialog::updateState()
{   
    bool isCorrect = false;
    std::string hint;
    
    std::string name = listNameEdit_->text().toStdString();
    item_.set_name(name);
    
    std::tie(isCorrect, hint) = validator_(name);
    
    const QPixmap& pixmap = isCorrect ? rightPixmap_ : wrongPixmap_;
    
    correctnessLabel_->setPixmap(pixmap);
    okButton_->setEnabled(isCorrect);
    
    QString const& statusText = QString::fromStdString(hint);

    statusBarLabel_->setText(statusText);
    
    int currentRowIndex = itemList_->currentRow();
    
    editButton_->setEnabled(currentRowIndex >= 0);
    removeButton_->setEnabled(currentRowIndex >= 0);
}


void ExpressionsListDialog::setupDialog()
{
    Ui::ExpressionsListDialog ui;
    
    ui.setupUi(this);
    
    rightPixmap_ = QIcon::fromTheme("dialog-information").pixmap(16, 16);
    wrongPixmap_ = QIcon::fromTheme("dialog-error").pixmap(16, 16);
    
    listNameEdit_ = findChild<QLineEdit*>("listNameEdit");
    itemList_ = findChild<QListWidget*>("itemList");
    correctnessLabel_ = findChild<QLabel *>("correctnessLabel");
    statusBarLabel_   = findChild<QLabel *>("statusBarLabel");
    addButton_ = findChild<QPushButton *>("addButton");
    editButton_ = findChild<QPushButton *>("editButton");
    removeButton_ = findChild<QPushButton *>("removeButton");

    assert(listNameEdit_ != 0);
    assert(itemList_ != 0);
    assert(correctnessLabel_ != 0);
    assert(statusBarLabel_ != 0);
    assert(addButton_ != 0);
    assert(editButton_ != 0);
    assert(removeButton_ != 0);
    
    // set list name
    listNameEdit_->setText(QString::fromStdString(item_.name()));
    
    // fill list content
    for (int i = 0; i < item_.size(); ++i) {
        QString textItem = QString::fromStdString(item_[i]);
        itemList_->addItem(textItem);
    }
    
    cxxdbg_connect(addButton_, &QPushButton::clicked,  [this] { onAddButtonClicked(); });
    
    cxxdbg_connect(editButton_, &QPushButton::clicked, [this] { onEditButtonClicked(); });
    
    cxxdbg_connect(removeButton_, &QPushButton::clicked, [this] { onRemoveButtonClicked(); });
    
    cxxdbg_connect(listNameEdit_, &QLineEdit::textChanged,
            [this] (const QString&){ updateState(); });
            
    cxxdbg_connect(itemList_, &QListWidget::currentRowChanged,
            [this](int index) { updateState(); });
    
    updateState();
}


}
