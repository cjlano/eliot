/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>

#include "aux_window.h"


AuxWindow::AuxWindow(QWidget &iWidget, QAction *iAction)
    : m_widget(iWidget), m_action(iAction)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(&iWidget);
    setLayout(layout);
}


AuxWindow::~AuxWindow()
{
    delete &m_widget;
}


void AuxWindow::showEvent(QShowEvent * event)
{
    if (m_action)
        m_action->setChecked(true);
    QWidget::showEvent(event);
}


void AuxWindow::hideEvent(QHideEvent * event)
{
    if (m_action)
        m_action->setChecked(false);
    QWidget::hideEvent(event);
}

