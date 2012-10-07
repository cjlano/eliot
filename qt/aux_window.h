/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#ifndef AUX_WINDOW_H_
#define AUX_WINDOW_H_

#include <QtGui/QWidget>

#include "logging.h"

class QAction;


/**
 * This class is a wrapper for widgets which should be in their own window.
 * It ensures that all the auxiliary windows have a common behaviour, which
 * we can change easily.
 */
class AuxWindow: public QWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    /**
     * Constructor. The AuxWindow takes ownership of the given widget (i.e. it
     * is responsible for its deletion).
     * @param iWindowTitle: title of the window; e.g.: "Bag"
     * @param iWindowName: name of the window, used in the settings
     *      e.g.: "BagWindow"
     * @param iAction: if not NULL, it will be checked/unchecked when the
     *      window is shown/hidden
     */
    AuxWindow(QWidget &iWidget, QString iWindowTitle,
              QString iWindowName, QAction *iAction = NULL);
    virtual ~AuxWindow();

    const QWidget & getWidget() const { return m_widget; }

    void toggleVisibility();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    QWidget &m_widget;

    /// Name of the window, used to save its state
    QString m_windowName;

    /// Action to check/uncheck
    QAction *m_action;

    /// Save window state
    void writeSettings() const;
    /// Restore window state
    void readSettings();
};

#endif
