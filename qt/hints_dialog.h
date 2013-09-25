/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#ifndef HINTS_DIALOG_H_
#define HINTS_DIALOG_H_

#include <vector>

#include <QDialog>

#include "logging.h"


using std::vector;

class AbstractHint;
class Move;


/**
 * Hint widget, used in the HintsDialog class
 */
class HintWidget: public QWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit HintWidget(const AbstractHint &iHint,
                        bool iShowCosts,
                        QWidget *parent = 0);

    const AbstractHint & getHint() const { return m_hint; }

signals:
    void hintRequested(const AbstractHint &iHint);

private slots:
    void buttonClicked();

private:
    const AbstractHint &m_hint;

};


/**
 * Hints dialog
 */
class HintsDialog: public QDialog
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit HintsDialog(QWidget *parent = 0, bool iShowCosts = false);
    ~HintsDialog();

public slots:
    void setMove(const Move &iMove);

signals:
    void hintUsed(const AbstractHint &iHint);
    void notifyProblem(QString msg);

private slots:
    void showHint(const AbstractHint &iHint);

private:
    const Move *m_move;
    vector<const AbstractHint *> m_allHints;
    bool m_showCosts;

    /// Initialize the m_allHints vector
    void initializeHints();

};

#endif

