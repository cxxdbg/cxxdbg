// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file text_document_widget.ipp
/// Contains implementation of the text_document_widget class.


#include "text_document_widget.hpp"
#include "CxxSyntaxHighlighter.h"
#include "LineNumbersWidget.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/text_file_document.hpp"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QToolTip>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/find_iterator.hpp>
#include <ranges.hpp>
#include <QTextCursor>
#include <QDebug>

namespace ba = boost::algorithm;


namespace cxxdbg::gui {


/// Widget for displaying mark icons in source code on the left of line numbers.
/// Redirects size requests and paint events to main source code widget
template <
    tmvc::text_model TextModel,
    tmvc::selection_model SelectionModel,
    tmvc::selection_controller_for<TextModel> Controller
>
class DocumentMarksWidget: public QWidget {
public:
    /// Constructs widget with specified pointer to source code widget
    DocumentMarksWidget(text_document_widget<TextModel, SelectionModel, Controller> * docWidget):
    docWidget_{docWidget} {
    }

    /// Returns minimum size for widget
    QSize minimumSizeHint() const override {
        return sizeHint();
    }

    /// Returns preferred size for widget
    QSize sizeHint() const override {
        return QSize{docWidget_->documentMarksWidth(), 0};
    }

protected:
    /// Paint event handler
    void paintEvent(QPaintEvent * event) override {
        docWidget_->handleDocumentMarksPaint(event);
    }

    /// Mouse move event handler
    void mouseMoveEvent(QMouseEvent * event) override {
        docWidget_->handleDocumentMarksMouseMove(event);
    }

    /// Mouse click event handler
    void mouseReleaseEvent(QMouseEvent * event) override {
        docWidget_->handleDocumentMarksClick(event);
    }

private:
    /// Main document widget
    text_document_widget<TextModel, SelectionModel, Controller> * docWidget_;
};


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
text_document_widget<TextModel, SelectionModel, Controller>::text_document_widget(text_document_t * doc,
                                                                  text_document_widget_delegate * delegate,
                                                                  QWidget * parent):
text_document_widget_base{parent},
doc_{doc},
delegate_{delegate} {
    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);

    //setFrameShape(QFrame::StyledPanel);

    // document marks
    documentMarks_ = new DocumentMarksWidget(this);
    layout->addWidget(documentMarks_);
    documentMarks_->setMouseTracking(true);
    documentMarks_->setCursor(Qt::PointingHandCursor);
    documentMarks_->setContextMenuPolicy(Qt::CustomContextMenu);
    cxxdbg_connect(documentMarks_, &QWidget::customContextMenuRequested, [this](auto && pos) {
        onDocumentMarksContextMenu(pos);
    });

    // line numbers
    lineNumbers_ = new LineNumbersWidget(this);
    layout->addWidget(lineNumbers_);
    lineNumbers_->setCursor(Qt::PointingHandCursor);
    lineNumbers_->setMouseTracking(true);
    lineNumbers_->setContextMenuPolicy(Qt::PreventContextMenu);

    layout->addSpacing(5);

    // text view
    textWidget_ = new text_document_view{doc_->text_view_model()};
    layout->addWidget(textWidget_);
    textWidget_->setFrameStyle(QFrame::NoFrame);
    new CxxSyntaxHighlighter{textWidget_->document()};
    setFocusProxy(textWidget_);
    textWidget_->setLineWrapMode(QPlainTextEdit::NoWrap);
    textWidget_->setTextInteractionFlags(textWidget_->textInteractionFlags() | Qt::TextSelectableByKeyboard);
    textWidget_->set_custom_context_menu_delegate([this](QMenu * menu) {
        addTextWidgetContextMenuItems(menu);
    });


    setFocusPolicy(Qt::StrongFocus);

    // set monospace font
    QTextCharFormat fmt = textWidget_->currentCharFormat();
    fmt.setFontFixedPitch(true);
    textWidget_->setCurrentCharFormat(fmt);


    // update layout on changing number of lines
    cxxdbg_connect(textWidget_, &QPlainTextEdit::blockCountChanged, [this] {
        documentMarks_->updateGeometry();
        lineNumbers_->updateGeometry();
    });

    // repaint line numbers on text widget update
    cxxdbg_connect(textWidget_, &QPlainTextEdit::updateRequest, [this]() {
        documentMarks_->update();
        lineNumbers_->update();
    });

    // updating marks area when delegate says it should be updated
    if (delegate_) {
        mark_area_update_con_ = delegate_->mark_area_should_be_updated.connect([this] {
            documentMarks_->update();
        });
    }

