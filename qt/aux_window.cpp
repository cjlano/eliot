/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include "config.h"

#include <QApplication>
#include <QAction>
#include <QWidget>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QSettings>

#include "aux_window.h"
#include "qtcommon.h"


INIT_LOGGER(qt, AuxWindow);


AuxWindow::AuxWindow(QWidget &iWidget, QString iWindowTitle,
                     QString iWindowName, QAction *iAction)
    : m_widget(iWidget), m_windowName(iWindowName), m_action(iAction)
{
    setWindowTitle(iWindowTitle);
    setWindowIcon(qApp->windowIcon());
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(&iWidget);
    setLayout(layout);

    readSettings();
}


AuxWindow::~AuxWindow()
{
    delete &m_widget;
}


void AuxWindow::toggleVisibility()
{
    if (isVisible())
        hide();
    else
        show();
}


void AuxWindow::showEvent(QShowEvent *event)
{
    if (m_action)
        m_action->setChecked(true);
    QWidget::showEvent(event);
}


void AuxWindow::hideEvent(QHideEvent *event)
{
    if (m_action)
        m_action->setChecked(false);
    QWidget::hideEvent(event);
}


void AuxWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void AuxWindow::writeSettings() const
{
    // For some reason, this method is called when the window is destroyed,
    // even when it was already closed. So avoid saving an invalid position.
    if (!isVisible())
        return;
    QSettings settings;
    settings.beginGroup(m_windowName);
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
}


void AuxWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(m_windowName);
    QSize size = settings.value("size").toSize();
    if (size.isValid())
        resize(size);
    const QRect &desktopRect = QApplication::desktop()->screenGeometry();
    QPoint point = settings.value("pos", QPoint(20, 20)).toPoint();
    // If the position was saved when an external monitor was plugged, and
    // is restored when the monitor is not there anymore, the window could
    // be off screen...
    if (point.x() < 0 || point.x() > desktopRect.right())
        point.setX(20);
    if (point.y() < 0 || point.y() > desktopRect.bottom())
        point.setY(20);
    move(point);
    settings.endGroup();
}

