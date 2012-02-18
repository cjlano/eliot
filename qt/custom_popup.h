/*****************************************************************************
 * Eliot
 * Copyright (C) 2011 Olivier Teulière
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

#ifndef CUSTOM_POPUP_H_
#define CUSTOM_POPUP_H_

#include <QtCore/QObject>
#include <QtCore/QString>

#include "logging.h"

class QWidget;
class QPoint;
class QMenu;


class CustomPopup: public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    CustomPopup(QWidget *iWidget);
    void addShowDefinitionEntry(QMenu &iPopup, QString iWord);

signals:
    void requestDefinition(QString iWord);
    void popupCreated(QMenu &iPopup, const QPoint &iPoint);

private slots:
    void showPopup(const QPoint &iPoint);
    void definitionRequested();

private:
    /// Widget triggering the custom popup menu
    QWidget *m_widget;

    /// Word to define
    QString m_word;
};

#endif