    // listening for highlighted lines changes and updating highlighted lines
    if (delegate_) {
        highlighted_lines_changed_con_ = delegate_->highlighted_lines_changed.connect([this] {
            updateHighlights();
        });
    }
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::highlight_search_string(const QString & str, bool match_case) {
    searchResults_.clear();

    std::wstring text = textWidget_->toPlainText().toStdWString();
    std::wstring wstr = str.toStdWString();

    typedef ba::find_iterator<std::wstring::iterator> find_iterator;
    find_iterator it;

    if (match_case) {
        auto finder = ba::first_finder(wstr, boost::algorithm::is_equal());
        it = ba::make_find_iterator(text, finder);
    } else {
        auto finder = ba::first_finder(wstr, boost::algorithm::is_iequal());
        it = ba::make_find_iterator(text, finder);
    }

    for (; it != find_iterator(); ++it) {
        // adding highlight selection
        QTextCursor curs = textWidget_->textCursor();
        curs.clearSelection();
        curs.setPosition(it->begin() - text.begin());
        curs.movePosition(QTextCursor::Right,
                          QTextCursor::KeepAnchor,
                          wstr.size());
        searchResults_.push_back(curs);
    }

    updateHighlights();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::remove_search_string_highlight() {
    searchResults_.clear();
    updateHighlights();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::search_next(const QString & str, bool match_case) {
    std::wstring text = textWidget_->toPlainText().toStdWString();
    std::wstring wstr = str.toStdWString();

    std::size_t curPos = textWidget_->textCursor().position();

    auto range = boost::make_iterator_range(text.begin() + curPos, text.end());
    auto res = match_case ?
               ba::find_first(range, wstr) :
               ba::ifind_first(range, wstr);

    if (res.begin() == res.end()) {
        // trying search from begining

        std::size_t endPos = curPos + wstr.size();
        if (endPos > text.size())
            endPos = text.size();

        auto range2 = boost::make_iterator_range(text.begin(), text.begin() + endPos);
        res = match_case ?
              ba::find_first(range2, wstr) :
              ba::ifind_first(range2, wstr);
    }

    if (res.begin() == res.end()) {
        // not found
        return;
    }

    // setting position
    QTextCursor cursor = textWidget_->textCursor();
    cursor.setPosition(res.begin() - text.begin());
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, wstr.size());
    textWidget_->setTextCursor(cursor);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::search_prev(const QString & str, bool match_case) {
    std::wstring text = textWidget_->toPlainText().toStdWString();
    std::wstring wstr = str.toStdWString();

    std::size_t curPos = textWidget_->textCursor().anchor();

    auto range = boost::make_iterator_range(text.begin(), text.begin() + curPos);
    auto res = match_case ?
               ba::find_last(range, wstr) :
               ba::ifind_last(range, wstr);

    if (res.begin() == res.end()) {
        // trying search from end

        std::size_t startPos = curPos > wstr.size() ? curPos - wstr.size() : 0;
        auto range2 = boost::make_iterator_range(text.begin() + startPos, text.end());
        res = match_case ?
              ba::find_last(range2, wstr) :
              ba::ifind_last(range2, wstr);
    }

    if (res.begin() == res.end()) {
        // not found
        return;
    }

    // setting position
    QTextCursor cursor = textWidget_->textCursor();
    cursor.setPosition(res.begin() - text.begin());
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, wstr.size());
    textWidget_->setTextCursor(cursor);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::set_font(const QFont & font) {
    documentMarks_->setFont(font);
    lineNumbers_->setFont(font);
    textWidget_->setFont(font);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::set_tab_size(unsigned sz) {
    auto fnt = textWidget_->font();
    QFontMetrics fmetrics{fnt};
    textWidget_->setTabStopDistance(fmetrics.horizontalAdvance(' ') * sz);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleDocumentMarksPaint(QPaintEvent * event) {

    if (!delegate_) {
        event->accept();
        return;
    }

    QPainter painter(documentMarks_);

    // painting icons for each line

    unsigned int firstLine = firstVisibleLine();
    unsigned int lastLine = lastVisibleLine();

    for (unsigned int l = firstLine; l < lastLine; ++l) {
        auto icons = delegate_->calculate_line_icons(doc_, l);
        for (auto && icon : icons) {
            paintIcon(painter, l, icon);
        }
    }

    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleDocumentMarksMouseMove(QMouseEvent * event) {
    // show line description

    int line = lineNumberFromMousePos(event->pos());
    if (line == -1) {
        QToolTip::hideText();
        return;
    }

    std::string desc;
    if (delegate_) {
        desc = delegate_->get_line_description(doc_, line);
    }

    if (desc.empty()) {
        QToolTip::hideText();
        return;
    }

    QToolTip::showText(lineNumbers_->mapToGlobal(event->pos()), QString::fromStdString(desc));

    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleDocumentMarksClick(QMouseEvent * event) {
    int line = lineNumberFromMousePos(event->pos());
    if (line == -1)
        return;

    // setting position to selected line
    doc_->go_to_line(line);

    if (event->button() != Qt::LeftButton)
        return;

    if (delegate_) {
        delegate_->handle_line_click(doc_, line);
    }

    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleLineNumbersPaint(QPaintEvent * event) {
    QPainter painter(lineNumbers_);
    paintLineNumbers(painter);
    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleLineNumbersMouseMove(QMouseEvent *event) {
    // doing nothing for now
    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::handleLineNumbersClick(QMouseEvent * event) {

    int line = lineNumberFromMousePos(event->pos());
    if (line == -1)
        return;

    if (event->button() != Qt::LeftButton)
        return;

    // setting position to selected line
    doc_->go_to_line(line);

    event->accept();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::lineNumberFromMousePos(const QPoint & pos) {

    // searching line number
    int line = firstVisibleLine();
    int endLine = lastVisibleLine();
    if (line == -1)
        return -1;

    for (; line < endLine; ++line) {
        int lPos = linePos(line);
        if (lPos <= pos.y() && pos.y() <= lPos + lineNumbers_->fontMetrics().height()) {
            break;
        }
    }

    if (line == endLine)
        return -1;

    return line;
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::paintLineNumbers(QPainter & painter) {

    for (int line = firstVisibleLine(), endLine = lastVisibleLine(); line < endLine; ++line) {

        int pos = linePos(line);

        // use bold font for current line
        QFont oldFont = painter.font();
        if (textWidget_->textCursor().blockNumber() == line) {
            QFont boldFont = painter.font();
            boldFont.setBold(true);
            painter.setFont(boldFont);
        }

        // paint line number
        QString number = QString::number(line + 1);
        auto cWidth = lineNumbers_->fontMetrics().horizontalAdvance(QLatin1Char('9'));
        painter.drawText(0, pos, lineNumbers_->width() - cWidth, painter.fontMetrics().height(),
                         Qt::AlignRight, number);

        // restore old font
        if (textWidget_->textCursor().blockNumber() == line) {
            painter.setFont(oldFont);
        }
    }
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::paintIcon(QPainter & painter, unsigned int line, QIcon icon) {

    int fontHeight = painter.fontMetrics().height();
    int pos = linePos(line);

    // getting icon pixmap
    QPixmap pixmap = icon.pixmap(fontHeight, fontHeight);

    // calculating drawing rect
    QRect drawAreaRect(0, pos, fontHeight, fontHeight);
    QRect pixmapRect(0, pos, pixmap.width(), pixmap.height());
    pixmapRect.moveCenter(drawAreaRect.center());

    // drawing pixmap
    painter.drawPixmap(pixmapRect, pixmap);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::documentMarksWidth() const {
    return documentMarks_->fontMetrics().height();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::linesNumbersWidth() const {
    int lines = textWidget_->blockCount();
    unsigned digits = 1;

    while (lines != 0) {
        lines = lines / 10;
        ++digits;
    }

    return lineNumbers_->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::firstVisibleLine() const {
    QTextBlock block =  textWidget_->firstVisibleBlock();
    if (!block.isValid())
        return -1;

    return block.blockNumber();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::lastVisibleLine() const {
    int line = firstVisibleLine();
    if (line == -1)
        return -1;

    while (line < textWidget_->blockCount() &&
           linePos(line) < lineNumbers_->height() + lineNumbers_->fontMetrics().height()) {
        ++line;
    }

    return line;
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
int text_document_widget<TextModel, SelectionModel, Controller>::linePos(int lineNum) const {
    assert(lineNum >= 0 && "Invalid line number");
    assert(lineNum < textWidget_->blockCount() && "Invalid line number");

    QTextBlock block = textWidget_->document()->findBlockByNumber(lineNum);
    assert(block.isValid() && "Invalid text block");

    return textWidget_->blockBoundingGeometry(block)
        .translated(textWidget_->contentOffset()).top();
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::onDocumentMarksContextMenu(const QPoint & pos) {

    if (!delegate_) {
        return;
    }

    int line = lineNumberFromMousePos(pos);
    if (line == -1) {
        return;
    }

    auto menu = delegate_->create_marks_area_context_menu(doc_, line);
    QPoint globalPos = documentMarks_->mapToGlobal(pos);
    menu->exec(globalPos);
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::addTextWidgetContextMenuItems(QMenu * menu) {
    if (delegate_) {
        menu->addSeparator();

        int line = textWidget_->textCursor().blockNumber();
        auto selText = textWidget_->textCursor().selectedText().toStdWString();
        delegate_->add_text_context_menu_actions(menu, doc_, line, selText);
    }
}


template <tmvc::text_model TextModel, tmvc::selection_model SelectionModel, tmvc::selection_controller_for<TextModel> Controller>
void text_document_widget<TextModel, SelectionModel, Controller>::updateHighlights() {

    QList<QTextEdit::ExtraSelection> selections;

    // calculating list of highlighted lines using delegate
    std::list<std::pair<size_t, QColor>> highlightLines;
    if (delegate_) {
        highlightLines = delegate_->calculate_highlighted_lines(doc_);
    }

    // highlighting lines returned from delegate
    for (auto highlightLine : highlightLines) {
        assert(highlightLine.first < textWidget_->document()->blockCount() && "invalid highlight line number");
        auto block = textWidget_->document()->findBlockByLineNumber(highlightLine.first);

        QTextEdit::ExtraSelection selection;
        selection.cursor = textWidget_->textCursor();
        selection.cursor.setPosition(block.position());
        selection.format.setBackground(highlightLine.second);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selections.append(selection); 
    }

    // highlighting search results
    for (auto curs : searchResults_) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = curs;
        sel.format.setBackground(Qt::yellow);
        selections.append(sel);
    }

    textWidget_->setExtraSelections(selections);
}


}
