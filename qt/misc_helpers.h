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

#ifndef MISC_HELPERS_H_
#define MISC_HELPERS_H_

#include <QtGui/QLabel>


class QMouseEvent;
class QEvent;


/// Event filter used for the edition of the players display
class KeyEventFilter: public QObject
{
    Q_OBJECT;

public:
    KeyEventFilter(QObject *parent, int key, int modifier = Qt::NoModifier);

signals:
    /// As its name indicates...
    void keyPressed();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    int m_modifiers;
    int m_key;
};


class ClickableLabel: public QLabel
{
    Q_OBJECT;

public:
    explicit ClickableLabel(QWidget *parent = 0);

signals:
    void clicked();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif

