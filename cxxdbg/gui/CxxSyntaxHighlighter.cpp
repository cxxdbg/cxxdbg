// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file CxxSyntaxHighlighter.cpp
/// Contains implementation of the CxxSyntaxHighlighter class.

#include "CxxSyntaxHighlighter.h"
#include <locale>
#include <QColor>
#include <set>


namespace cxxdbg::gui {


static const QColor commentColor{0, 128, 0};
static const QColor preprocColor{0, 0, 128};
static const QColor keywordColor{128, 128, 0};
static const QColor typeColor{59, 115, 175};
static const QColor stringColor = commentColor;

static std::set<std::string> keywords = {
    "static",
    "register",
    "auto",
    "if",
    "else",
    "for",
    "do",
    "while",
    "switch",
    "case",
    "default",
    "return",
    "const",
    "volatile",
    "override",
    "virtual",
    "operator",
    "namespace",
    "constexpr",
    "class",
    "struct",
    "union",
    "private",
    "public",
    "protected",
    "new",
    "delete",
    "assert",
    "break",
    "try",
    "catch",
    "throw",
    "template",
    "typename",
    "typedef",
    "static_cast",
    "dynamic_cast",
    "reinterpret_cast",
    "const_cast",
    "this",
    "enum"
};


static std::set<std::string> types = {
    "true",
    "false",
    "bool",
    "NULL",
    "nullptr",
    "void",
    "unsigned",
    "char",
    "short",
    "int",
    "long",
    "float",
    "double",
    "size_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "intptr_t",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "uintptr_t"
};


CxxSyntaxHighlighter::CxxSyntaxHighlighter(QTextDocument * doc):
QSyntaxHighlighter{doc} {
}


void CxxSyntaxHighlighter::highlightBlock(const QString & qtext) {

    std::string text = qtext.toStdString();

    int wordStartIdx = 0;
    int commentStartIdx = 0;
    int curIdx = 0;
    int endIdx = text.size();
    int stringStartIdx = 0;
    bool inStringLiteral = false;
    bool inCharLiteral = false;
    int charStartIdx = 0;
    std::locale loc;

    bool inComment = previousBlockState() == 1;

    while (true) {
        if (curIdx == endIdx)
            break;

        if (std::isalnum(text[curIdx], loc) || text[curIdx] == '_') {
            ++curIdx;
            continue;
        }

        if (wordStartIdx != curIdx) {
            std::string word = text.substr(wordStartIdx, curIdx - wordStartIdx);
            if (keywords.find(word) != keywords.end()) {
                setFormat(wordStartIdx, curIdx - wordStartIdx, keywordColor);
            } else if (types.find(word) != types.end()) {
                setFormat(wordStartIdx, curIdx - wordStartIdx, typeColor);
            }
        }


        if (text[curIdx] == '/') {
            if (!inComment) {
                // checking for comment start

                ++curIdx;
                if (curIdx == endIdx)
                    break;

                char c = text[curIdx];
                if (c == '/') {
                    // line comment
                    setFormat(curIdx - 1, endIdx - commentStartIdx, commentColor);
                    break;
                } else if (c == '*') {
                    // C-style comment
                    commentStartIdx = curIdx - 1;
                    inComment = true;
                }
            }
        } else if (text[curIdx] == '*') {
            if (inComment) {
                // checking for comment end

                ++curIdx;
                if (curIdx == endIdx)
                    break;

                char c = text[curIdx];
                if (c == '/') {
                    // end of comment
                    inComment = false;
                    setFormat(commentStartIdx, curIdx - commentStartIdx + 1, commentColor);
                }
            }

        } else if (text[curIdx] == '#') {
            if (!inComment) {
                // preprocessor directive
                setFormat(curIdx, endIdx - curIdx, preprocColor);
                break;
            }
        } else if (text[curIdx] == '"') {
            if (inStringLiteral) {
                inStringLiteral = false;
                setFormat(stringStartIdx, curIdx - stringStartIdx + 1, stringColor);
            } else {
                inStringLiteral = true;
                stringStartIdx = curIdx;
            }
        } else if (text[curIdx] == '\\' && (inStringLiteral || inCharLiteral)) {
            // escape character
            ++curIdx;
            if (curIdx == endIdx) {
                break;
            }
        } else if (text[curIdx] == '\'') {
            if (inCharLiteral) {
                inCharLiteral = false;
                setFormat(charStartIdx, curIdx - charStartIdx + 1, stringColor);
            } else {
                inCharLiteral = true;
                charStartIdx = curIdx;
            }
        }

        ++curIdx;
        wordStartIdx = curIdx;
    }

    // highlighting multiline C-style comment
    if (inComment) {
        setFormat(commentStartIdx, endIdx - commentStartIdx, commentColor);
    }

    setCurrentBlockState(inComment ? 1 : 0);
}


}
