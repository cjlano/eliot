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

#include <vector>
#include <QLabel>
#include <QString>

using std::vector;

class QMouseEvent;
class QEvent;
class QTimer;


/**
 * Concatenate the given strings, as long as the delay between 2 consecutive
 * strings is not higher than the delayMs constructor parameter.
 * Otherwise, the string restarts fom scratch.
 */
class KeyAccumulator: public QObject
{
    Q_OBJECT;

public:
    KeyAccumulator(QObject *parent, int delayMs);

    QString addText(QString iAddedText);

private:
    int m_delayMs;
    QTimer *m_timer;
    QString m_currText;
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

