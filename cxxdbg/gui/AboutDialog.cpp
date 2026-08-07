// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#include "AboutDialog.h"
#include "cxxdbg_connect.hpp"
#include "cxxdbg/app/version.hpp"
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QSvgRenderer>
#include <QVBoxLayout>


namespace cxxdbg::gui {


/// Renders an SVG file onto a square pixmap of specified size, with a white
/// background and a rounded border, clipped to the same rounded rectangle
static QPixmap renderSvgCard(const QString & path, int size, qreal radius = 16,
                             qreal borderWidth = 2, const QColor & borderColor = QColor{0xB0, 0xB0, 0xB0}) {
    QPixmap pixmap{size, size};
    pixmap.fill(Qt::transparent);

    QPainter painter{&pixmap};
    painter.setRenderHint(QPainter::Antialiasing);

    qreal inset = borderWidth / 2.0;
    QRectF rect{inset, inset, size - borderWidth, size - borderWidth};

    QPainterPath roundedRect;
    roundedRect.addRoundedRect(rect, radius, radius);

    painter.setClipPath(roundedRect);
    painter.fillPath(roundedRect, Qt::white);

    QSvgRenderer renderer{path};
    renderer.render(&painter, rect);

    painter.setClipping(false);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen{borderColor, borderWidth});
    painter.drawPath(roundedRect);

    return pixmap;
}


AboutDialog::AboutDialog(const QString & productName, QWidget *parent) :
    QDialog(parent)
{

    setWindowTitle(tr("About %1").arg(productName));

    QString appDir = QCoreApplication::applicationDirPath();

    QVBoxLayout * layout = new QVBoxLayout(this);

    {
        QHBoxLayout * mainLayout = new QHBoxLayout;
        layout->addLayout(mainLayout);

        // logo

        logoPixmap_ = renderSvgCard(appDir + "/../share/cxxdbg/images/logo-title.svg", 128);
        QLabel * logo = new QLabel;
        mainLayout->addWidget(logo, 0, Qt::AlignTop);
        logo->setPixmap(logoPixmap_);

        mainLayout->addSpacing(20);

        // version label
        QString version = tr("%1 version").arg(productName) + " " +
                QString::fromStdString(cxxdbg::version_display_str());

        version += ".<br>";

        version += "<br>";

        version += "For updates and more info, please visit <a href=\"https://cxxdbg.org\">https://cxxdbg.org</a><br>";

        version += "<br>";

        version += tr("This software uses Qt framework (<a href=http://www.qt.io>http://www.qt.io</a>)<br>");
        version += tr("This software uses LLVM Compiler Infrastructure (<a href=http://www.llvm.org>http://www.llvm.org</a>)<br>");

        QLabel * vLabel = new QLabel(version);
        vLabel->setTextFormat(Qt::RichText);
        vLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        mainLayout->addWidget(vLabel, 0, Qt::AlignTop);

        mainLayout->addStretch(1);
    }


    // Ok button
    QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttons);
    cxxdbg_connect(buttons, &QDialogButtonBox::accepted, [this] { accept(); });
}


}
