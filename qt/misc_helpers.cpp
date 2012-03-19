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

#include <QtGui/QKeyEvent>

#include "misc_helpers.h"


KeyEventFilter::KeyEventFilter(QObject *parent, int key, int modifiers)
    : QObject(parent)
{
    addKey(key, modifiers);
}


void KeyEventFilter::addKey(int key, int modifiers)
{
    m_keyVect.push_back(key);
    m_modifiersVect.push_back(modifiers);
}


bool KeyEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    // If the Delete key is pressed, remove the selected line, if any
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        for (unsigned i = 0; i < m_keyVect.size(); ++i)
        {
            if (keyEvent->key() == m_keyVect[i] &&
                keyEvent->modifiers() == m_modifiersVect[i])
            {
                emit keyPressed(m_keyVect[i], m_modifiersVect[i]);
                return true;
            }
        }
    }

    // Standard event processing
    return QObject::eventFilter(obj, event);
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

