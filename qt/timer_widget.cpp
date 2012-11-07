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
#include "debug.h"


INIT_LOGGER(qt, TimerModel);
INIT_LOGGER(qt, TimerWidget);


TimerModel::TimerModel(int iTotalDuration, int iAlertDuration)
    : m_totalDuration(0), m_alertDuration(0), m_remaining(m_totalDuration)
{
    m_timer = new QTimer(this);

    setTotalDuration(iTotalDuration);
    setAlertDuration(iAlertDuration);
    m_alertTriggered = false;

    QObject::connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTime()));
}


void TimerModel::setValue(int iNewValue)
{
    if (iNewValue != m_remaining)
    {
        m_remaining = iNewValue;
        emit valueChanged(iNewValue);
    }
}


void TimerModel::updateTime()
{
    setValue(m_remaining - 1);

    if (m_remaining <= 0)
    {
        // We reached the end of the countdown
        m_timer->stop();
        emit expired();
        return;
    }

    if (m_remaining == m_alertDuration)
    {
        m_alertTriggered = true;
        emit alert(m_alertDuration);
    }
}


void TimerModel::startTimer()
{
    // Timeout every second
    m_timer->start(1000);
}


void TimerModel::pauseTimer()
{
    m_timer->stop();
}


void TimerModel::resetTimer()
{
    pauseTimer();
    m_alertTriggered = false;
    setValue(m_totalDuration);
    emit timerReset();
}


bool TimerModel::isActiveTimer() const
{
    return m_timer->isActive();
}


void TimerModel::setTotalDuration(int iSeconds)
{
    ASSERT(iSeconds > 0, "Invalid total duration");
    if (iSeconds == m_totalDuration)
        return;
    m_totalDuration = iSeconds;
    emit newTotalDuration(m_totalDuration);
    resetTimer();
}


void TimerModel::setAlertDuration(int iSeconds)
{
    // The duration can be negative (to deactivate the alert)
    if (iSeconds == m_alertDuration)
        return;
    m_alertDuration = iSeconds;
    resetTimer();
}

// ---------------------------------------

TimerWidget::TimerWidget(QWidget *parent, TimerModel &iTimerModel)
    : QLCDNumber(parent), m_model(iTimerModel)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QObject::connect(&m_model, SIGNAL(valueChanged(int)),
                     this, SLOT(displayTime(int)));
    QObject::connect(&m_model, SIGNAL(alert(int)),
                     this, SLOT(alertTriggered()));
    QObject::connect(&m_model, SIGNAL(timerReset()),
                     this, SLOT(timerReset()));
    QObject::connect(&m_model, SIGNAL(newTotalDuration(int)),
                     this, SLOT(newTotalDuration(int)));

    // Initialize the display
    newTotalDuration(m_model.getTotalDuration());
    if (m_model.wasAlertTriggered())
        alertTriggered();
    displayTime(m_model.getValue());
}


void TimerWidget::displayTime(int iSeconds)
{
    display(QString("%1:%2")
        .arg(iSeconds / 60, 2)
        .arg(iSeconds % 60, 2, 10, QChar('0')));
}


void TimerWidget::newTotalDuration(int iNewTotal)
{
    // Adapt the number of digits dynamically
    int minutesLength = QString("%1").arg(iNewTotal / 60).length();
    setDigitCount(minutesLength + 3);
    setMinimumSize(QSize(60 + 20 * minutesLength, 40));
}


void TimerWidget::alertTriggered()
{
    QPalette pal = palette();
    pal.setColor(QPalette::Foreground, Qt::red);
    setPalette(pal);
}


void TimerWidget::timerReset()
{
    // Restore the default color
    QPalette pal = palette();
    pal.setColor(QPalette::Foreground, Qt::black);
    setPalette(pal);
}


void TimerWidget::mousePressEvent(QMouseEvent *iEvent)
{
    if (iEvent->button() == Qt::LeftButton)
    {
        if (m_model.isActiveTimer())
        {
            // Pause execution
            m_model.pauseTimer();
        }
        else if (m_model.getValue() > 0)
        {
            // Resume execution
            m_model.startTimer();
        }
        else
        {
            // Restart timer
            m_model.resetTimer();
            m_model.startTimer();
        }
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        m_model.resetTimer();
    }
}


void TimerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    m_model.resetTimer();
    m_model.startTimer();
}


QSize TimerWidget::sizeHint() const
{
    return QSize(340, 170);
}


