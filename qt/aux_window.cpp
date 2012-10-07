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

#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCloseEvent>
#include <QtCore/QSettings>

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
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

