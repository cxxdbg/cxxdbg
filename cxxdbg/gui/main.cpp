// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file main.cpp
/// cxxdbg gui main file

#include "MainWindow.h"
#include "CxxdbgApplication.h"
#include "cxxdbg/app/log.hpp"
#include "cxxdbg/dbg/command_line.hpp"
#include "cxxdbg/log/log_init.hpp"
#include "cxxdbg/log/log.hpp"
#include <clocale>
#include <iostream>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    try {
        // parsing options


        std::setlocale(LC_ALL, "");

        cxxdbg::dbg::command_line cmdLine;
        cmdLine.parse(argc, argv);

        auto product_name = "CxxDbg";

        // set application name
        QCoreApplication::setOrganizationName("cxxdbg");
        QCoreApplication::setApplicationName("cxxdbg");

        // create application object
        ::cxxdbg::gui::CxxdbgApplication a(argc, argv);

        // show help message if requested
        if (cmdLine.show_help()) {
            cmdLine.print_help(std::cout, product_name);
#ifdef _WIN32
            std::ostringstream help_msg;
            cmdLine.print_help(help_msg, product_name);
            QMessageBox::information(nullptr,
                                     QString{product_name} + " Help",
                                     QString::fromStdString(help_msg.str()));
#endif
            return 1;
        }

        // show version info if requested
        if (cmdLine.show_version()) {
            cmdLine.print_version(std::cout, product_name);
#ifdef _WIN32
            std::ostringstream vers_msg;
            cmdLine.print_version(vers_msg, product_name);
            QMessageBox::information(nullptr,
                                     QString{product_name} + " Version",
                                     QString::fromStdString(vers_msg.str()));
#endif
            return 1;
        }

        cxxdbg::log::init(cmdLine.vars());

        // additional try-ctach block for displaying message boxes for errors
        try {
            // show main window
            ::cxxdbg::gui::MainWindow w(a);
            w.show();

            // process command line
            a.processCommandLine(cmdLine);

            return a.exec();
        }
        catch(std::exception & ex) {
            QMessageBox::critical(nullptr,
                                  "Critical error",
                                  QString::fromStdString(ex.what()));
            throw;
        }
    }
    catch(std::exception & ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 2;
    }
}
