/*****************************************************************************
 * Eliot
 * Copyright (C) 2011-2012 Olivier Teulière
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

#include <QMenu>

#include "custom_popup.h"
#include "qtcommon.h"


INIT_LOGGER(qt, CustomPopup);


CustomPopup::CustomPopup(QWidget *iWidget)
    : QObject(iWidget), m_widget(iWidget)
{
    iWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(iWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
                     this, SLOT(showPopup(const QPoint&)));
}

void CustomPopup::showPopup(const QPoint &iPoint)
{
    QMenu menu(m_widget);
    emit popupCreated(menu, iPoint);
    if (!menu.isEmpty())
        menu.exec(m_widget->mapToGlobal(iPoint));
}


QAction * CustomPopup::getShowDefinitionEntry(QString iWord)
{
    m_word = iWord;

    QAction *definitionAction = new QAction(_q("Show definition"), m_widget);
    definitionAction->setStatusTip(_q("Show definition of '%1' in an external browser").arg(m_word));
    QObject::connect(definitionAction, SIGNAL(triggered()),
                     this, SLOT(definitionRequested()));

    return definitionAction;
}


void CustomPopup::definitionRequested()
{
    emit requestDefinition(m_word);
}
