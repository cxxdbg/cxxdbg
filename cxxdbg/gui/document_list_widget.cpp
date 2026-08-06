// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "document_list_widget.hpp"
#include "document_widget.hpp"
#include "LineNumberDialog.h"
#include "text_document_widget.hpp"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/document_list.hpp"
#include "cxxdbg/app/text_file_document.hpp"

#include <cassert>
#include <filesystem>
#include <sstream>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QStyle>
#include <QTabBar>
#include <QTextStream>
#include <QTabWidget>
#include <QVBoxLayout>


namespace fs = std::filesystem;


namespace cxxdbg::gui {


document_list_widget::document_list_widget(cxxdbg::document_list & docs,
                                           const std::shared_ptr<document_widget_factory> & doc_widget_factory,
                                           QWidget * parent):
QFrame(parent),
docs_(docs),
doc_widget_factory_{doc_widget_factory},
matchCase_(false),
searchHighlight_(false),
textFont_{font()},
tabSize_{4} {

    setFrameStyle(QFrame::NoFrame);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins({});

    // tab widget
    tabs_ = new QTabWidget;
    layout->addWidget(tabs_);
    tabs_->setMovable(true);
    tabs_->setTabsClosable(true);
    tabs_->setAutoFillBackground(true);
    cxxdbg_connect(tabs_, &QTabWidget::tabCloseRequested, [this](int index) {
        document_widget * widget = dynamic_cast<document_widget*>(tabs_->widget(index));
        assert(widget != nullptr && "Invalid tab widget");
        docs_.close(widget->doc());
    });

    // redirect focus in to current opened widget
    cxxdbg_connect(tabs_, &QTabWidget::currentChanged, [this](auto && ...) {
        auto tab = tabs_->currentWidget();
        setFocusProxy(tab);
    });

    // context menu for tabs
    tabs_->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    cxxdbg_connect(tabs_->tabBar(), &QTabBar::customContextMenuRequested, [this](const QPoint & pos) {
        QMenu menu;

        // close
        QIcon closeIcon = style()->standardIcon(QStyle::SP_DialogCloseButton);
        QAction * closeAction = menu.addAction(closeIcon, tr("Close"));
        cxxdbg_connect(closeAction, &QAction::triggered, [this, pos](auto && ...) {
            int tabIndex = tabs_->tabBar()->tabAt(pos);
            assert(tabIndex != -1 && "Invalid tab");

            document_widget * widget = dynamic_cast<document_widget*>(tabs_->widget(tabIndex));
            assert(widget != nullptr && "Invalid tab widget");
            docs_.close(widget->doc());
        });

        // close all
        QAction * closeAllAction = menu.addAction(tr("Close all"));
        cxxdbg_connect(closeAllAction, &QAction::triggered, [this](auto && ...) {
            docs_.close_all();
        });

        // separator
        menu.addSeparator();

        // open
        QIcon openIcon = style()->standardIcon(QStyle::SP_DirOpenIcon);
        QAction * openAction = menu.addAction(openIcon, tr("Open..."));
        cxxdbg_connect(openAction, &QAction::triggered, [this](auto && ...) { openSelected(); });

        menu.exec(mapToGlobal(pos));
    });


    // context menu for central widget
    setContextMenuPolicy(Qt::CustomContextMenu);
    cxxdbg_connect(this, &document_list_widget::customContextMenuRequested, [this](const QPoint & pos) {
        QMenu menu;

        // open
        QIcon openIcon = style()->standardIcon(QStyle::SP_DirOpenIcon);
        QAction * openAction = menu.addAction(openIcon, tr("Open..."));
        cxxdbg_connect(openAction, &QAction::triggered, [this](auto && ...) { openSelected(); });

        menu.exec(mapToGlobal(pos));
    });

    // hadling activating new tab
    cxxdbg_connect(tabs_, &QTabWidget::currentChanged, [this](int idx) {
        if (idx == -1)
            return;

        document_widget * widget = dynamic_cast<document_widget*>(tabs_->widget(idx));
        assert(widget != nullptr && "Invalid tab widget");
        docs_.set_current(widget->doc());
    });


    // connecting to document list signals

    source_file_opened_con_ = docs_.document_opened.connect(
    [this](cxxdbg::document * doc) {
        onDocumentOpened(doc);
    });

    source_file_closed_con_ = docs_.document_closed.connect(
    [this](cxxdbg::document * doc) {
        onDocumentClosed(doc);
    });

    current_source_changed_con_ = docs_.current_changed.connect([this]() {
        onCurrentSourceChanged();
    });

    document_name_changed_con_ = docs_.document_name_changed.connect([this](auto doc) {
        this->updateDocumentName(doc);
    });

    document_changed_changed_con_ = docs_.document_changed_changed.connect([this](auto doc) {
        this->updateDocumentName(doc);
    });
}


void document_list_widget::setTextFont(const QFont & font) {
    // setting font in QWidget to be able to reuse it for new document widgets
    textFont_ = font;

    // updating font for all document widgets
    for (auto codeview : sourceWidgets_) {
        codeview.second->setFont(textFont_);
    }
}


void document_list_widget::setTabSize(unsigned sz) {
    tabSize_ = sz;

    for (auto codeview : sourceWidgets_) {
        codeview.second->set_tab_size(tabSize_);
    }
}


void document_list_widget::highlightSearchString(const QString & str, bool mCase) {
    searchString_ = str;
    matchCase_ = mCase;
    emit canDoSearchChanged();

    if (searchString_.isEmpty()) {
        removeSearchStringHighlight();
        return;
    }

    searchHighlight_ = true;

    if (tabs_->count() > 0) {
        activeDocumentWidget()->highlight_search_string(str, matchCase_);
    }
}


void document_list_widget::removeSearchStringHighlight() {
    searchHighlight_ = false;

    for (auto srcWidget : sourceWidgets_) {
        srcWidget.second->remove_search_string_highlight();
    }
}


void document_list_widget::searchNext() {
    assert(!searchString_.isEmpty() && "Can't search next empty string");
    activeDocumentWidget()->search_next(searchString_, matchCase_);
}


void document_list_widget::searchPrev() {
    assert(!searchString_.isEmpty() && "Can't search next empty string");
    activeDocumentWidget()->search_prev(searchString_, matchCase_);
}


void document_list_widget::gotoLineNumber() {
    assert(tabs_->count() > 0); // this should never happen
        
    auto lcount = static_cast<unsigned int>(docs_.lines_count());
    LineNumberDialog dialog(1, lcount, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        docs_.go_to_line(static_cast<size_t>(dialog.lineNumber() - 1));
    }
}


bool document_list_widget::canDoSearch() const {
    return docs_.current() != nullptr && !searchString_.isEmpty();
}


void document_list_widget::displayNextTab() {
    if (tabs_->count() == 0) {
        return;
    }

    auto idx = tabs_->currentIndex();
    if (idx == tabs_->count() - 1) {
        idx = 0;
    } else {
        ++idx;
    }
    
    tabs_->setCurrentIndex(idx);
}

    
void document_list_widget::displayPrevTab() {
    if (tabs_->count() == 0) {
        return;
    }

    auto idx = tabs_->currentIndex();
    if (idx == 0) {
        idx = tabs_->count() - 1;
    } else {
        --idx;
    }
    
    tabs_->setCurrentIndex(idx);
}


void document_list_widget::onDocumentOpened(cxxdbg::document * doc) {
    auto tdoc = dynamic_cast<cxxdbg::text_document*>(doc);
    assert(tdoc && "unsupported document type opened");

    // make new document widget
    auto doc_widget = doc_widget_factory_->create_document_widget(*doc);
    doc_widget->setFont(textFont_);
    doc_widget->set_tab_size(tabSize_);

    // add source file in file -> widget map
    sourceWidgets_.insert(std::make_pair(doc, doc_widget.get()));

    // adding new tab
    tabs_->addTab(doc_widget.release(), QString::fromStdWString(doc->name_desc()));

    emit canDoSearchChanged();
}


void document_list_widget::onDocumentClosed(cxxdbg::document * doc) {
    // getting source code widget from file
    auto it = sourceWidgets_.find(doc);

    if (it == sourceWidgets_.end()) {
        // no widget for file. This happens when we can't read file after it was opened
        return;
    }

    auto srcWidget = it->second;

    // removing widget from map
    sourceWidgets_.erase(it);

    // closing tab
    tabs_->removeTab(tabs_->indexOf(srcWidget));

    emit canDoSearchChanged();
}


void document_list_widget::onCurrentSourceChanged() {

    // doing nothing if no current widget
    if (docs_.current() == nullptr)
        return;

    // getting source code widget from file
    auto it = sourceWidgets_.find(docs_.current());
    assert(it != sourceWidgets_.end() && "Source file widget not found");
    auto srcWidget = it->second;

    // making tab active
    tabs_->setCurrentWidget(srcWidget);

    // highlighting search text in new active widget
    if (searchHighlight_) {
        srcWidget->highlight_search_string(searchString_, matchCase_);
    }
}


void document_list_widget::updateDocumentName(const cxxdbg::document * doc) {
    // getting pointer to document widget
    auto it = sourceWidgets_.find(doc);
    assert(it != sourceWidgets_.end() && "can't find document in map");

    // searching for index of document widget in tab widget
    auto idx = tabs_->indexOf(it->second);
    assert(idx != -1 && "cant' find document tab index");

    // setting tab name using document description
    tabs_->setTabText(idx, QString::fromStdWString(doc->name_desc()));
}


document_widget * document_list_widget::activeDocumentWidget() {
    QWidget * w = tabs_->currentWidget();
    assert(w != nullptr && "No active source code widget");
    auto doc_w = dynamic_cast<document_widget*>(w);
    assert(doc_w != nullptr && "invalid document widget");
    return doc_w;
}


void document_list_widget::updateStyle() {
    if (tabs_->count() == 0) {
        setFrameStyle(QFrame::StyledPanel);

        // setting background color for widget
        {
            auto p = QApplication::palette();
            auto col = p.color(QPalette::Base);
            p.setColor(QPalette::Base, col);
            setPalette(p);
            setAutoFillBackground(true);
        }
    } else {
        setFrameStyle(QFrame::NoFrame);
        setAutoFillBackground(false);
    }
}


}
