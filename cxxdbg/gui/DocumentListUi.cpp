// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file DocumentListUi.cpp
/// Contains implementation of the DocumentListUi class.

#include "DocumentListUi.h"
#include "SaveChangesDialog.h"
#include "cxxdbg/app/text_file_document.hpp"
#include <ranges.hpp>
#include <sstream>
#include <QMessageBox>


namespace cxxdbg::gui {


bool DocumentListUi::ask_open_big_file(const std::filesystem::path & p, size_t sz) const {
    std::wostringstream msgText;
    msgText << "The size of file " << p << " is too large (" << sz << " bytes)."
            << " Are you sure you want open it?";

    QMessageBox msg(QMessageBox::Question,
                    QObject::tr("Opening large file"),
                    QString::fromStdWString(msgText.str()),
                    QMessageBox::Yes | QMessageBox::No,
                    getMainWindow());

    return msg.exec() == QMessageBox::Yes;
}


/// Asks user for saving modified file. Returns false if user cancelled action.
bool DocumentListUi::ask_save_files(const std::list<const cxxdbg::file_document*> & files, bool & save) const {
    auto fn = [](auto && doc) { return doc->path(); };
    auto file_paths_r = files | std::ranges::views::transform(fn);
    auto file_paths = std::vector<std::filesystem::path>{std::ranges::begin(file_paths_r), std::ranges::end(file_paths_r)};

    SaveChangesDialog dlg{file_paths, getMainWindow()};
    auto res = dlg.exec();

    if (res == QDialog::Rejected) {
        return false;
    } else if (res == QDialog::Accepted) {
        save = true;
    } else {
        save = false;
    }

    return true;
}


}
