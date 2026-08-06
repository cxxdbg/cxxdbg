// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file Utils.cpp
/// Contains implementation of utility GUI functions.

#include "Utils.h"
#include <QFontDatabase>
#include <QFontInfo>
#include <QIcon>
#include <QtGlobal>


namespace cxxdbg::gui {


QFont defaultFixedFont() {
#ifdef __APPLE__
    {
        auto fontName = "Menlo";
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QFont f(QStringList{fontName});
#else
        QFont f(fontName);
#endif
        f.setStyleHint(QFont::Monospace);
        return f;
    }
#elif defined(__WIN32)
    // trying use Consolas font on Windows
    {
        QFont cf("Consolas");
        if (cf.family() == "Consolas") {
            return cf;
        }
    }
#else
    {
        static const char * candidates[] = {
            "DejaVu Sans Mono",
            "Liberation Mono",
            "Noto Sans Mono",
            "Ubuntu Mono",
        };

        for (auto name: candidates) {
            QFont f(name);
            f.setPointSize(QFont().pointSize());
            if (f.family() == name && QFontInfo{f}.fixedPitch()) {
                return f;
            }
        }

        QFont f("monospace");
        f.setStyleHint(QFont::Monospace);
        f.setPointSize(QFont().pointSize());
        if (QFontInfo{f}.fixedPitch()) {
            return f;
        }
    }
#endif

    // getting default font via Qt
    auto f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    f.setPointSize(QFont().pointSize());
    return f;
}


QFont defaultCodeViewerFont() {
    auto f = defaultFixedFont();
    f.setPointSize(f.pointSize() + 1);
    return f;
}


QIcon makeSvgIcon(const QString & path, const QString & name) {
    QIcon icon;
    icon.addFile(path + "/" + name + "_16.svg");
    //icon.addFile(path + "/" + name + "_32.svg");
    //icon.addFile(path + "/" + name + "_48.svg");
    return icon;
}


}
