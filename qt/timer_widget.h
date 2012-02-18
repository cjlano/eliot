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


/// Helper class, used to synchronize the various instances of TimerWidget
class TimerModel : public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    TimerModel(int iTotalDuration, int iAlertDuration);

    // Accessors
    int getValue() const { return m_remaining; }
    void setValue(int iNewValue);

    int getTotalDuration() const { return m_totalDuration; }
    void setTotalDuration(int iSeconds);

    int getAlertDuration() const { return m_alertDuration; }
    void setAlertDuration(int iSeconds);

    bool wasAlertTriggered() const { return m_alertTriggered; }
    bool isExpired() const { return m_remaining == 0; }

    // Timer handling
    void startTimer();
    void pauseTimer();
    void resetTimer();
    bool isActiveTimer() const;

signals:
    void valueChanged(int iNewValue);
    void alert(int iRemainingSeconds);
    void expired();
    void timerReset();
    void newTotalDuration(int iNewTotal);

private slots:
    void updateTime();

private:
    /// Total duration of the timer, in seconds
    int m_totalDuration;

    /// Alert duration of the timer, in seconds (must be lower than the total duration)
    int m_alertDuration;

    /// Number of remaining seconds
    int m_remaining;

    /// Indicate whether we triggered an alert
    bool m_alertTriggered;

    /// Timer used for the countdown
    QTimer *m_timer;
};


class TimerWidget : public QLCDNumber
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    TimerWidget(QWidget *parent, TimerModel &iTimerModel);

private slots:
    /// Format the given time and display it
    void displayTime(int iSeconds);

    void newTotalDuration(int iNewTotal);
    void alertTriggered();
    void timerReset();

protected:
    // Events handling
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    /// Define a default size
    virtual QSize sizeHint() const;

private:
    /// Model representing the number of remaining seconds
    TimerModel &m_model;
};

#endif
