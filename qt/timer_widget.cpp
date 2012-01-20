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

#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>

#include "timer_widget.h"

INIT_LOGGER(qt, TimerWidget);


TimerWidget::TimerWidget(QWidget *parent, int iTotalDuration, int iAlertDuration)
    : QLCDNumber(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_timer = new QTimer(this);

    setTotalDuration(iTotalDuration);
    setAlertDuration(iAlertDuration);
    m_remaining = m_totalDuration;

    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    QObject::connect(this, SIGNAL(alert(int)), this, SLOT(alertTriggered()));
}


void TimerWidget::startTimer()
{
    // Timeout every second
    m_timer->start(1000);
}


void TimerWidget::resetTimer()
{
    m_timer->stop();
    displayTime(m_totalDuration);
    m_remaining = m_totalDuration;

    // Restore the default color
    QPalette pal = palette();
    pal.setColor(QPalette::Foreground, Qt::black);
    setPalette(pal);
}


void TimerWidget::alertTriggered()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Foreground, Qt::red);
    setPalette(pal);
}


void TimerWidget::setTotalDuration(int iSeconds)
{
    m_totalDuration = iSeconds;
    if (m_totalDuration < 0)
        m_totalDuration = 0;
    // Make sure we don't exceed the digit count
    int minutesLength = QString("%1").arg(m_totalDuration / 60).length();
    setDigitCount(minutesLength + 3);

    if (!m_timer->isActive())
    {
        displayTime(m_totalDuration);
    }
}


void TimerWidget::setAlertDuration(int iSeconds)
{
    m_alertDuration = iSeconds;
}


void TimerWidget::updateTime()
{
    --m_remaining;

    displayTime(m_remaining);

    if (m_remaining <= 0)
    {
        // We reached the end of the countdown
        m_timer->stop();
        emit expired();
        return;
    }

    if (m_remaining == m_alertDuration)
    {
        emit alert(m_remaining);
    }
}


void TimerWidget::displayTime(int iSeconds)
{
    display(QString("%1:%2")
        .arg(iSeconds / 60, 2)
        .arg(iSeconds % 60, 2, 10, QChar('0')));
}


void TimerWidget::mousePressEvent(QMouseEvent *iEvent)
{
    if (iEvent->button() == Qt::LeftButton)
    {
        if (m_timer->isActive())
        {
            // Pause execution
            m_timer->stop();
        }
        else if (m_remaining > 0)
        {
            // Resume execution
            startTimer();
        }
        else
        {
            // Restart timer
            resetTimer();
            startTimer();
        }
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        resetTimer();
    }
}


void TimerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    resetTimer();
    startTimer();
}


QSize TimerWidget::sizeHint() const
{
    return QSize(340, 170);
}


