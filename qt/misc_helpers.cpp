/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#include <QKeyEvent>
#include <QTimer>

#include "misc_helpers.h"


KeyAccumulator::KeyAccumulator(QObject *parent, int delayMs)
    : QObject(parent), m_delayMs(delayMs), m_currText("")
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
}


QString KeyAccumulator::addText(QString iAddedText)
{
    if (m_timer->isActive())
        m_timer->stop();
    else
        m_currText = "";
    m_currText += iAddedText;
    m_timer->start(m_delayMs);
    return m_currText;
}



ClickableLabel::ClickableLabel(QWidget *parent)
    : QLabel(parent)
{
}


void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    QLabel::mousePressEvent(event);
    emit clicked();
}

