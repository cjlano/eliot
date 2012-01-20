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

#ifndef TIMER_WIDGET_H_
#define TIMER_WIDGET_H_

#include <QtGui/QLCDNumber>

#include "logging.h"


class QTimer;

class TimerWidget : public QLCDNumber
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    TimerWidget(QWidget *parent = 0,
                int iTotalDuration = 180,
                int iAlertDuration = 30);

signals:
    void alert(int iRemainingSeconds);
    void expired();

public slots:
    void setTotalDuration(int iSeconds);
    void setAlertDuration(int iSeconds);

private slots:
    void updateTime();
    void alertTriggered();

protected:
    // Events handling
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    /// Define a default size
    virtual QSize sizeHint() const;

private:
    /// Total duration of the timer, in seconds
    int m_totalDuration;

    /// Alert duration of the timer, in seconds (must be lower than the total duration)
    int m_alertDuration;

    /// Number of remaining seconds
    int m_remaining;

    /// Timer used for the countdown
    QTimer *m_timer;

    /// Start the timer
    void startTimer();

    /// Reset the timer
    void resetTimer();

    /// Format the given time and display it
    void displayTime(int iSeconds);
};

#endif
